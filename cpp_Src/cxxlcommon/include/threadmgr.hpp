/****************************************************************************************
 * threadmgr.hpp v1.0.11
 *
 *  提供兩個執行緒的管理功能
 *
 *  ThreadLimiter 有任務時才會創建執行緒，可以指定最多的執行緒活動數量，任務減少到低於執行緒
 *                的最大活動數量，沒事做的執行緒就會結束。這樣比較節省執行緒的使用量，但是多
 *                了重複創造執行緒所須的時間
 *
 *  ThreadPool 準備好指定的執行緒數量，ThreadPool 未結束前執行緒不會結束，這樣可以節省
 *             創建執行緒的時間。但是會占用執行緒的數量，系統所能提供的執行緒數量是有限的
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
#include <optional>

#include "commondef.hpp"
#include "sysdef.hpp"
#include "semaphore.hpp"

namespace CXXL
{

    // 機動增減執行緒的數量
	// NOTWAIT = true 時，解構函數不等待所有執行緒結束 
	template <bool NOTWAIT = false>
    class ThreadLimiter
    {
        std::mutex m_task_mutex;
        std::queue<std::function<void()>> m_tasks; // 待處理的任務

        bool m_isStop = false; // 表示所有執行緒都結束

        size_t m_maxThreads;     // 最大執行緒數量
        size_t m_numThreads = 0; // 目前執行緒數量

        cxxlSemaphore m_allTasksDone{1,1};    // 等待所有 thread 都結束
        
        // 目前有多少在使用 waitAllTask()
        std::atomic<size_t> m_numWaitUsers{0}; 

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
                    m_allTasksDone.release();
                }

                return false;
            }
            Func = std::move(m_tasks.front());
            m_tasks.pop();
            return true;
        }

        // std::thread 要使用的函數
        void cxxlFASTCALL threadProc()
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
            {
                std::lock_guard<std::mutex> lock(m_task_mutex);
                m_isStop = true;
            }

            if constexpr (NOTWAIT == false) 
            {
                m_allTasksDone.wait();        
            }

            while(m_numWaitUsers > 0)
            {
                m_allTasksDone.release();
            }
        }

        // 清除任務佇列
        void cxxlFASTCALL clearTask()
        {
            std::lock_guard<std::mutex> lock(m_task_mutex);
            while (m_tasks.empty() == false)
                m_tasks.pop();
        }


        // 等待所有任務結束
        void cxxlFASTCALL waitAllTask() 
        { 
            ++m_numWaitUsers;
            while(true)
            {
                m_allTasksDone.wait();
                {
                    std::lock_guard<std::mutex> lock(m_task_mutex);
                    if(m_numThreads == 0)
                    {
                        m_allTasksDone.release();
                        --m_numWaitUsers;
                        break;
                    }    
                }
            }           
        }

        // 放入要執行的任務
        // f = 要執行的函數
        // args = 要傳入 f 的參數
        // return_type = f 的回傳型態
        // res = f 的回傳
        // 但如果已經不能使用了，則回傳 std::nullopt
        template <class F, class... Args>
        auto operator()(F &&f, Args &&...args) -> std::optional<std::future<std::invoke_result_t<F, Args...>>>
        {
            using return_type = std::invoke_result_t<F, Args...>;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();

            {
                std::lock_guard<std::mutex> lock(m_task_mutex);
                if(m_isStop) // 如果是要結束所有執行緒
                    return std::nullopt;

                m_allTasksDone.zero();

                m_tasks.emplace([task]()
                                { (*task)(); });

                if (m_numThreads < m_maxThreads) // 未達執行緒的上限
                {
                    std::thread ([this]
                                { this->threadProc(); }).detach();
                    
                    ++m_numThreads;
                }
            }

            return std::move(res); // 返回結果
        }
    };

    /*********************************************************************************************** */

    // 建構時已備妥所指定的執行緒數量 
    // NOTWAIT = true 時，解構函數不等待所有執行緒結束 
    template <bool NOTWAIT = false>
    class ThreadPool
    {
        std::mutex m_task_mutex;
        std::queue<std::function<void()>> m_tasks; // 待處理的任務

        bool m_isStop = false; // 表示所有執行緒都結束
        cxxlSemaphore m_gate;  // block 執行緒

        size_t m_maxThreads;     // 最大執行緒數量
        size_t m_numThreads = 0; // 目前多少執行緒在執行
        size_t m_existThreads; // 目前存活執行緒數量

        cxxlSemaphore m_allTasksDone{1,1};    // 等待所有的任務都結束

        // 目前有多少在使用 waitAllTask()
        std::atomic<size_t> m_numWaitUsers{0}; 

        // 由 thredProc() 呼叫
        // 取出一個任務，若回覆 false 則 thredProc 會 block
        // 若已是最後一個 thredProc 的呼叫，還會通知所有任務結束
        bool cxxlFASTCALL getTask(std::function<void(void)> &Func)
        {
            std::lock_guard<std::mutex> lock(m_task_mutex);
            if (m_tasks.empty())
            {
                --m_numThreads; // 表示有一個執行緒會 block

                if (m_numThreads == 0) // 所有執行緒都結束
                {
                    m_allTasksDone.release();
                }

                return false;
            }
            Func = std::move(m_tasks.front());
            m_tasks.pop();
            return true;
        }

        // std::thread 要使用的函數
        void cxxlFASTCALL threadProc()
        {
            std::function<void(void)> Func;
            while (!m_isStop)
            {
                m_gate.wait();
                {
                    std::lock_guard<std::mutex> lock(m_task_mutex);
                    if(m_isStop && m_tasks.empty())
                        break;
                    else
                        ++m_numThreads;
                }

                while (true)
                {
                    if (!getTask(Func))
                        break;                    
                    Func();
                }
            }
            
            m_task_mutex.lock();
            if(--m_existThreads == 0) // 所有執行緒都結束
            {
                m_task_mutex.unlock();
                m_allTasksDone.release();
            }
            else
                m_task_mutex.unlock();
        }


    public:
        // Constructor
        // maxThreads = 最大執行緒數量
        ThreadPool(size_t maxThreads = std::thread::hardware_concurrency())
            : m_maxThreads(maxThreads),
              m_existThreads(maxThreads),
              m_gate(maxThreads, 0)
        {
            for (size_t i = 0; i < maxThreads; ++i)
            {
                std::thread([this]
                            { this->threadProc(); }).detach();
            }
        }

        // Destructor
        ~ThreadPool()
        {

            if constexpr (NOTWAIT == false) 
            {
                m_allTasksDone.wait();
            }

            {
                std::lock_guard<std::mutex> lock(m_task_mutex);
                m_isStop = true;
                // 讓被 block 的執行緒結束
                for (size_t i = 0; i < m_maxThreads; ++i)
                    m_gate.release();
            }


            if constexpr (NOTWAIT == false) 
            {
                m_allTasksDone.wait();
            }
    
            while(m_numWaitUsers > 0)
            {
                m_allTasksDone.release();
            }
        }

        // ThreadPool 獨有
        // 等待完成所有任務后清除所有執行緒
        void cxxlFASTCALL waitAllTaskAndClear()
        {
            waitAllTask();
            {
                std::lock_guard<std::mutex> lock(m_task_mutex);
                m_isStop = true;
            }

            // 讓被 block 的執行緒結束
            for (size_t i = 0; i < m_maxThreads; ++i)
                m_gate.release();

            m_allTasksDone.wait();

            while(m_numWaitUsers > 0)
            {
                m_allTasksDone.release();
            }

            m_allTasksDone.release();
        }


        // 等待所有任務結束
        void cxxlFASTCALL waitAllTask() 
        { 
            ++m_numWaitUsers;
            while(true)
            {
                m_allTasksDone.wait();
                {
                    std::lock_guard<std::mutex> lock(m_task_mutex);
                    if (m_tasks.empty() && m_numThreads == 0)
                    {
                        m_allTasksDone.release();
                        --m_numWaitUsers;
                        break;
                    }    
                }
            }           
        }

        // 放入要執行的任務
        // f = 要執行的函數
        // args = 要傳入 f 的參數
        // return_type = f 的回傳型態
        // res = f 的回傳
        // 但如果已經不能使用了，則回傳 std::nullopt
        template <class F, class... Args>
        auto operator()(F &&f, Args &&...args) -> std::optional<std::future<std::invoke_result_t<F, Args...>>>
        {
            using return_type = std::invoke_result_t<F, Args...>;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();

            {
                std::lock_guard<std::mutex> lock(m_task_mutex);
                if(m_isStop) // 如果是要結束所有執行緒
                    return std::nullopt;

                m_allTasksDone.zero();

                m_tasks.emplace([task]()
                                { (*task)(); });

                m_gate.release();
            }
            
            return std::move(res); // 返回結果
        }
    };

} // namespace CXXL

#endif // __CXXLCOMMON_THREADMGR_HPP_CxxlMan3
