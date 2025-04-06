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
#include <functional>
#include <memory>
#include <cassert>

#include "rmconst.hpp"
#include "liferesdestructor.hpp"



namespace CXXL
{

    // LifeRes 的 template 參數
    enum class LifeResType
    {
        ALL,
        ONE
    };

    // 會被特化為 LifeRes<ALL> 和 LifeRes<One>
    template <LifeResType T>
    class LifeRes;

    template <typename LIFERES>
    class LifeObserver;

    template <typename LIFERES>
    class LifeOwner;

    // 在此宣告一些 private 類別
    class LifeResourcePrivate
    {
        class _OwnerObserverBase;

        // LifeRes<> 的基礎類別
        class CXXLCORE_DLLEXPORT _LifeRes : public IDestroyable
        {
            union
            {
                struct
                {
                    bool cFlag : 1; // 已放入待刪佇列為 true，否則為 false
                    bool fFlag : 1; // 銷毀處理器已搜尋過為 true，否則為 false
                    bool rFlag : 1; // 本身是 rootLifeRes,或未放入過 _OwnerObserverBase 為 false，否則為 true
                    bool ldFlag : 1; // 銷毀處理器已判定須銷毀為 true
                    bool m_isDestroy : 1; // 用來標記 _LifeRes 物件是不是要銷毁了，被標記的物件不能再被 _OwnerObserverBase 持有
                   // bool pFlag : 1; // 放入待刪佇列的後端為 false，否則為 true
                };
                uint8_t allFlags = 0; // 用於快速清為 0
            };


            // 巡查是不是已經沒有未銷毀的 Host 存在
            bool cxxlFASTCALL checkNoHost();

            virtual bool cxxlFASTCALL LD_shouldDestroy() override final; // class IDestroyable

            virtual void cxxlFASTCALL LD_destroy() override final; // class IDestroyable

            virtual void cxxlFASTCALL LD_clearFlag() override final; // class IDestroyable

            std::mutex &m_LifeResMutex;


            // 持有此物件的 _OwnerObserverBase 集合
            std::unordered_set<const _OwnerObserverBase *>
                &m_OwnerObserverSet;

            // 虛擬函數，用來通知延伸類別增加了一個 LifeOwner 持有者
            virtual void cxxlFASTCALL addOwner() = 0;

            // 虛擬函數用來告知延伸者減少了一個 LifeOwner 持有者
            // 返回值為 true 表示此物件須要被標記為銷毀
            virtual bool cxxlFASTCALL removeOwner() = 0;


            bool cxxlFASTCALL attach(const _OwnerObserverBase *pOwnerObserver);

            void cxxlFASTCALL detach(const _OwnerObserverBase *pOwnerObserver);

        protected:
#ifndef NDEBUG
            // LifeRes<ALL> 和 LifeRes<ONE> 不能多重繼承
            bool chkLifeAll = false, chkLifeOne = false;
#endif

        public:
            // Constructor
            _LifeRes();

            // Destructor
            virtual ~_LifeRes();

            bool cxxlFASTCALL attachObserver(const _OwnerObserverBase *pObserver);

            void cxxlFASTCALL detachObserver(const _OwnerObserverBase *pObserver);

            bool cxxlFASTCALL attachOwner(const _OwnerObserverBase *pOwner);

            void cxxlFASTCALL detachOwner(const _OwnerObserverBase *pOwner);

            // 叫用 detachOwner() 之後呼叫銷毁器檢查是否需要銷毀
            // lifeRes_ptr 其實就是自己，只是為了有 std::shared_ptr 包裹，會交給銷毀器
            void cxxlFASTCALL checkDestroy(const std::shared_ptr<_LifeRes> &lifeRes_ptr);

        };

        // 所有 LifeOwner 放棄持有才會被銷毀
        // 強制 virtual 繼承 _LifeRes
        class _LifeResAll : virtual _LifeRes
        {
            // LifeOwner 的持有數，歸零即須銷毀
            size_t m_OwnerCount = 0;

            // 虛擬函數，用來通知延伸類別增加了一個 LifeOwner 持有者
            virtual void cxxlFASTCALL addOwner() override final
            {
                ++m_OwnerCount;
            }

            // 虛擬函數用來告知延伸者減少了一個 LifeOwner 持有者
            // 返回值為 true 表示此物件須要被標記為銷毀
            virtual bool cxxlFASTCALL removeOwner() override final
            {
                return --m_OwnerCount == 0;
            }

        protected:
            // Constructor
            _LifeResAll()
            {
#ifndef NDEBUG
                chkLifeAll = true;
                assert(!chkLifeOne && "LifeRes<ALL> and LifeRes<ONE> are multiple inheritance!");
#endif
            }
        public:

