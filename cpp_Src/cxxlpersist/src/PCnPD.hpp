/************************************************************************************************
 * PCnPD.hpp v0.1.0
 *
 * 這個類別在存放 IPersistChannel 和 TreeNode<T> 兩者對應的實作物件
 * 並提供兩者之間的操作
 *
 * T 
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLPERSIST_PCNPD_HPP_CxxlMan3
#define __CXXLPERSIST_PCNPD_HPP_CxxlMan3

#include <persist_storage.hpp>
#include "SerializeSave.hpp"

namespace CXXL
{

// T 為 TreeNode<> 包裹的類別，比如 PersistData_String
template<typename T>
class PCnPD_Save:public TreeNodeBase<PCnPD_Save<T> >
{
    IPersistChannel *m_pPC;
    std::shared_ptr<TreeNode<T> > m_PD_ptr;
    int m_lockResult;

public:
    // Constructor
    PCnPD_Save(const std::u8string &name) 
      :TreeNodeBase<PCnPD_Save<T> >(name)
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
        const std::shared_ptr<TreeNode<T> > &PD_ptr)
    {
        m_lockResult = pPC->lockMutex();
        if(m_lockResult == 0)
            return false;

        m_pPC = pPC;
        m_PD_ptr = PD_ptr;

        // 第一次鎖住，須繼續建立子 PCnPD_Save
        if(m_lockResult == 1)
        {
            // 新增一個名為 "_CLs" 的子節點，用來存放 m_pPC 的各個 IChildLinkChannel
            std::shared_ptr<TreeNode<T> > CLs_ptr =
                m_PD_ptr->addChild(u8"_CLs");

            // m_pPC 的所有 IChildLinkChannel 
            const std::list<IChildLinkChannel *>& childLink_list = m_pPC->getChildLinks();
            for(auto link_it = childLink_list.begin(); link_it != childLink_list.end(); ++link_it)
            {
                // 創建一個存放 IChildLinkChannel 的子節點，用來存放其 IPersistChannel 陣列
                std::shared_ptr<TreeNode<T> > CPs_ptr = CLs_ptr->addChild(u8"");

                // 取得 IChildLinkChannel 包裹的 IPersistChannel 列表
                std::list<IPersistChannel *> &PC_list = (*link_it)->getChildPersistables();
                for(auto PC_it = PC_list.begin(); PC_it != PC_list.end(); ++PC_it)
                {
                    // 創建存放 IPersistChannel 的子節點
                    std::shared_ptr<TreeNode<T> > child_PD_ptr = CPs_ptr->addChild(u8"");

                    // 創建子節點對應的 PCnPD_Save
                    std::shared_ptr<PCnPD_Save> child_PCnPD_ptr = addChild(u8"");
                    if (!child_PCnPD_ptr->init(*PC_it, child_PD_ptr)) 
                        return false;
                }
            }
        }

        return true;
    }

    // 保存 IPersistChannel
    // 先深後廣
    void cxxlFASTCALL save()
    {
        // 先保存子節點
        const std::list<std::shared_ptr<PCnPD<T> > > &childrens = getChildren();
        for(auto child_it = childrens.begin(); child_it != childrens.end(); ++child_it)
        {
            if(!(*child_it)->save())
        }

        // 再保存自己
        m_pPC->save( SerializeSave<T>(m_PD_ptr) );
    }
};

// T 為 TreeNode<> 包裹的類別，比如 PersistData_String
template<typename T>
class PCnPD_Load:public TreeNodeBase<PCnPD_Load<T> >
{
    IPersistChannel *m_pPC;
    std::shared_ptr<TreeNode<T> > m_PD_ptr;
    int m_lockResult;

public:
    // Constructor
    PCnPD_Load(const std::u8string &name)
        : TreeNodeBase<PCnPD_Load<T> >(name)
    {}

    // 初始化
    PersistLoadResult cxxlFASTCALL init(IPersistChannel *pPC, 
        const std::shared_ptr<TreeNode<T> > &PD_ptr)
    {
        m_lockResult = pPC->lockMutex();
        if(m_lockResult == 0)
            return PersistLoadResult::NOT_LOCKABLE;

        m_pPC = pPC;
        m_PD_ptr = PD_ptr;

        // 第一次鎖住，須繼續建立子 PCnPD_Load
        if(m_lockResult == 1)
        {
            // 取得名為 "_CLs" 的子節點，其內含有 m_pPC 的各個 IChildLinkChannel
            std::shared_ptr<TreeNode<T> > CLs_ptr = m_PD_ptr->getChild(u8"_CLs");
            if(!CLs_ptr) return PersistLoadResult::DATA_FORMAT_CORRUPT;
             
            // m_pPC 的所有 IChildLinkChannel 
            const std::list<IChildLinkChannel *>& childLink_list = m_pPC->getChildLinks();

            if(CLs_ptr-childCount() != childLink_list.size())
                return PersistLoadResult::DATA_NOT_MATCH;            
            
            const std::list<std::shared_ptr<TreeNode<T> > >& CLs_list = CLs_ptr->getChildren();
            // CLs_list 的 iterator
            auto CLs_it = CLs_list.begin();

            for(auto link_it = childLinks.begin(); link_it != childLinks.end(); ++link_it)
            {
                // 取得一個存放 IChildLinkChannel 的子節點，其內存放其 IPersistChannel 陣列
                std::shared_ptr<TreeNode<T> > CPs_ptr = *(CLs_it++);

                // 取得一個 IChildLinkChannel 包裹的 IPersistChannel
                std::list<IPersistChannel *> &PC_list = (*link_it)->getChildPersistables();
                const std::list<std::shared_ptr<TreeNode<T> > > &CP_list = CPs_ptr->getChildren();

                if(CP_list.size() != PC_list.size())
                    return PersistLoadResult::DATA_NOT_MATCH;

                // CP_list 的 iterator
                auto CP_it = CP_list.begin();

                for(auto PC_it = PC_list.begin(); PC_it != PC_list.end(); ++PC_it)
                {
                    // 取得存放 IPersistChannel 的子節點
                    std::shared_ptr<TreeNode<T> > child_PD_ptr = *(CP_it++);

                    // 創建子節點對應的 PCnPD_Load
                    std::shared_ptr<PCnPD_Load> child_PCnPD_ptr = addChild(u8"");
                    PersistLoadResult result = child_PCnPD_ptr->init(*PC_it, child_PD_ptr);
                    if(result != PersistLoadResult::SUCCESS) 
                        return result;                    
                }
            }
        }

        return PersistLoadResult::SUCCESS;
    }



};

} // namespace CXXL

#endif // __CXXLPERSIST_PCNPD_HPP_CxxlMan3
