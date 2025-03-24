/************************************************************************************************
 * liferes.hpp v0.1.0
 *
 * LifeRes<>    生命資源，由此延伸出來的類別可以被安全共享，可以由 LifeOwner 的持有來決定物件的生存。
 *              可分為
 *              LifeRes<ALL> 須所有 LifeOwner 放棄持有才會被銷毀
 *              LifeRes<ONE> 只要有一個 LifeOwner 放棄持有就會被銷毀
 *              LifeRes<ALL> 和 LifeRes<ONE> 不可以多重繼承，在 DEBUG 模式下會進行檢查
 * LifeOwner    生命管理器，可安全的共用 LifeRes<>，可以藉由放棄來決定 LifeRes<> 物件的生存
 * LifeObserver 生命觀察者，可安全的共用 LifeRes<>，但不參與 LifeRes<> 的生存管理
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLCORE_LIFERES_HPP_CxxlMan3
#define __CXXLCORE_LIFERES_HPP_CxxlMan3

#include <unordered_set>
#include <mutex>

#include "liferesdestructor.hpp"

namespace CXXL
{

    template <typename LIFERES>
    class LifeObserver;

    template <typename LIFERES>
    class LifeOwner;


    // 在此宣告一些 private 類別
    class LifeResourcePrivate
    {
        class _OwnerObserverBase;

        // LifeRes<> 的基礎類別
        class _LifeRes: IDestroyable
        {
            mutable std::mutex m_LifeResMutex;

            // 用來標記 _LifeRes 物件是不是要銷毁了，被標記的物件不能再被 _OwnerObserverBase 持有
            mutable bool m_isDestroy = false;

            // 持有此物件的 _OwnerObserverBase 集合
            mutable std::unordered_set<const _OwnerObserverBase *>
                m_OwnerObserverSet;

            // 虛擬函數，用來通知延伸類別增加了一個 LifeOwner 持有者
            virtual void cxxlFASTCALL addOwner() const = 0;

            // 虛擬函數用來告知延伸者減少了一個 LifeOwner 持有者
            // 返回值為 true 表示此物件須要被標記為銷毀
            virtual bool cxxlFASTCALL removeOwner() const = 0;

            bool cxxlFASTCALL attach(const _OwnerObserverBase *pOwnerObserver) const
            {
                if(m_isDestroy)
                    return false;
                
                m_OwnerObserverSet.insert(pOwnerObserver);
                return true;
            }

            void cxxlFASTCALL detach(const _OwnerObserverBase *pOwnerObserver) const
            {
                m_OwnerObserverSet.erase(pOwnerObserver);
            }

        public:
            // Constructor
            _LifeRes() {}
            
            // Destructor
            virtual ~_LifeRes() {}


            bool cxxlFASTCALL attachObserver(const _OwnerObserverBase *pObserver) const
            {
                std::lock_guard<std::mutex> lock(m_LifeResMutex);
                return attach(pObserver);
            }

            void cxxlFASTCALL detachObserver(const _OwnerObserverBase *pObserver) const
            {
                std::lock_guard<std::mutex> lock(m_LifeResMutex);
                detach(pObserver);
            }

            bool cxxlFASTCALL attachOwner(const _OwnerObserverBase *pOwner) const
            {
                std::lock_guard<std::mutex> lock(m_LifeResMutex);
                if(!attach(pOwner))
                    return false;
                
                addOwner();
                return true;
            }

            void cxxlFASTCALL detachOwner(const _OwnerObserverBase *pOwner) const
            {
                std::lock_guard<std::mutex> lock(m_LifeResMutex);
                m_isDestroy = removeOwner();
            }

            // 叫用 detachOwner() 之後呼叫銷毁器檢查是否需要銷毀
            void cxxlFASTCALL checkDestroy() const
            {
                g_pLifeResDestructor->checkDestroy(const_cast<_LifeRes *>(this));
            }
        };

        // LifeOwner 和 LifeObserver 的基礎類別
        // 作為 m_pHost 和 m_lifeRes_ptr 的連結
        class _OwnerObserverBase
        {
            // 持有 m_lifeRes_ptr 的 _LifeRes
            const _LifeRes *const m_pHost;

        protected:
            // 要持有的 _LifeRes
            mutable std::shared_ptr<const _LifeRes> m_lifeRes_ptr;

            // Constructor
            _OwnerObserverBase(const _LifeRes *pHost) : m_pHost(pHost)
            {
            }

        public:
            // Destructor
            virtual ~_OwnerObserverBase() {}
        };

        template <typename LIFERES>
        friend class LifeObserver;

        template <typename LIFERES>
        friend class LifeOwner;
    
    };

    /*****************************************************************************/
    /*
    ** 生命觀察者，可安全的共用 LifeRes<>，但不參與 LifeRes<> 的生存管理
    ** LifeRes<> 要被結束會獲得通知
    */
    template <typename LIFERES>
    class LifeObserver : public LifeResourcePrivate::_OwnerObserverBase
    {
        mutable std::mutex m_LifeObserverMutex;
        void cxxlFASTCALL attachLifeRes(const std::shared_ptr<LIFERES> &lifeRes_ptr)
        {
            if (lifeRes_ptr)
            {
                if (lifeRes_ptr->attachObserver(this))
                {
                    std::lock_guard<std::mutex> lock(m_LifeObserverMutex);
                    m_LifeRes_ptr = lifeRes_ptr;
                }
            }

            m_LifeRes_ptr = nullptr;
        }

    public:
        // Constructor
        LifeObserver(const shared_ptr<LIFERES> &liferes_ptr ,const _LifeRes *pHost) 
          : _OwnerObserverBase(pHost) 
        {
            attachLifeRes(liferes_ptr);
        }

        // Destructor
        virtual ~LifeObserver() 
        {
            destroy();
        }

        // Setter
        void cxxlFASTCALL setLifeRes(const shared_ptr<LIFERES> &liferes_ptr)
        {
            destroy();
            attachLifeRes(liferes_ptr);
        }

        void cxxlFASTCALL destroy()
        {
            std::decltype(m_LifeRes_ptr) tmp_ptr;
            {
                std::lock_guard<std::mutex> lock(m_LifeObserverMutex);
                tmp_ptr = m_LifeRes_ptr;
                m_lifeRes_ptr = nullptr;
            }

            if (tmp_ptr)
                tmp_ptr->detachObserver(this);
        }

    };

    /*
    ** LifeOwner 生命管理器，可安全的共用 LifeRes<>，可以藉由放棄來決定 LifeRes<> 物件的生存
    ** LifeRes<> 要被結束會獲得通知
    */
    template <typename LIFERES>
    class LifeOwner : public LifeResourcePrivate::_OwnerObserverBase
    {
        mutable std::mutex m_LifeOwnerMutex;
        void cxxlFASTCALL attachLifeRes(const std::shared_ptr<LIFERES> &lifeRes_ptr)
        {
            if (lifeRes_ptr)
            {
                if (lifeRes_ptr->attachOwner(this))
                {
                    std::lock_guard<std::mutex> lock(m_LifeOwnerMutex);
                    m_LifeRes_ptr = lifeRes_ptr;
                }
            }

            m_LifeRes_ptr = nullptr;
        }

    public:
        // Constructor
        LifeOwner(const shared_ptr<LIFERES> &liferes_ptr ,const _LifeRes *pHost) 
          : _OwnerObserverBase(pHost) 
        {
            attachLifeRes(liferes_ptr);
        }

        // Destructor
        virtual ~LifeOwner() 
        {
            destroy();
        }

        // Setter
        void cxxlFASTCALL setLifeRes(const std::shared_ptr<LIFERES> &lifeRes_ptr)
        {
            destroy();
            attachLifeRes(lifeRes_ptr);
        }

        void cxxlFASTCALL destroy()
        {
            std::decltype(m_LifeRes_ptr) tmp_ptr;
            {
                std::lock_guard<std::mutex> lock(m_LifeOwnerMutex);
                tmp_ptr = m_LifeRes_ptr;
                m_lifeRes_ptr = nullptr;
            }
            if(tmp_ptr)
            {
                tmp_ptr->detachOwner(this);
                tmp_ptr->checkDestroy();
            }
        }

    };

}

#endif