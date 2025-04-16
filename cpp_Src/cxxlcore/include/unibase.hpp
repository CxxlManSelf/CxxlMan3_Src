/************************************************************************************************
 * unibase.hpp v1.1.21
 *
 * UniBase<>    統一基礎，由此延伸出來的類別可以被安全共享，可以由 UniOwner 的持有來決定物件的是否
 *              結束共用。可分為
 *              UniBase<ALL> 須所有 UniOwner 放棄持有才會被結束共用
 *              UniBase<ONE> 只要有一個 UniOwner 放棄持有就會被結束共用
 *              注意！UniBase<ALL> 和 UniBase<ONE> 不可以多重繼承，在 DEBUG 模式下會進行檢查
 * UniOwner     UniBase 的管理器，可安全的共用 UniBase<>，可以藉由放棄來決定 UniBase<> 物件
 *              的是否結束共用
 * UniObserver  UniBase 的觀察者，可安全的共用 UniBase<>，但不參與 UniBase<> 的共用管理
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLCORE_UNIBASE_HPP_CxxlMan3
#define __CXXLCORE_UNIBASE_HPP_CxxlMan3

#include <unordered_set>
#include <mutex>
#include <functional>
#include <memory>
#include <cassert>

#include "rmconst.hpp"
#include "unibasedestructor.hpp"

namespace CXXL
{

    // UniBase 的 template 參數
    enum class UniBaseType
    {
        ALL,
        ONE
    };

    // 會被特化為 UniBase<ALL> 和 UniBase<One>
    template <UniBaseType T>
    class UniBase;

    template <typename UNIBASE>
    class UniObserver;

    template <typename UNIBASE>
    class UniOwner;

    // 在此宣告一些 private 類別
    class UniResourcePrivate
    {
        class _OwnerObserverBase;

        // UniBase<> 的基礎類別
        class CXXLCORE_DLLEXPORT _UniBase : public IDestroyable
        {
            union
            {
                struct
                {
                    bool cFlag : 1;       // 已放入待刪佇列為 true，否則為 false
                    bool fFlag : 1;       // 結束共用處理器已搜尋過為 true，否則為 false
                    bool rFlag : 1;       // 本身是 rootUniBase,或未放入過 _OwnerObserverBase 為 false，否則為 true
                    bool ldFlag : 1;      // 結束共用處理器已判定須結束共用為 true
                    bool m_isDestroy : 1; // 用來標記 _UniBase 物件是不是要結束共用了，被標記的物件不能再
                                          // 被 _OwnerObserverBase 持有
                };
                uint8_t allFlags = 0; // 用於快速清為 0
            };

            // 巡查是不是已經沒有 root 的 Host 存在
            bool cxxlFASTCALL checkNoHost();

            virtual bool cxxlFASTCALL LD_shouldDestroy() override final; // class IDestroyable

            virtual void cxxlFASTCALL LD_destroy() override final; // class IDestroyable

            virtual void cxxlFASTCALL LD_clearFlag() override final; // class IDestroyable

            mutable std::mutex m_UniBaseMutex;

            // 持有此物件的 _OwnerObserverBase 集合
            std::unordered_set<const _OwnerObserverBase *>
                m_OwnerObserverSet;

            // 虛擬函數，用來通知延伸類別增加了一個 UniOwner 持有者
            virtual void cxxlFASTCALL addOwner() = 0;

            // 虛擬函數用來告知延伸者減少了一個 UniOwner 持有者
            // 返回值為 true 表示此物件須要被標記為結束共用
            virtual bool cxxlFASTCALL removeOwner() = 0;

            bool cxxlFASTCALL attach(const _OwnerObserverBase *pOwnerObserver);

            void cxxlFASTCALL detach(const _OwnerObserverBase *pOwnerObserver);

        protected:
#ifndef NDEBUG
            // UniBase<ALL> 和 UniBase<ONE> 不能多重繼承
            bool chkUniAll = false, chkUniOne = false;
#endif

            bool cxxlFASTCALL attachObserver(const _OwnerObserverBase *pObserver);

            void cxxlFASTCALL detachObserver(const _OwnerObserverBase *pObserver);

            bool cxxlFASTCALL attachOwner(const _OwnerObserverBase *pOwner);

            void cxxlFASTCALL detachOwner(const _OwnerObserverBase *pOwner);
            void cxxlFASTCALL detachMoveOwner(const _OwnerObserverBase *pOwner); // 不做銷毁標記

            // 叫用 detachOwner() 之後呼叫銷毁器檢查是否需要結束共用
            // UniBase_ptr 其實就是自己，只是為了有 std::shared_ptr 包裹，會交給結束共用處理器
            void cxxlFASTCALL checkDestroy(const std::shared_ptr<_UniBase> &uniBase_ptr);

        public:
            // Constructor
            _UniBase();

            // Destructor
            virtual ~_UniBase();

            // 檢查是否已經標記為結束共用
            bool cxxlFASTCALL isDestroy() const;

            template <typename UNIBASE>
            friend class UniObserver;

            template <typename UNIBASE>
            friend class UniOwner;
        };

        // 所有 UniOwner 放棄持有才會被結束共用
        // 強制 virtual 繼承 _UniBase
        class _UniBaseAll : virtual public _UniBase
        {
            // UniOwner 的持有數，歸零即須銷毀
            size_t m_OwnerCount = 0;

            // 虛擬函數，用來通知延伸類別增加了一個 UniOwner 持有者
            virtual void cxxlFASTCALL addOwner() override final
            {
                ++m_OwnerCount;
            }

            // 虛擬函數用來告知延伸者減少了一個 UniOwner 持有者
            // 返回值為 true 表示此物件須要被標記為結束共用
            virtual bool cxxlFASTCALL removeOwner() override final
            {
                return --m_OwnerCount == 0;
            }

        protected:
            // Constructor
            _UniBaseAll()
            {
#ifndef NDEBUG
                chkUniAll = true;
                assert(!chkUniOne && "UniBase<ALL> and UniBase<ONE> are multiple inheritance!");
#endif
            }

        public:
            // Destructor
            virtual ~_UniBaseAll() {}
        };

        // 只要一個 UniOwner 放棄持有就會被結束共用
        // 強制 virtual 繼承 _UniBase
        class _UniBaseOne : virtual public _UniBase
        {
            // 虛擬函數，用來通知延伸類別增加了一個 UniOwner 持有者
            virtual void cxxlFASTCALL addOwner() override final
            {
            }

            // 虛擬函數用來告知延伸者減少了一個 UniOwner 持有者
            // 返回值為 true 表示此物件須要被標記為結束共用
            virtual bool cxxlFASTCALL removeOwner() override final
            {
                return true;
            }

        protected:
            // Constructor
            _UniBaseOne()
            {
#ifndef NDEBUG
                chkUniOne = true;
                assert(!chkUniAll && "UniBase<ALL> and UniBase<ONE> are multiple inheritance!");
#endif
            }

        public:
            // Destructor
            virtual ~_UniBaseOne() {}
        };

        // UniOwner 和 UniObserver 的基礎類別
        // 作為 m_pHost 和 m_uniBase_ptr 的連結
        class _OwnerObserverBase
        {
            // 持有 m_uniBase_ptr 的 _UniBase
            const _UniBase *const m_pHost;

        protected:
            // Constructor
            _OwnerObserverBase(const _UniBase *pHost) : m_pHost(pHost)
            {
            }

            // move constructor
            _OwnerObserverBase(_OwnerObserverBase &&Other) noexcept
                : m_pHost(Other.m_pHost) 
            {}

        public:
            // Destructor
            virtual ~_OwnerObserverBase() {}

            // 給 _UniBase::destroy() 使用，pChkUniBase 作為要檢查的 _UniBase
            virtual void cxxlFASTCALL detachUniBase(const _UniBase *pChkUniBase) const = 0;

            friend class _UniBase;
        };

        template <typename UNIBASE>
        friend class UniObserver;

        template <typename UNIBASE>
        friend class UniOwner;

        template <UniBaseType T>
        friend class UniBase;
    };

    /*****************************************************************************/

    // 所有 UniOwner 放棄持有才會被結束共用
    // 強制 virtual 繼承 _UniBaseAll
    template <>
    class UniBase<UniBaseType::ALL> : public virtual UniResourcePrivate::_UniBaseAll
    {
    public:
        // Constructor
        UniBase() {}
        // Destructor
        virtual ~UniBase() {}
    };

    // 只要一個 UniOwner 放棄持有就會被結束共用
    // 強制 virtual 繼承 _UniBaseOne
    template <>
    class UniBase<UniBaseType::ONE> : public virtual UniResourcePrivate::_UniBaseOne
    {
    public:
        // Constructor
        UniBase() {}
        // Destructor
        virtual ~UniBase() {}
    };

    /*****************************************************************************/
    /*
    ** UniBase 觀察者，可安全的共用 UniBase<>，但不參與 UniBase<> 的生存管理
    ** UniBase<> 要被結束共用會獲得通知
    */
    template <typename UNIBASE>
    class UniObserver : UniResourcePrivate::_OwnerObserverBase
    {
        // 使用端要做好 _OwnerObserverBase 的同步控制，以及放棄持有 _UniBase。
        // pChkUniBase 作為要檢查的 _UniBase，判斷持有的 _UniBase 是否和要被放棄的 pChkUniBase 相同
        // pSender 發出通知的 UniObserver
        std::function<void(UniObserver<UNIBASE> *pSender, void *pChkUniBase)> m_detachUniBaseFunc;

        // 要持有的 UniBase
        mutable std::shared_ptr<UNIBASE> m_uniBase_ptr;

        // 給 _UniBase::destroy() 使用，pChkUniBase 作為要檢查的 _UniBase
        virtual void cxxlFASTCALL detachUniBase(const UniResourcePrivate::_UniBase *pChkUniBase)
            const override final
        {
            m_detachUniBaseFunc((UniObserver<UNIBASE> *)this, (void *)pChkUniBase);
        }

        bool cxxlFASTCALL attachUniBase(const std::shared_ptr<UNIBASE> &uniBase_ptr)
        {
            if (uniBase_ptr)
            {
                auto pUniBase = (UniResourcePrivate::_UniBase *)uniBase_ptr.get();
                if (pUniBase->attachObserver(this))
                {
                    m_uniBase_ptr = uniBase_ptr;
                    return true;
                }
            }

            m_uniBase_ptr = nullptr;
            return false;
        }

    public:
        // Constructor
        template <typename HOST>
        UniObserver(HOST *pHost,
                     const std::function<void(UniObserver<UNIBASE> *pSender, void *pChkUniBase)> &detachUniBaseFunc)
            : _OwnerObserverBase(pHost)
        {
            m_detachUniBaseFunc = detachUniBaseFunc;
        }

        // move constructor
        UniObserver(UniObserver &&other)
            : _OwnerObserverBase(std::move(other))
        {
            m_detachUniBaseFunc = other.m_detachUniBaseFunc;
            attachUniBase(other.m_uniBase_ptr);
            other.destroy();
        }

        // Destructor
        virtual ~UniObserver()
        {
            destroy();
        }

        // Setter
        // 若成功被加入則回傳 true
        // 若 uniBase_ptr 被標示為結束共用狀態則不會被加入，改設定為 nullptr，且回傳 false
        bool cxxlFASTCALL setUniBase(const std::shared_ptr<UNIBASE> &uniBase_ptr)
        {
            destroy();
            return attachUniBase(uniBase_ptr);
        }

        // Getter
        std::shared_ptr<UNIBASE> cxxlFASTCALL getUniBase() const
        {
            return m_uniBase_ptr;
        }

        // 結束共用
        void cxxlFASTCALL destroy()
        {
            if (m_uniBase_ptr == nullptr)
                return;

            auto pUniBase = (UniResourcePrivate::_UniBase *)m_uniBase_ptr.get();
            pUniBase->detachObserver(this);
            m_uniBase_ptr.reset();
        }

        // 給使用端檢查 pChkUniBase 是不是和持有的 UniBase 相匹配
        bool cxxlFASTCALL chkUniBase(void *pChkUniBase) const
        {
            if (m_uniBase_ptr == nullptr)
                return false;

            const UniResourcePrivate::_UniBase *p = (const UniResourcePrivate::_UniBase *)m_uniBase_ptr.get();
            void *pUniBase = const_cast<void*>(static_cast<const void*>(p));

            return pUniBase == pChkUniBase;
        }

        UniObserver(const UniObserver &) = delete;
        UniObserver &operator=(const UniObserver &) = delete;
        UniObserver &operator=(UniObserver &&) = delete;
    };

    /*
    ** UniBase 的管理器，可安全的共用 UniBase<>，可以藉由放棄來決定 UniBase<> 物件的生存
    ** UniBase<> 要被結束共用會獲得通知
    */
    template <typename UNIBASE>
    class UniOwner : UniResourcePrivate::_OwnerObserverBase
    {
        // 使用端要做好 _OwnerObserverBase 的同步控制，以及放棄持有 _UniBase。
        // pChkUniBase 作為要檢查的 _UniBase，判斷持有的 _UniBase 是否和要被放棄的 pChkUniBase 相同
        // pSender 發出通知的 UniOwner
        std::function<void(UniOwner<UNIBASE> *pSender, void *pChkUniBase)> m_detachUniBaseFunc;

        // 要持有的 UniBase
        mutable std::shared_ptr<UNIBASE> m_uniBase_ptr;

        bool m_isMoved = false; // 標記是否已被移動

        // 給 _UniBase::destroy() 使用，pChkUniBase 作為要檢查的 _UniBase
        virtual void cxxlFASTCALL detachUniBase(const UniResourcePrivate::_UniBase *pChkUniBase)
            const override final
        {
            m_detachUniBaseFunc((UniOwner<UNIBASE> *)this, (void *)pChkUniBase);
        }

        bool cxxlFASTCALL attachUniBase(const std::shared_ptr<UNIBASE> &uniBase_ptr)
        {
            if (uniBase_ptr)
            {
                auto pUniBase = (UniResourcePrivate::_UniBase *)uniBase_ptr.get();
                if (pUniBase->attachOwner(this))
                {
                    m_uniBase_ptr = uniBase_ptr;
                    return true;
                }
            }

            m_uniBase_ptr = nullptr;
            return false;
        }

    public:
        // Constructor
        template <typename HOST>
        UniOwner(HOST *pHost,
                  const std::function<void(UniOwner<UNIBASE> *pSender, void *pChkUniBase)> &detachUniBaseFunc)
            : _OwnerObserverBase(pHost)
        {
            m_detachUniBaseFunc = detachUniBaseFunc;
        }

        // move constructor
        UniOwner(UniOwner &&other)
            : _OwnerObserverBase(std::move(other))
        {
            m_detachUniBaseFunc = other.m_detachUniBaseFunc;
            attachUniBase(other.m_uniBase_ptr);
            other.m_isMoved = true;
            other.destroy();
        }

        // Destructor
        virtual ~UniOwner()
        {
            destroy();
        }

        // Setter
        // 若成功被加入則回傳 true
        // 若 uniBase_ptr 被標示為結束共用狀態則不會被加入，改設定為 nullptr，且回傳 false
        bool cxxlFASTCALL setUniBase(const std::shared_ptr<UNIBASE> &uniBase_ptr)
        {
            destroy();
            return attachUniBase(uniBase_ptr);
        }

        // Getter
        std::shared_ptr<UNIBASE> cxxlFASTCALL getUniBase() const
        {
            return m_uniBase_ptr;
        }

        // 結束共用
        void cxxlFASTCALL destroy()
        {

            if (m_uniBase_ptr == nullptr)
            return;

            // 檢查是否已被移動
            if (m_isMoved)
            {
                UniResourcePrivate::_UniBase *pUniBase = (UniResourcePrivate::_UniBase *)m_uniBase_ptr.get();

                pUniBase->detachMoveOwner(this); // 不做結束共用
                m_uniBase_ptr.reset();
            }
            else
            {
                // tmp_ptr 確保 checkDestroy() 執行後 _UniBase 還活著
                std::shared_ptr<UniResourcePrivate::_UniBase> 
                  tmp_ptr(m_uniBase_ptr,
                          (UniResourcePrivate::_UniBase *)m_uniBase_ptr.get());

                m_uniBase_ptr.reset();

                tmp_ptr->detachOwner(this);
                tmp_ptr->checkDestroy(tmp_ptr);
            }
        }

        // 給使用端檢查 pChkUniBase 是不是和持有的 UniBase 相匹配
        bool cxxlFASTCALL chkUniBase(void *pChkUniBase) const
        {
            if (m_uniBase_ptr == nullptr)
                return false;

            const UniResourcePrivate::_UniBase *p = (const UniResourcePrivate::_UniBase *)m_uniBase_ptr.get();
            void *pUniBase = const_cast<void*>(static_cast<const void*>(p));

            return pUniBase == pChkUniBase;
        }

        UniOwner(const UniOwner &) = delete;
        UniOwner &operator=(const UniOwner &) = delete;
        UniOwner &operator=(UniOwner &&) = delete;
    };

}

#endif