/*****************************************************************************
 * persistence.hpp v0.1.0
 * 
 * CxxlMan3 採用的永續儲存標準介面約定
 * 
 * IPersistable   提供物件的永續儲存功能
 * ChildLink      IPersistable 可含有子 IPersistable，但要用 ChildLink 來連接
 * ISerializable  IPersistable 永續儲存的存取介面
 *  
 * 
 * Author: CxxlMan
 * Date: 2025 -
******************************************************************************/
#ifndef __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3
#define __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3

#include <memory>
#include <list>

#include "cxxlpersist.hpp"
#include "uniptr.hpp"
#include "persist_storage.hpp"

namespace CXXL
{

    template<UniBaseType T>
    class IPersistable;

    // 在此宣告一些 private 類別
    class PersistResourcePrivate
    {
        class _ChildLink;

        // Persistable 的基底類別
        // 負責和儲存體溝通
        class _Persistable: public IPersistChannel
        {
            std::list<_ChildLink *> m_childLinks; // 子物件集合
        public:
            virtual ~_Persistable() {}

            friend class _ChildLink;
        };

        class _ChildLink
        {
            _Persistable *m_pPersistable; // 包裹子物件

        public:
            _ChildLink(_Persistable *persistable_ptr, _Persistable *pHost) 
                : m_pPersistable(persistable_ptr) 
            {
                pHost->m_childLinks.push_back(this);
            }
            virtual ~_ChildLink() {}
        };

        template<UniBaseType T>
        friend class IPersistable;
    };

    // Persistable 執行永緒儲存的序列化介面
    // 實作分為 SAVE 與 LOAD 兩種型態
    class ISerializable
    {
    public:
        virtual ~ISerializable() {}

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
    class IPersistable :virtual public UniBase<T>, virtual public PersistResourcePrivate::_Persistable
    {
    protected:
        // 執行永緒儲存
        virtual bool cxxlFASTCALL
        doPersist(ISerializable *pSerializable) = 0;

    public:
        // Constructor
        IPersistable() = default;

        // Destructor
        virtual ~IPersistable() {}

        
    };

    // IPersistable 和子 IPersistable 的連接關係
    template <typename T>
    class ChildLink: public PersistResourcePrivate::_ChildLink
    {
    public:
        // Constructor
        template <typename H>
        ChildLink(const UniPtr<T> &child, const H *pHost)        
        {}
        
    };
    
    

}

#endif // __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3