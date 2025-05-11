/*****************************************************************************
 * persist_storage.hpp v0.1.0
 * 
 * IPersistStorage  IPersistable 的永續儲存資料保存與讀取
 * 
 * author: CxxlMan
 * date: 2025 -
*****************************************************************************/
#ifndef __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3
#define __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3

#include "sysdef.hpp"

namespace CXXL
{
    // IPersistStorage 和 IPersistable 的溝通介面
    class IPersistChannel
    {
    public:
        virtual ~IPersistChannel() {}
    };

    // 永緒儲存資料保存與讀取
    class IPersistStorage
    {
    public:
        virtual ~IPersistStorage() {}

        // 保存永緒儲存物件的資料
        virtual bool cxxlFASTCALL save(IPersistChannel *pPersistable) = 0;

        // 取回永緒儲存物件的資料
        // 注意！若失敗，pPersistable 的資料會毀損
        virtual bool cxxlFASTCALL load(IPersistChannel *pPersistable) = 0;

    };

    // 繼承自 IPersistStorage 的簡易版實作類別
    class LitePersistStorage : public IPersistStorage
    {
        
    }

    // 取得 IPersistStorage 的預設實作
    std::shared_ptr<IPersistStorage> cxxlFASTCALL
    CXXLPERSIST_DLLEXPORT defaultPersistStorage(const std::shared_ptr<IPersistContainer> &container);


} // namespace CXXL

#endif // __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3