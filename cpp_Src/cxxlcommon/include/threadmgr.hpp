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
        std::mutex m_queue_mutex;
        std::queue<std::function<void()>> m_tasks; // 待處理的任務

        size_t m_maxThreads;     // 最大執行緒數量
        size_t m_numThreads = 0; // 目前執行緒數量

        cxxlSemaphore m_isOver;


        // 由 thredProc() 呼叫
        // 取出一個任務，若回覆 false 則 thredProc 會結束
        // 若已是最後一個 thredProc 還會
        bool cxxlFASTCALL getTask(std::function<void(void)> &Func)
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            if (m_tasks.empty())
            {
                --m_numThreads; // 表示有一個執行緒會結束

                if (m_numThreads == 0)
                m_isOver.setBlock(false);

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
        virtual ~ThreadLimiter() {}

        // 放入要執行的任務
        template <class F, class... Args>
        auto operator()(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
        {
            using return_type = decltype(f(args...));

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();

            {
                std::unique_lock<std::mutex> lock(m_queue_mutex);
                m_tasks.emplace([task]()
                                { (*task)(); });
                if (m_numThreads < m_maxThreads)
                {
                    std::thread m([this]
                                  { this->threadProc(); });
                    m.detach();
                    ++m_numThreads;
                    m_isOver.setBlock(true);
                }
            }
            m_condition.notify_one();
            return res;
        }
    };

    class ThreadMgr
    {
    public:
        ThreadMgr(size_t numThreads = std::thread::hardware_concurrency())
            : m_stop(false)
        {
            for (size_t i = 0; i < numThreads; ++i)
            {
                m_threads.emplace_back([this]
                                       {
                    for (;;)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(m_queue_mutex);
                            m_condition.wait(lock, [this] { return m_stop || !m_tasks.empty(); });
                            if (m_stop && m_tasks.empty())
                                return;
                            task = std::move(m_tasks.front());
                            m_tasks.pop();
                        }
                        task();
                    } });
            }
        }

        ~ThreadMgr()
        {
            {
                std::lock_guard<std::mutex> lock(m_queue_mutex);
                m_stop = true;
            }
            m_condition.notify_all();
            for (std::thread &thread : m_threads)
                thread.join();
        }

        template <class F, class... Args>
        auto enqueue(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
        {
            using return_type = decltype(f(args...));

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(m_queue_mutex);

                if (m_stop)
                    throw std::runtime_error("enqueue on stopped ThreadPool");

                m_tasks.emplace([task]()
                                { (*task)(); });
            }
            m_condition.notify_one();
            return res;
        }

    private:
        std::vector<std::thread> m_threads;
        std::queue<std::function<void()>> m_tasks;

        std::mutex m_queue_mutex;
        std::condition_variable m_condition;
        bool m_stop;
    };

} // namespace CXXL

#endif // __CXXLCOMMON_THREADMGR_HPP_CxxlMan3
