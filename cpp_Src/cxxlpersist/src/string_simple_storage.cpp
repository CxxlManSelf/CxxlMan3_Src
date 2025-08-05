#include <simple_persist_storage.hpp>
#include "PCnPD.hpp"

namespace CXXL
{
    
// 宣告在 simple_persist_storage.cpp 中
PersistSaveResult cxxlFASTCALL simpPersist_save(IPersistChannel *pPersistable, 
    const std::shared_ptr<TreeNode<PersistData_String> > &PD_ptr, 
    const std::u8string &name)
{
    // 名字不可為空
    if(name.empty())
        return PersistSaveResult::NAME_EMPTY_OR_EXIST;

    // 建立儲存永續資料的子節點
    std::shared_ptr<TreeNode<PersistData_String> > PDroot_ptr = PD_ptr->addChild(name);
    // 提供的 PD_ptr 是否已有名為 name 的子節點
    if(!PDroot_ptr)
        return PersistSaveResult::NAME_EMPTY_OR_EXIST;

    PCnPD_Save<PersistData_String> PCnPD_root(u8"PCnPD_root"); 
    if(!PCnPD_root.init(pPersistable,PDroot_ptr))
    {
        return PersistSaveResult::NOT_LOCKABLE;
    }

    // 執行永續儲存    
    PCnPD_root.save();

    return PersistSaveResult::SUCCESS;
    
}

// 宣告在 simple_persist_storage.cpp 中
PersistLoadResult cxxlFASTCALL simpPersist_load(IPersistChannel *pPersistable, 
    const std::shared_ptr<const TreeNode<PersistData_String> > &PD_ptr, 
    const std::u8string &name)
{
    // 取得儲存永續資料的子節點
    std::shared_ptr<const TreeNode<PersistData_String> > PDroot_ptr = PD_ptr->findChildByName(name);
    if(!PDroot_ptr)
        return PersistLoadResult::NAME_NOT_FOUND;

    PCnPD_Load<PersistData_String> PCnPD_root(u8"PCnPD_root");
    PersistLoadResult initResult = PCnPD_root.init(pPersistable,PDroot_ptr);
    if(initResult != PersistLoadResult::SUCCESS)
    {
        return initResult;
    }

    if(PCnPD_root.check() == false)
    {
        return PersistLoadResult::LOAD_FAILED;
    }

    PCnPD_root.load();
        
    return PersistLoadResult::SUCCESS;
}


}