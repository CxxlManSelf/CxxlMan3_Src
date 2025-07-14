#include <simple_persist_storage.hpp>

namespace CXXL
{

// 這個類別在存放 IPersistChannel 和 TreeNode<PersistData_String> 兩者對應的實作物件
// 並提供兩者之間的操作
class PCnPD_Save: public TreeNodeBase<PCnPD_Save>
{
    IPersistChannel *m_pPC;
    std::shared_ptr<TreeNode<PersistData_String> > m_PD_ptr{nullptr};
public:
    // Constructor
    PCnPD_Save(const std::u8string &name) 
      :TreeNodeBase<PCnPD_Save>(name),
      m_PD_ptr(nullptr)
    {}

    // 初始化
    // 無法鎖住回覆 false
    bool cxxlFASTCALL init(IPersistChannel *pPC, 
        const std::shared_ptr<TreeNode<PersistData_String> > &PD_ptr)
    {
        int lockResult = pPC->lockMutex();
        if(lockResult == 0)
            return false;

        m_pPC = pPC;
        m_PD_ptr = PD_ptr;

        if(lockResult == 1)
        {

        }

        return true;
    }
        
};

    
// 宣告在 simple_persist_storage.cpp 中
PersistSaveResult cxxlFASTCALL simpPersist_save(const IPersistChannel *pPersistable, 
    const std::shared_ptr<TreeNode<PersistData_String> > &treeNode_ptr, 
    const std::u8string &name)
{
    std::shared_ptr<TreeNode<PersistData_String> > root_ptr = treeNode_ptr->addChild(name);
    if(!root_ptr)
    {
        return PersistSaveResult::NAME_CONFLICT;
    }

    return PersistSaveResult::SUCCESS;
    
}

}