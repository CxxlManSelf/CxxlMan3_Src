/***********************************************************************
 * uniptr.hpp v1.0.0
 *
 * UniPtr<>  封裝 std::shared_ptr，取代 std::shared_ptr，用法
 *           類似 std::shared_ptr，以附加一些額外的處理
 *
 * 注意事項：沒做 std::mutex 保護，因為 UniPtr 定位在暫時性或參數的
 *          使用，不應拿來共用
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


            // tmp_ptr 確保 onlyAdd() 執行後 _UniBase 還活著
            std::shared_ptr<UniResourcePrivate::_UniBase>
                tmp_ptr(m_uniBase_ptr,
                    (UniResourcePrivate::_UniBase *)m_uniBase_ptr.get());

            m_uniBase_ptr.reset();

            tmp_ptr->onlyAdd(tmp_ptr);
        }


        // Constructor
        UniPtr(const std::shared_ptr<UNIBASE> &uniBase_ptr) : m_uniBase_ptr(uniBase_ptr) {}

    public:

        // Constructor
        UniPtr(UNIBASE *pUniBase)
            : m_uniBase_ptr(pUniBase)
        {
        }

        // Copy Constructor
        UniPtr(const UniPtr &uniPtr)
            : m_uniBase_ptr(uniPtr.m_uniBase_ptr)
        {
        }

        // Destructor
        virtual ~UniPtr()
        {
            destroyUniBase();
        }

        // Assignment
        UniPtr &operator=(const UniPtr &uniPtr)
        {
            destroyUniBase();
            m_uniBase_ptr = uniPtr.m_uniBase_ptr;
            return *this;
        }

        // reset
        void reset(UNIBASE *pUniBase = nullptr)
        {
            destroyUniBase();
            m_uniBase_ptr.reset(pUniBase);
        }

        // operator->
        UNIBASE *operator->() const
        {
            return m_uniBase_ptr.get();
        }

        // operator*
        UNIBASE &operator*() const
        {
            return *m_uniBase_ptr;
        }

        // operator bool
        operator bool() const
        {
            return m_uniBase_ptr != nullptr;
        }

        // get UniBase
        UNIBASE *get() const
        {
            return m_uniBase_ptr.get();
        }

        template <typename T>
        friend class UniObserver;
    
        template <typename T>
        friend class UniOwner;

        
    };

}

#endif