            // Destructor
            virtual ~_LifeResAll() {}
        };

        // 只要一個 LifeOwner 放棄持有就會被銷毀
        // 強制 virtual 繼承 _LifeRes
        class _LifeResOne : virtual _LifeRes
        {
            // 虛擬函數，用來通知延伸類別增加了一個 LifeOwner 持有者
            virtual void cxxlFASTCALL addOwner() override final
            {
            }

            // 虛擬函數用來告知延伸者減少了一個 LifeOwner 持有者
            // 返回值為 true 表示此物件須要被標記為銷毀
            virtual bool cxxlFASTCALL removeOwner() override final
            {
                return true;
            }

        protected:
            // Constructor
            _LifeResOne()
            {
#ifndef NDEBUG
                chkLifeOne = true;
                assert(!chkLifeAll && "LifeRes<ALL> and LifeRes<ONE> are multiple inheritance!");
#endif
            }
		public:
			// Destructor
			virtual ~_LifeResOne() {}
        };

        // LifeOwner 和 LifeObserver 的基礎類別
        // 作為 m_pHost 和 m_lifeRes_ptr 的連結
        class _OwnerObserverBase
        {
            // 持有 m_lifeRes_ptr 的 _LifeRes
            const _LifeRes *const m_pHost;

        protected:
            // Constructor
            _OwnerObserverBase(const _LifeRes *pHost) : m_pHost(pHost)
            {
            }

        public:
            // Destructor
            virtual ~_OwnerObserverBase() {}

            // 給 _LifeRes::destroy() 使用，pChkLifeRes 作為要檢查的 _LifeRes
            virtual void cxxlFASTCALL detachLifeRes(const _LifeRes *pChkLifeRes) const = 0;

            friend class _LifeRes;
        };

        template <typename LIFERES>
        friend class LifeObserver;

        template <typename LIFERES>
        friend class LifeOwner;

        template <LifeResType T>
        friend class LifeRes;
    };
   


    /*****************************************************************************/

    // 所有 LifeOwner 放棄持有才會被銷毀
    // 強制 virtual 繼承 _LifeResAll
    template <>
    class LifeRes<LifeResType::ALL> : public virtual LifeResourcePrivate::_LifeResAll
    {
	public:
		// Constructor
        LifeRes() {}
		// Destructor
        virtual ~LifeRes() {}

    };

    // 只要一個 LifeOwner 放棄持有就會被銷毀
    // 強制 virtual 繼承 _LifeResOne
    template <>
    class LifeRes<LifeResType::ONE> : public virtual LifeResourcePrivate::_LifeResOne
    {
	public:
        // Constructor
        LifeRes() {}
        // Destructor
        virtual ~LifeRes() {}
    };

    /*****************************************************************************/
    /*
    ** 生命觀察者，可安全的共用 LifeRes<>，但不參與 LifeRes<> 的生存管理
    ** LifeRes<> 要被結束會獲得通知
    */
    template <typename LIFERES>
    class LifeObserver : LifeResourcePrivate::_OwnerObserverBase
    {
        // 使用端要做好 _OwnerObserverBase 的同步控制，以及放棄持有 _LifeRes。
        // pChkLifeRes 作為要檢查 _LifeRes，判斷持有的 _LifeRes 是否和要被放棄的 pChkLifeRes 相同
        std::function<void(const LIFERES *pChkLifeRes)> m_detachLifeResFunc;

        // 要持有的 LifeRes
        mutable std::shared_ptr<LIFERES> m_lifeRes_ptr;

        // 給 _LifeRes::destroy() 使用，pChkLifeRes 作為要檢查的 _LifeRes
        virtual void cxxlFASTCALL detachLifeRes(const LifeResourcePrivate::_LifeRes *pChkLifeRes) 
          const override final
        {
            const LIFERES *pLifeRes = dynamic_cast<const LIFERES *>(pChkLifeRes);
            m_detachLifeResFunc(pLifeRes);
        }

        void cxxlFASTCALL attachLifeRes(const std::shared_ptr<LIFERES> &lifeRes_ptr)
        {
            if (lifeRes_ptr)
            {
                auto pLifeRes = (LifeResourcePrivate::_LifeRes *)lifeRes_ptr.get();                
                if (pLifeRes->attachObserver(this))
                {
                    m_lifeRes_ptr = lifeRes_ptr;
                    return;
                }
            }

            m_lifeRes_ptr = nullptr;
        }

