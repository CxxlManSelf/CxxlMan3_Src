/*****************************************************************************
 * persistence.hpp v0.1.0
 * 
 * CxxlMan3 採用的永續儲存標準介面約定
 * 
 * Author: CxxlMan
 * Date: 2025 -
******************************************************************************/
#ifndef __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3
#define __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3

#include <memory>

#include "cxxlpersist.hpp"

namespace CXXL
{

    class Persistable;

    // 在此宣告一些 private 類別
    class PersistResourcePrivate
    {
        // Persistable 的真正實作基底類別
        class _Persistable
        {
        };

        friend class Persistable;
    };

    // Persistable 執行永緒儲存的序列化介面
    class ISerializable
    {
    public:
        virtual ~ISerializable() {}
        
        // 序列化類型
        enum PersistType
        {
            SAVE,
            LOAD
        };

        virtual PersistType cxxlFASTCALL type() const = 0;
    };

    /**
     * 可永久儲存的物件基礎類別
     * 所有需要永久儲存的物件都應該繼承此類別
    **/
    class Persistable : virtual public PersistResourcePrivate::_Persistable
    {
    protected:
        // 執行永緒儲存
        virtual bool cxxlFASTCALL
        doPersist(ISerializable *pSerializable) = 0;

    public:
        // Constructor
        Persistable() = default;

        // Destructor
        virtual ~Persistable() {}

        
    };

    // 容器物件的操作介面
    class IPersistContainer
    {
    };

    // 可永久儲存的物件的儲存體
    class IPersistStorage
    {

    protected:
        // Constructor
        IPersistStorage() = default;


    public:

        // Destructor
        virtual ~IPersistStorage() {}

        // 保存永緒儲存物件的資料
        virtual bool cxxlFASTCALL save(Persistable *pPersistable) = 0;

        // 取回永緒儲存物件的資料
        // 注意！若失敗，pPersistable 的資料會毀損
        virtual bool cxxlFASTCALL load(Persistable *pPersistable) = 0;
    };

    // 取得 IPersistStorage 的預設實作
    std::shared_ptr<IPersistStorage> cxxlFASTCALL
    CXXLPERSIST_DLLEXPORT defaultPersistStorage(const std::shared_ptr<IPersistContainer> &container);
}

#endif // __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3