#include <simple_persist_storage.hpp>

namespace CXXL
{

// 這個類別在存放 IPersistChannel 和 TreeNode<PersistData_String> 兩者對應的實作物件
// 並提供兩者之間的操作
class PCnPD_Save: public TreeNodeBase<PCnPD_Save>
{
    IPersistChannel *m_pPC;
    std::shared_ptr<TreeNode<PersistData_String> > m_PD_ptr{nullptr};

    int m_lockResult;
public:
    // Constructor
    PCnPD_Save(const std::u8string &name) 
      :TreeNodeBase<PCnPD_Save>(name)
    {}

    // Destructor
    ~PCnPD_Save() 
    {
        if(m_lockResult != 0)
        {
            m_pPC->unlockMutex();
        }
    }

    // 初始化
    // 無法鎖住回覆 false
    bool cxxlFASTCALL init(IPersistChannel *pPC, 
        const std::shared_ptr<TreeNode<PersistData_String> > &PD_ptr)
    {
        m_lockResult = pPC->lockMutex();
        if(m_lockResult == 0)
            return false;

        m_pPC = pPC;
        m_PD_ptr = PD_ptr;

        // 第一次鎖住，須繼續建立子 PCnPD_Save
        if(m_lockResult == 1)        
        {
            // 新增一個名為 "_CLs" 的子節點，用來維護存放 IChildLinkChannel
            std::shared_ptr<TreeNode<PersistData_String> > CLs_ptr =
                m_PD_ptr->addChild(u8"_CLs");

            // 用來維護存放 IChildLinkChannel 的子節點的順序，並作為子節點的名稱
            size_t linkIndex = 0;

            const std::list<IChildLinkChannel *>& childLinks = m_pPC->getChildLinks();
            for(auto link_it = childLinks.begin(); link_it != childLinks.end(); ++link_it)
            {
                // 創建存放 IChildLinkChannel 子節點名稱
                std::u8string linkName(u8"_link");
                linkName += (const char8_t *)(std::to_string(linkIndex++).c_str());
                // 創建存放 IChildLinkChannel 的子節點
                std::shared_ptr<TreeNode<PersistData_String> > CPs_ptr = CLs_ptr->addChild(linkName);

                // 用來維護存放 IPersistChannel 的子節點的順序，並作為子節點的名稱
                size_t PersistIndex = 0;

                // 取得 IChildLinkChannel 包裹的 IPersistChannel
                std::list<IPersistChannel *> &PCs = (*link_it)->getChildPersistables();
                for(auto PC_it = PCs.begin(); PC_it != PCs.end(); ++PC_it)
                {
                    // 創建存放 IPersistChannel 子節點名稱
                    std::u8string childName(u8"_child");
                    childName += (const char8_t *)(std::to_string(PersistIndex++).c_str());
                    // 創建存放 IPersistChannel 的子節點
                    std::shared_ptr<TreeNode<PersistData_String> > child_PD_ptr = CPs_ptr->addChild(childName);

                    // 創建子節點對應的 PCnPD_Save
                    std::shared_ptr<PCnPD_Save> child_PCnPD_ptr = addChild(linkName + childName);
                    if (!child_PCnPD_ptr->init(*PC_it, child_PD_ptr)) 
                        return false;

                }
            }
        }

        return true;
    }
        
};

    
// 宣告在 simple_persist_storage.cpp 中
PersistSaveResult cxxlFASTCALL simpPersist_save(IPersistChannel *pPersistable, 
    const std::shared_ptr<TreeNode<PersistData_String> > &PD_ptr, 
    const std::u8string &name)
{
    // 先試試提供的 TreeNode 是否已有名為 name 的子節點
    std::shared_ptr<TreeNode<PersistData_String> > PDroot_ptr = PD_ptr->addChild(name);
    if(!PDroot_ptr)
    {
        return PersistSaveResult::NAME_CONFLICT;
    }

    PCnPD_Save PCnPD_root(name); // 這裡名字不重要只要不重複就好
    if(!PCnPD_root.init(pPersistable,PDroot_ptr))
    {
        return PersistSaveResult::NOT_LOCKABLE;
    }

    return PersistSaveResult::SUCCESS;
    
}

}