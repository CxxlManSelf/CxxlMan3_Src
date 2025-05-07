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
#include "uniptr.hpp"

namespace CXXL
{

    class Persistable;
    class IPersistStorage;

    // 在此宣告一些 private 類別
    class PersistResourcePrivate
    {
        // Persistable 的真正實作基底類別
        class _Persistable
        {
        public:
            virtual ~_Persistable() {}
        };

        class _PersistStorage
        {
        public:
            virtual ~_PersistStorage() {}
        };


        friend class Persistable;
        friend class IPersistStorage;
    };

    // Persistable 執行永緒儲存的序列化介面
    class ISerializable
    {
    public:
        virtual ~ISerializable() {}

        // 序列化函數，存取同型，可用 type() 來判別
        // 只有在 LOAD 型態，回傳值才有意義，若有一個失敗 Persistable::doPersist() 就
        // 就應回傳 false
        virtual bool cxxlFASTCALL operator()(int8_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(int16_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(int32_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(int64_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(uint8_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(uint16_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(uint32_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(uint64_t *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(float *p, size_t count) = 0;
        virtual bool cxxlFASTCALL operator()(double *p, size_t count) = 0;
        
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
    template<UniBaseType T>   
    class Persistable :virtual public UniBase<T>, virtual public PersistResourcePrivate::_Persistable
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

    // Persistable 和子 Persistable 的連接關係
    template <typename T>
    class ChildLink
    {
    public:
        // Constructor
        template <typename H>
        ChildLink(const UniPtr<T> &child, const H *pHost)
        {}
        
    };
    
    

    // 容器物件的操作介面
    class IPersistContainer
    {
    public:
        virtual ~IPersistContainer() {}


    };

    // 可永久儲存的物件的儲存體
    class IPersistStorage:virtual public PersistResourcePrivate::_PersistStorage
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