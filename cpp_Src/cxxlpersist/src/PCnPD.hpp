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

// 負責將具有永續資料的 m_pPC 物件，儲存到永續資料儲存容器 m_PD_ptr
// T 為 m_PD_ptr 這個永續資料儲存容器要包裹的類別，比如 PersistData_String
template<typename T>
class PCnPD_Save:public TreeNodeBase<PCnPD_Save<T> >
{
    IPersistChannel *m_pPC; // 具有永續資料儲存能力的物件
    std::shared_ptr<TreeNode<T> > m_PD_ptr; // 永續資料儲存容器
    int m_lockResult;  // m_pPC 鎖住的狀態

public:
    // Constructor
    // name 是 TreeNode<> 的要求，不具有意義
    PCnPD_Save(const std::u8string &name) 
      :TreeNodeBase<PCnPD_Save<T> >(name)
    {}

    // Destructor
    ~PCnPD_Save() 
    {
        // 有鎖住過的話就要解鎖
        if(m_lockResult != 0)
        {
            m_pPC->unlockMutex();
        }
    }

    // 初始化
    // 並為 m_pPC 的子物件，在 m_PD_ptr 中建立相應的儲存容器
    // 子節點，並交給子 PCnPD_Save 處理初始化
    // 無法鎖住回覆 false
    bool cxxlFASTCALL init(IPersistChannel *pPC, 
        const std::shared_ptr<TreeNode<T> > &PD_ptr)
    {
        m_lockResult = pPC->lockMutex();
        if(m_lockResult == 0)
            return false;

        // 能鎖住才有續續的處理
        m_pPC = pPC;
        m_PD_ptr = PD_ptr;

        // 第一次鎖住，須繼續建立子 PCnPD_Save
        // 否則表示 m_pPC 已處理過，不繼續處理其子物件
        if(m_lockResult == 1)
        {
            // 儲存容器中新增一個名為 "_CLs" 的子節點，用來存放 m_pPC 的各個 IChildLinkChannel
            std::shared_ptr<TreeNode<T> > CLs_ptr =
                m_PD_ptr->addChild(u8"_CLs");

            // m_pPC 的所有 IChildLinkChannel 
            const std::list<IChildLinkChannel *>& childLink_list = m_pPC->getChildLinks();

            // 取出 m_pPC 的子物件和建立對應的永續資料儲存子容器
            for(auto link_it = childLink_list.begin(); link_it != childLink_list.end(); ++link_it)
            {
                // 儲存容器中創建一個存放 IChildLinkChannel 的子節點，用來存放其 IPersistChannel 陣列
                std::shared_ptr<TreeNode<T> > CPs_ptr = CLs_ptr->addChild(u8"");

                // 取得 IChildLinkChannel 包裹的 IPersistChannel 列表
                std::list<IPersistChannel *> &PC_list = (*link_it)->getChildPersistables();

                // 為每個 m_pPC 的子物件和對應的永續資料儲存子容器，建立
                // 子 PCnPD_Save，並交給子 PCnPD_Save 初始化
                for(auto PC_it = PC_list.begin(); PC_it != PC_list.end(); ++PC_it)
                {
                    // 儲存容器中創建一個存放 IPersistChannel 的子節點
                    std::shared_ptr<TreeNode<T> > child_PD_ptr = CPs_ptr->addChild(u8"");

                    // 創建一個處理子節點的子 PCnPD_Save
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
        const std::list<std::shared_ptr<PCnPD_Save<T> > > &children_list = getChildren();
        for(auto child_it = children_list.begin(); child_it != children_list.end(); ++child_it)
        {
            (*child_it)->save();
        }

        // 再保存自己

        // 儲存容器中新增一個名為 "_ATTRs" 的子節點，用來保存 m_pPC 的屬性       
        m_pPC->save( SerializeSave<T>(m_PD_ptr->addChild(u8"_ATTRs")) );
    }
};

// 負責將永續資料儲存容器 m_PD_ptr 中的資料，取回到 m_pPC
// T 為 m_PD_ptr 這個永續資料儲存容器要包裹的類別，比如 PersistData_String
template<typename T>
class PCnPD_Load:public TreeNodeBase<PCnPD_Load<T> >
{
    IPersistChannel *m_pPC; // 具有永續資料儲存能力的物件
    std::shared_ptr<const TreeNode<T> > m_PD_ptr; // 永續資料儲存容器
    int m_lockResult; // m_pPC 鎖住的狀態

    SerializeLoad<T> m_serializeLoad;

public:
    // Constructor
    // name 是 TreeNode<> 的要求，不具有意義
    PCnPD_Load(const std::u8string &name)
        : TreeNodeBase<PCnPD_Load<T> >(name)
    {}

    // Destructor
    virtual ~PCnPD_Load() 
    {
        // 有鎖住過的話就要解鎖
        if(m_lockResult != 0)
        {
            m_pPC->unlockMutex();
        }
    }

    // 初始化
    // 為 m_pPC 的子物件，找出在 m_PD_ptr 中對應的永續資料儲
    // 存子容器，並交給子 PCnPD_Load 處理初始化
    PersistLoadResult cxxlFASTCALL init(IPersistChannel *pPC, 
        const std::shared_ptr<const TreeNode<T> > &PD_ptr)
    {
        m_lockResult = pPC->lockMutex();
        if(m_lockResult == 0)
            return PersistLoadResult::NOT_LOCKABLE;

        // 能鎖住才有續續的處理
        m_pPC = pPC;
        m_PD_ptr = PD_ptr;

        // 第一次鎖住，須繼續建立子 PCnPD_Load
        // 否則表示 m_pPC 已處理過，不繼續處理其子物件
        if(m_lockResult == 1)
        {
            // 取得儲存容器中名為 "_CLs" 的子節點，其內含有 m_pPC 的各個 IChildLinkChannel
            std::shared_ptr<TreeNode<T> > CLs_ptr = m_PD_ptr->getChild(u8"_CLs");
            if(!CLs_ptr) return PersistLoadResult::DATA_FORMAT_CORRUPT;
             
            // m_pPC 的所有 IChildLinkChannel
            const std::list<IChildLinkChannel *>& childLink_list = m_pPC->getChildLinks();

            // IChildLinkChannel 數量不一致
            if(CLs_ptr-childCount() != childLink_list.size())
                return PersistLoadResult::DATA_NOT_MATCH;            
            
            const std::list<std::shared_ptr<TreeNode<T> > >& CLs_list = CLs_ptr->getChildren();
            // CLs_list 的 iterator
            auto CLs_it = CLs_list.begin();

            // 取出 m_pPC 的子物件和對應的永續資料儲存子容器
            for(auto link_it = childLinks.begin(); link_it != childLinks.end(); ++link_it)
            {
                // 取得儲存容器中一個存放 IChildLinkChannel 的子節點，其內存放其 IPersistChannel 陣列
                std::shared_ptr<TreeNode<T> > CPs_ptr = *(CLs_it++);

                // 取得一個 IChildLinkChannel 包裹的 IPersistChannel 列表
                std::list<IPersistChannel *> &PC_list = (*link_it)->getChildPersistables();
                const std::list<std::shared_ptr<TreeNode<T> > > &CP_list = CPs_ptr->getChildren();

                // IPersistChannel 的數量不一致
                if(CP_list.size() != PC_list.size())
                    return PersistLoadResult::DATA_NOT_MATCH;

                // CP_list 的 iterator
                auto CP_it = CP_list.begin();

                // 為每個 m_pPC 的子物件和對應的永續資料儲存子容器，建立
                // 子 PCnPD_Load，並交給子 PCnPD_Load 處理初始化
                for(auto PC_it = PC_list.begin(); PC_it != PC_list.end(); ++PC_it)
                {
                    // 取得儲存容器中一個存放 IPersistChannel 的子節點
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

    // 進行檢查階段，先深後廣
    // 只有父子孫節點都檢查通過才算成功
    bool cxxlFASTCALL check()
    {
        // 先檢查子節點
        const std::list<std::shared_ptr<PCnPD_Load<T> > > &children_list = getChildren();
        for(auto child_it = children_list.begin(); child_it != children_list.end(); ++child_it)
        {
            if( (*child_it)->check() == false )
                return false;
        }

        // 再檢查自己

        // 從儲存容器中名為 "_ATTRs" 的子節點，取得 m_pPC 保存的屬性
        std::shared_ptr<const TreeNode<T> > ATTRs_ptr = m_PD_ptr->getChild(u8"_ATTRs");
        if(ATTRs_ptr == nullptr) 
            return false;

        m_serializeLoad.setPD(ATTRs_ptr);

        return m_pPC->load( m_serializeLoad );
    }

    // 執行讀取階段，先深後廣    
    void cxxlFASTCALL load()
    {
        // 先讀取子節點
        const std::list<std::shared_ptr<PCnPD_Load<T> > > &children_list = getChildren();
        for(auto child_it = children_list.begin(); child_it != children_list.end(); ++child_it)
        {
            (*child_it)->load();
        }

        // 再讀取自己
        m_serializeLoad.setLoadMode();
        m_pPC->load( m_serializeLoad );
    }

};

} // namespace CXXL

#endif // __CXXLPERSIST_PCNPD_HPP_CxxlMan3
