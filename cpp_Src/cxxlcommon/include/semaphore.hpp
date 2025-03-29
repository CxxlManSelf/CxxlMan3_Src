/************************************************************************************************
 * semaphore.hpp v1.0.4
 *
 * 提供一個 semaphore 功能，這個 semaphore 可以設定遇到進入 block 狀態
 * 前，會先呼叫由使用端提供的回叫函數
 *
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLCOMMON_SEMAPHORE_HPP_CxxlMan3
#define __CXXLCOMMON_SEMAPHORE_HPP_CxxlMan3

#include <mutex>
#include <condition_variable>

#include "commondef.hpp"
#include "sysdef.hpp"

namespace CXXL
{

    class cxxlSemaphore final
    {
        const size_t m_maxThread = 0; // 最多可多少 thread 允許通行，0 表示不限
        size_t m_numThread = 0; // 有多少 thread 允許通行，若設為 0，表示 block
        std::mutex m_semaphore_mutex;
        std::condition_variable m_condition;

    public:
        // Constructor
        // maxThread = 最多可多少 thread 允許通行，0 表示不限
        // numThread = 設定一開始有多少 thread 允許通行，若設為 0，表示一開始會先被 block
        // 內定只有一個執行緒可以通行並設為 block
        cxxlSemaphore(size_t maxThread = 1, size_t numThread = 0)
            : m_maxThread(maxThread), m_numThread(numThread)
        {
        }

        ~cxxlSemaphore() = default;

        // 等待取得執行權
        // 不提供 blockEvent()
        void cxxlFASTCALL wait()
        {
            auto WaitEventlambda = []() {};
            wait(WaitEventlambda);
        }

        // 等待取得執行權
        // 若要進入 block 前，會先呼叫 blockEvent()
        template <typename F>
        void cxxlFASTCALL wait(F blockEvent)
        {
            std::unique_lock<std::mutex> lock(m_semaphore_mutex);
            m_condition.wait(lock,
                             [&]() -> bool
                             { return (m_numThread > 0) ? true : (blockEvent(), false); });
            --m_numThread;
        }

        // 釋放一個被 block 的執行緒
        void cxxlFASTCALL release()
        {
            std::lock_guard<std::mutex> lock(m_semaphore_mutex);
            if (m_maxThread == 0 || m_numThread < m_maxThread)
            {
                ++m_numThread;
                m_condition.notify_one();
            }
        }

        // 讓 m_numThread 歸 0，即設為 block 狀態
        void cxxlFASTCALL zero()
        {
            std::lock_guard<std::mutex> lock(m_semaphore_mutex);
            m_numThread = 0;
        }

        cxxlSemaphore(const cxxlSemaphore &) = delete;
        cxxlSemaphore &operator=(const cxxlSemaphore &) = delete;
        cxxlSemaphore(cxxlSemaphore &&) = delete;
        cxxlSemaphore &operator=(cxxlSemaphore &&) = delete;
    };

    // 輔助類別，在一個區塊內
    // 幫助自動叫用 cxxlSemaphore::wait() 和 cxxlSemaphore::release()，
    // 對於有回傳值的函數必須使用，以確保 return 指令之後會叫
    // 用 cxxlSemaphore::release()
    class cxxlSemaphoreHelper final
    {
        cxxlSemaphore &m_semaphore;

    public:
        cxxlSemaphoreHelper(cxxlSemaphore &semaphore) 
          : m_semaphore(semaphore)
        {
            m_semaphore.wait();
        }

        ~cxxlSemaphoreHelper()
        {
            m_semaphore.release();
        }
    };

}    

#endif // __CXXLCOMMON_SEMAPHORE_HPP_CxxlMan3

    /************************************************************************************************
     * End of semaphore.hpp
     ************************************************************************************************/
