/*****************************************************************************
 * persist_storage.hpp v0.1.0
 *
 * IPersistable 的永續資料儲存體的標準介面
 *
 * author: CxxlMan
 * date: 2025 -
 *****************************************************************************/
#ifndef __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3
#define __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3

#include <cstdint>
#include <stdfloat>
#include <list>

#include "sysdef.hpp"

namespace CXXL
{
    class IChildLinkChannel;

    // IPersistStorage 和 _Persistable 的溝通介面
    class IPersistChannel
    {
        virtual std::list<IChildLinkChannel *>& cxxlFASTCALL getChildLinks() const = 0;
        virtual void cxxlFASTCALL lock() = 0;
        virtual void cxxlFASTCALL unlock() = 0;

    public:
        virtual ~IPersistChannel() {}
    };

    // 和 _ChildLink 的溝通介面
    class IChildLinkChannel
    {
        
        virtual std::list<IPersistChannel *> cxxlFASTCALL getPersistable() const = 0;
    public:
        virtual ~IChildLinkChannel() {}
    };

    // Persistable 執行永續儲存的序列化介面
    // 實作分為 SAVE 與 LOAD 兩種型態
    class ISerialize
    {
    public:
        virtual ~ISerialize() {}

        // 序列化函數，存取同型可用 type() 來判別
        // 在 SAVE 型態回傳值為 false 表示遇到 null pointer 的情況
        // 在 LOAD 型態回傳值為 false 表示失敗，pPersistable 的資料不會改變
        // 只要有一個失敗 Persistable::doPersist() 就應回傳 false
        virtual bool cxxlFASTCALL operator()(char8_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::int8_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::int16_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::int32_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::int64_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::uint8_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::uint16_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::uint32_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::uint64_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::float32_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::float64_t *p, size_t count, const std::u8string &name) = 0;
        virtual bool cxxlFASTCALL operator()(std::float128_t *p, size_t count, const std::u8string &name) = 0;
        
        // 序列化類型
        enum SerializeType
        {
            SAVE,
            LOAD
        };

        virtual SerializeType cxxlFASTCALL type() const = 0;
    };

    // 永續資料儲存體的介面
    class IPersistStorage
    {
    public:
        virtual ~IPersistStorage() {}

        // 保存永續儲存物件的資料
        // 若回傳值為 false 表示失敗，遇到了 pPersistable 有 null pointer 的情況
        virtual bool cxxlFASTCALL save(IPersistChannel *pPersistable) = 0;

        // 取回永續儲存物件的資料
        virtual bool cxxlFASTCALL load(IPersistChannel *pPersistable) = 0;
    };

} // namespace CXXL

#endif // __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3