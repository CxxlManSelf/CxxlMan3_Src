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
#include <treenode.hpp>

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
        virtual void cxxlFASTCALL operator()(char8_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::int8_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::int16_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::int32_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::int64_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::uint8_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::uint16_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::uint32_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::uint64_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::float32_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::float64_t *p, size_t count, const std::u8string &name) = 0;
        virtual void cxxlFASTCALL operator()(std::float128_t *p, size_t count, const std::u8string &name) = 0;
    };

    // ISerializeLoad() 的回傳值
    enum class SerializeLoadResult
    {
        LOAD_SUCCESS, // 讀取階段下成功
        CHK_SUCCESS, // 檢查階段下成功
        CHK_FAILED // 檢查階段下失敗
    };

    // Persistable 執行永續儲存的 Load 序列化介面
    class ISerializeLoad
    {
    public:
        virtual ~ISerializeLoad() {}

        // 是否處於檢查階段
        // virtual bool cxxlFASTCALL isChecking() = 0;

        
        // p: 用來獲得永續資料陣列，檢查階段不會用到
        // count: 在讀取階段用來獲得永續資料陣列的長度
        //        在檢查階段用來指定要檢查的永續資料長度，若指定為 0 表示不檢查
        // name: 要讀取的永續資料名稱，只有在檢查階段才有意義
        //
        // 若無資料 p 會指向 nullptr，count 會為 0
        virtual SerializeLoadResult cxxlFASTCALL operator()(char8_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::int8_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::int16_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::int32_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::int64_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::uint8_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::uint16_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::uint32_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::uint64_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::float32_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::float64_t **p, size_t &count, const std::u8string &name) = 0;
        virtual SerializeLoadResult cxxlFASTCALL operator()(std::float128_t **p, size_t &count, const std::u8string &name) = 0;
        
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

        virtual void cxxlFASTCALL save(ISerializeSave &SS) = 0;

        // 只要有一個使用 SL 得到 SerializeLoadResult::CHK_FAILED 回覆，就應回傳 false
        virtual bool cxxlFASTCALL load(const ISerializeLoad &SL) = 0;
    };

    // 和 _ChildLink 的溝通介面
    class IChildLinkChannel
    {        
    public:
        virtual ~IChildLinkChannel() {}
        
        virtual std::list<IPersistChannel *>& cxxlFASTCALL getChildPersistables() const = 0;
    };

} // namespace CXXL

#endif // __CXXLPERSIST_PERSIST_STORAGE_HPP_CxxlMan3