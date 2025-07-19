/*****************************************************************************
 * persist_storage.hpp v0.1.0
 *
 * 永續資料儲存體和 IPersistable 溝通的的標準介面
 *
 * author: CxxlMan
 * date: 2025 -
 *****************************************************************************/
#ifndef __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3
#define __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3

#include <cstdint>
#include <stdfloat>
#include <list>
#include <string>

#include "sysdef.hpp"

namespace CXXL
{
    class IChildLinkChannel;

    // Persistable 執行永續儲存的 Save 序列化介面
    class ISerializeSave
    {
    public:
        virtual ~ISerializeSave() {}

        // 回傳值為 false 表示名稱未指定或已存在或是 p 為 null
        // 只要有一個失敗，IPersistable::Save() 就應回傳 false
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
    };

    // Persistable 執行永續儲存的 Load 序列化介面
    class ISerializeLoad
    {
    public:
        virtual ~ISerializeLoad() {}

        // 回傳值為 false 表示失敗，pPersistable 的資料不會改變
        // 只要有一個失敗 IPersistable::Load() 就應回傳 false
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
        
    };

    // IPersistStorage 和 _Persistable 的溝通介面
    class IPersistChannel
    {
    public:
        virtual ~IPersistChannel() {}

        virtual const std::list<IChildLinkChannel *>& cxxlFASTCALL getChildLinks() const = 0;

        // lock 失敗回覆 0
        // lock 成功回覆 1
        // 成功又再 lock 回覆 2
        virtual int cxxlFASTCALL lockMutex() = 0;

        // 呼叫端須管控好
        // lockMutex() 成功(回覆非 0)必須呼叫
        // lockMutex() 失敗絕對不能呼叫
        virtual void cxxlFASTCALL unlockMutex() = 0;

        // 只要有一個使用 ISerializeSave& 儲存失敗就應回傳 false
        virtual bool cxxlFASTCALL save(ISerializeSave &) = 0;

        virtual bool cxxlFASTCALL load(const ISerializeLoad &) = 0;
    };

    // 和 _ChildLink 的溝通介面
    class IChildLinkChannel
    {        
    public:
        virtual ~IChildLinkChannel() {}
        
        virtual std::list<IPersistChannel *>& cxxlFASTCALL getChildPersistables() const = 0;
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
        // 若回傳值為 false 表示失敗
        virtual bool cxxlFASTCALL load(IPersistChannel *pPersistable) = 0;
    };

} // namespace CXXL

#endif // __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3