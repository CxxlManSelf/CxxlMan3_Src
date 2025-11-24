/***********************************************************************
 * uniptr.hpp v1.1.6
 *
 * UniPtr<>  封裝 std::shared_ptr，取代 std::shared_ptr，用法
 *           類似 std::shared_ptr，以附加一些額外的處理
 *
 * 注意事項：沒做 std::mutex 保護，因為 UniPtr 定位在暫時性或參數的
 *          使用，不應拿來共用
 *
 * 用此代替 std::shared_ptr 的用意在加入 UniBaseDestructor 的處理，在單一
 * 線程下確保作為 Host 的 UniBase 會存在
 * 
 * Author:      CxxlMan
 * date:        2025 -
 ***********************************************************************/

#ifndef __CXXLCORE_UNIPTR_HPP_CxxlMan3
#define __CXXLCORE_UNIPTR_HPP_CxxlMan3

#include "unibase.hpp"

namespace CXXL
{

    template <typename UNIBASE>
    class UniPtr
    {
        std::shared_ptr<UNIBASE> m_uniBase_ptr;

        // get UniBase
        std::shared_ptr<UNIBASE> getUniBase() const
        {
            return m_uniBase_ptr;
        }

        // Destroy UniBase
        void destroyUniBase()
        {
            if(m_uniBase_ptr == nullptr) return;


            // tmp_ptr 確保 justAdd() 執行後 _UniBase 還活著
            std::shared_ptr<UniResourcePrivate::_UniBase>
                tmp_ptr(m_uniBase_ptr,
                    (UniResourcePrivate::_UniBase *)m_uniBase_ptr.get());

            m_uniBase_ptr.reset();

            tmp_ptr->justAdd(tmp_ptr);
        }


        // Constructor
        // 提供給共享處理內部使用
        UniPtr(const std::shared_ptr<UNIBASE> &uniBase_ptr) : m_uniBase_ptr(uniBase_ptr) {}

    public:

        // Constructor
        UniPtr(UNIBASE *pUniBase = nullptr)
            : m_uniBase_ptr(pUniBase)
        {
        }

        // Copy Constructor
        UniPtr(const UniPtr &uniPtr)
            : m_uniBase_ptr(uniPtr.m_uniBase_ptr)
        {
        }

        // Move Constructor
        UniPtr(UniPtr &&uniPtr) noexcept
            : m_uniBase_ptr(std::move(uniPtr.m_uniBase_ptr))
        {
        }

        // Destructor
        virtual ~UniPtr() noexcept
        {
            destroyUniBase();
        }

        // Copy Assignment
        UniPtr &operator=(const UniPtr &uniPtr)
        {
            if (this != &uniPtr)
            {
                destroyUniBase();
                m_uniBase_ptr = uniPtr.m_uniBase_ptr;
            }
            return *this;
        }

        // Move Assignment
        UniPtr &operator=(UniPtr &&uniPtr) noexcept
        {
            if (this != &uniPtr)
            {
                destroyUniBase();
                m_uniBase_ptr = std::move(uniPtr.m_uniBase_ptr);
            }
            return *this;
        }

        // reset
        void reset(UNIBASE *pUniBase = nullptr)
        {
            destroyUniBase();
            m_uniBase_ptr.reset(pUniBase);
        }

        // operator->
        [[nodiscard]] UNIBASE *operator->() const
        {
            return m_uniBase_ptr.get();
        }

        // operator*
        [[nodiscard]] UNIBASE &operator*() const
        {
            return *m_uniBase_ptr;
        }

        // operator bool
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return m_uniBase_ptr != nullptr;
        }

        // get UniBase
        [[nodiscard]] UNIBASE *get() const
        {
            return m_uniBase_ptr.get();
        }

        // 提供一個轉型機制
        // 先用 get() 取得 UniBase 後自行轉型
        // 再用此功能包裹成 UniPtr
        // 以使用相同的計數器
        template <typename T>
        [[nodiscard]] UniPtr<T> cast() const
        {
            return UniPtr<T>(std::shared_ptr<T>(m_uniBase_ptr, get()));
        }

        template <typename T>
        friend class UniPtr;

        template <typename T>
        friend class UniObserver;
    
        template <typename T>
        friend class UniOwner;

        
    };

}

#endif