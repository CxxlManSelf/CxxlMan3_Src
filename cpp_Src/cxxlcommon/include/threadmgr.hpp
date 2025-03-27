/****************************************************************************************
 * threadmgr.hpp v0.1.0
 *
 *  提供兩個執行緒的管理功能
 *
 *  ThreadLimiter 有任務時才會創建執行緒，可以指定最多的執行緒活動數量，任務減少到低於執行緒
 *                的最大活動數量，沒事做的執行緒就會結束。這樣比較節省執行緒的使用量，但是多
 *                了重複創造執行緒所須的時間
 *
 *  ThreadPool 準備好指定的執行緒數量，ThreadPool 未喂結束前執行緒不會結束，這樣可以節省
 *             創建執行緒的時間。但是未占用執行緒的數量，系統所能提供的執行緒數量是有限的
 *
 *  任務可以是有參數和回傳值的函數，用 std::packaged_task 包裝，這樣可以在兩個執
 *  行緒之間做溝通
 *
 * Author: CxxlMan
 * Date: 2025 -
 ****************************************************************************************/

#ifndef __CXXLCOMMON_THREADMGR_HPP_CxxlMan3
#define __CXXLCOMMON_THREADMGR_HPP_CxxlMan3

#include <thread>
#include <vector>
#include <future>
#include <functional>
#include <queue>

#include "commondef.hpp"
#include "sysdef.hpp"
#include "semaphore.hpp"

namespace CXXL
{

    // 機動增減執行緒的數量
    class ThreadLimiter
    {
        std::mutex m_task_mutex;
        std::queue<std::function<void()>> m_tasks; // 待處理的任務

        std::atomic_bool m_isStop = false; // 表示所有執行緒都結束

        size_t m_maxThreads;     // 最大執行緒數量
        size_t m_numThreads = 0; // 目前執行緒數量

        // 雙重驗證
        cxxlSemaphore m_isOver;    // 等待所有 thread 都結束
        bool m_isOverFlag = false; // 所有 thread 都結束了

        // 由 thredProc() 呼叫
        // 取出一個任務，若回覆 false 則 thredProc 會結束
        // 若已是最後一個 thredProc 的呼叫，還會通知所有 thread 結束
        bool cxxlFASTCALL getTask(std::function<void(void)> &Func)
        {
            std::lock_guard<std::mutex> lock(m_task_mutex);
            if (m_tasks.empty())
            {
                --m_numThreads; // 表示有一個執行緒會結束

                if (m_numThreads == 0) // 所有執行緒都結束
                {
                    m_isOverFlag = true;
                    m_isOver.release();
                }

                return false;
            }
            Func = std::move(m_tasks.front());
            m_tasks.pop();
            return true;
        }

        // std::thread 要使用的函數
        void cxxlFASTCALL thredProc()
        {
            std::function<void(void)> Func;
            while (true)
            {
                if (!getTask(Func))
                    break;
                Func();
            }
        }

    public:
        // Constructor
        // maxThreads = 最大執行緒數量
        ThreadLimiter(size_t maxThreads = std::thread::hardware_concurrency())
            : m_maxThreads(maxThreads)
        {
        }

        // Destructor
        virtual ~ThreadLimiter()
        {
            m_isStop = true;
            waitOver();
        }

        // 清除任務佇列
        void cxxlFASTCALL clearTask()
        {
            std::lock_guard<std::mutex> lock(m_task_mutex);
            while (m_tasks.empty() == false)
                m_tasks.pop();
        }

        // 等待完成所有任務
        void cxxlFASTCALL waitOver()
        {
            while (true)
            {
                m_isOver.wait();
                std::lock_guard<std::mutex> lock(m_task_mutex);
                if (m_isOverFlag)
                    break;
            }
        }

        // 放入要執行的任務
        // f = 要執行的函數
        // args = 要傳入 f 的參數
        // return_type = f 的回傳型態
        // res = f 的回傳
        template <class F, class... Args>
        auto operator()(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
        {
            if(m_isStop) // 如果是要結束所有執行緒
                return false;

            using return_type = decltype(f(args...));

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();

            {
                std::lock_guard<std::mutex> lock(m_task_mutex);
                m_isOver.zero();
                m_isOverFlag = false;

                m_tasks.emplace([task]()
                                { (*task)(); });

                if (m_numThreads < m_maxThreads) // 未達執行緒的上限
                {
                    std::thread([this]
                                { this->threadProc(); })
                        .detach;
                    ++m_numThreads;
                }
            }
            return res;
        }
    };

    /*********************************************************************************************** */

    class ThreadPool
    {
        std::mutex m_task_mutex;
        std::queue<std::function<void()>> m_tasks; // 待處理的任務

        std::atomic_bool m_isStop = false; // 表示所有執行緒都結束
        cxxlSemaphore m_gate;              // block 執行緒

        size_t m_maxThreads;     // 最大執行緒數量
        size_t m_numThreads = 0; // 目前多少執行緒在執行

        // 雙重驗證
        cxxlSemaphore m_isOver;    // 等待所有的任務都結束
        bool m_isOverFlag = false; // 所有的任務都結束了

        // 由 thredProc() 呼叫
        // 取出一個任務，若回覆 false 則 thredProc 會 block
        // 若已是最後一個 thredProc 的呼叫，還會通知所有任務結束
        bool cxxlFASTCALL getTask(std::function<void(void)> &Func)
        {
            std::lock_guard<std::mutex> lock(m_task_mutex);
            if (m_tasks.empty())
            {
                --m_numThreads; // 表示有一個執行緒會 block

                if (m_numThreads == 0) // 所有任務都結束
                {
                    m_isOverFlag = true;
                    m_isOver.release();
                }

                return false;
            }
            Func = std::move(m_tasks.front());
            m_tasks.pop();
            return true;
        }

        // std::thread 要使用的函數
        void cxxlFASTCALL thredProc()
        {
            std::function<void(void)> Func;
            while (m_isStop == false)
            {
                m_gate.wait();
                ++m_numThreads;
                while (true)
                {
                    if (!getTask(Func))
                        break;
                    Func();
                }
            }
        }

    public:
        // Constructor
        // maxThreads = 最大執行緒數量
        ThreadPool(size_t maxThreads = std::thread::hardware_concurrency())
            : m_maxThreads(maxThreads),
              m_gate(maxThreads, 0)
        {
            for (size_t i = 0; i < maxThreads; ++i)
            {
                std::thread([this]
                            { this->thredProc(); })
                    .detach();
            }
        }

        // Destructor
        ~ThreadPool()
        {
            m_isStop = true;
            for (size_t i = 0; i < m_maxThreads; ++i)
                m_gate.release();
            waitOver();
        }

        // 等待完成所有任務
        void cxxlFASTCALL waitOver()
        {
            while (true)
            {
                m_isOver.wait();
                std::lock_guard<std::mutex> lock(m_task_mutex);
                if (m_isOverFlag)
                    break;
            }
        }

        // 放入要執行的任務
        // f = 要執行的函數
        // args = 要傳入 f 的參數
        // return_type = f 的回傳型態
        // res = f 的回傳
        template <class F, class... Args>
        auto operator()(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
        {
            if(m_isStop) // 如果是要結束所有執行緒
                return false;

            using return_type = decltype(f(args...));

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();

            {
                std::lock_guard<std::mutex> lock(m_task_mutex);
                m_isOver.zero();
                m_isOverFlag = false;

                m_tasks.emplace([task]()
                                { (*task)(); });
            }
            
            return res;
        }
    };

} // namespace CXXL

#endif // __CXXLCOMMON_THREADMGR_HPP_CxxlMan3