        // Constructor
        LifeObserver(const std::shared_ptr<LIFERES> &liferes_ptr,
                     const LifeResourcePrivate::_LifeRes *pHost,
                     const std::function<void(const LIFERES *pChkLifeRes)> &detachLifeResFunc)
            : _OwnerObserverBase(pHost),
              m_detachLifeResFunc(detachLifeResFunc)
        {
            attachLifeRes(liferes_ptr);
        }

    public:

        // Constructor
        template <typename HOST>
        LifeObserver(const std::shared_ptr<LIFERES> &liferes_ptr,
                     HOST *pHost,
                     const std::function<void(const LIFERES *pChkLifeRes)> &detachLifeResFunc)
          : LifeObserver(liferes_ptr, (const LifeResourcePrivate::_LifeRes *)pHost, detachLifeResFunc)
        {
            
        }

        // Destructor
        virtual ~LifeObserver()
        {
            destroy();
        }

        // Setter
        void cxxlFASTCALL setLifeRes(const std::shared_ptr<LIFERES> &liferes_ptr)
        {
            destroy();
            attachLifeRes(liferes_ptr);
        }

        // 銷毀
        void cxxlFASTCALL destroy()
        {
            if(m_lifeRes_ptr == nullptr) return;

            auto pLifeRes = (LifeResourcePrivate::_LifeRes *)m_lifeRes_ptr.get();
            pLifeRes->detachObserver(this);
            m_lifeRes_ptr.reset();
        }
    };

    /*
    ** LifeOwner 生命管理器，可安全的共用 LifeRes<>，可以藉由放棄來決定 LifeRes<> 物件的生存
    ** LifeRes<> 要被結束會獲得通知
    */
    template <typename LIFERES>
    class LifeOwner : LifeResourcePrivate::_OwnerObserverBase
    {
        // 使用端要做好 _OwnerObserverBase 的同步控制，以及放棄持有 _LifeRes。
        // pChkLifeRes 作為要檢查 _LifeRes，判斷持有的 _LifeRes 是否和要被放棄的 pChkLifeRes 相同
        std::function<void(const LIFERES *pChkLifeRes)> m_detachLifeResFunc;

        // 要持有的 LifeRes
        mutable std::shared_ptr<LIFERES> m_lifeRes_ptr;

        // 給 _LifeRes::destroy() 使用，pChkLifeRes 作為要檢查的 _LifeRes
        virtual void cxxlFASTCALL detachLifeRes(const LifeResourcePrivate::_LifeRes *pChkLifeRes)
          const override final
        {
            const LIFERES *pLifeRes = dynamic_cast<const LIFERES *>(pChkLifeRes);
            m_detachLifeResFunc(pLifeRes);
        }

        void cxxlFASTCALL attachLifeRes(const std::shared_ptr<LIFERES> &lifeRes_ptr)
        {
            if (lifeRes_ptr)
            {
                auto pLifeRes = (LifeResourcePrivate::_LifeRes *)lifeRes_ptr.get();
                if (pLifeRes->attachOwner(this))
                {
                    m_lifeRes_ptr = lifeRes_ptr;
                    return;
                }
            }

            m_lifeRes_ptr = nullptr;
        }

        // Constructor
        LifeOwner(const std::shared_ptr<LIFERES> &liferes_ptr, 
                  const LifeResourcePrivate::_LifeRes *pHost,
                  const std::function<void(const LIFERES *pChkLifeRes)> &detachLifeResFunc)
            : _OwnerObserverBase(pHost),
              m_detachLifeResFunc(detachLifeResFunc)
        {
            attachLifeRes(liferes_ptr);
        }

    public:

        // Constructor
        template <typename HOST> 
        LifeOwner(const std::shared_ptr<LIFERES> &liferes_ptr, 
                  HOST *pHost,
                  const std::function<void(const LIFERES *pChkLifeRes)> &detachLifeResFunc)
          : LifeOwner(liferes_ptr, (const LifeResourcePrivate::_LifeRes *)pHost, detachLifeResFunc)
        {
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

        // 銷毀
        void cxxlFASTCALL destroy()
        {
            if (m_lifeRes_ptr == nullptr) return;

            // tmp_ptr 確保 checkDestroy() 執行後 _LifeRes 還活著
            std::shared_ptr<LifeResourcePrivate::_LifeRes> tmp_ptr(m_lifeRes_ptr,
                (LifeResourcePrivate::_LifeRes *)m_lifeRes_ptr.get() );

            m_lifeRes_ptr.reset();

            tmp_ptr->detachOwner(this);
            tmp_ptr->checkDestroy(tmp_ptr);
        }
    };

}

#endif