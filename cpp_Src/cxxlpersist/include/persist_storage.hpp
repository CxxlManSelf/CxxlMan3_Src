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

#include <stdfloat>
#include <list>

#include "sysdef.hpp"

namespace CXXL
{
    class IChildLinkChannel;

    // IPersistStorage 和 _Persistable 的溝通介面
    class IPersistChannel
    {
        virtual std::list<IChildLinkChannel *> cxxlFASTCALL getChildLinks() const = 0;
        virtual void cxxlFASTCALL lock() = 0;
        virtual void cxxlFASTCALL unlock() = 0;

    public:
        virtual ~IPersistChannel() {}
    };

    // IPersistStorage 和 _ChildLink 的溝通介面
    class IChildLinkChannel
    {
        virtual IPersistChannel *getPersistable() const = 0;
    public:
        virtual ~IChildLinkChannel() {}
    };

    // Persistable 執行永緒儲存的序列化介面
    // 實作分為 SAVE 與 LOAD 兩種型態
    class ISerialize
    {
    public:
        virtual ~ISerialize() {}

        // 序列化函數，存取同型，可用 type() 來判別
        // 在 SAVE 型態，回傳值為 true
        // 只有在 LOAD 型態，回傳值才有意義，若有一個失敗 Persistable::doPersist() 就
        // 就應回傳 false
        virtual bool cxxlFASTCALL operator()(int8_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(int16_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(int32_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(int64_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(uint8_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(uint16_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(uint32_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(std::uint64_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(std::float32_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(std::float64_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(std::float128_t *p, size_t count) = 0;
        
        // 序列化類型
        enum SerializeType
        {
            SAVE,
            LOAD
        };

        virtual SerializeType cxxlFASTCALL type() const = 0;
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

} // namespace CXXL

#endif // __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3