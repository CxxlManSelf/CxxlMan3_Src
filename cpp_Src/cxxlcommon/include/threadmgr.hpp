/****************************************************************************************
 * threadmgr.hpp v1.1.14
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
 *  NOTWAIT = true 時，解構函數不等待所有執行緒結束，這是為了作為全局變數時，在程式
 *            結束時，等不到任務執行緒結束，因為這時執行緒已經被結束了
 * 
 *            對全局變數可用 ThreadLimiter::waitAllTask(); 
 *            ThreadPool::waitAllTaskAndClear(); 來解決
 *
 * Author: CxxlMan
 * Date: 2025 -
 ****************************************************************************************/

#ifndef __CXXLCOMMON_THREADMGR_HPP_CxxlMan3
#define __CXXLCOMMON_THREADMGR_HPP_CxxlMan3

#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <optional>

#include "commondef.hpp"
#include "sysdef.hpp"

namespace CXXL
{

    // 機動增減執行緒的數量
	// NOTWAIT = true 時，解構函數不等待所有執行緒結束 
	template <bool NOTWAIT = false>
    class ThreadLimiter final
    {
        std::mutex m_task_mutex;
        // 通知所有 wait() 的等待線程
        std::condition_variable cv;
        bool cv_ready = true; // 沒有任務時會設為 true

        std::queue<std::function<void()>> m_tasks; // 待處理的任務

        bool m_isStop = false; // true 表示 ThreadLimiter 要結束，不可
                               // 以再加入任務

        size_t m_maxThreads;     // 最大執行緒數量
        size_t m_numThreads = 0; // 目前執行緒數量


        // 由 threadProc() 呼叫
        // 取出一個任務，若回覆 false 則 threadProc 會結束
        // 若已是最後一個 threadProc 的呼叫，還會通知所有 thread 結束
        bool cxxlFASTCALL getTask(std::function<void(void)> &Func)
        {
            std::lock_guard<std::mutex> lock(m_task_mutex);
            if (m_tasks.empty())
            {
                --m_numThreads; // 表示有一個執行緒會結束

                if (m_numThreads == 0) // 所有執行緒都結束
                {
                    cv_ready = true;
                    cv.notify_all();
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
        explicit ThreadLimiter(size_t maxThreads = std::thread::hardware_concurrency())
            : m_maxThreads(maxThreads)
        {
        }

        // Destructor
        ~ThreadLimiter()
        {
            {
                std::lock_guard<std::mutex> lock(m_task_mutex);
                m_isStop = true;
            }


            if constexpr (NOTWAIT == false) 
            {
                waitAllTask();
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
            std::unique_lock<std::mutex> lock(m_task_mutex);
            cv.wait(lock, [this]{ return cv_ready; });
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
                if(m_isStop) // 如果是要結束所有執行緒了，不放入任務
                    return std::nullopt;

                cv_ready = false;

                m_tasks.emplace([task]()
                                { (*task)(); });

                if (m_numThreads < m_maxThreads) // 未達執行緒的上限
                {
                    std::thread ([this]
                                { this->threadProc(); }).detach();
                    
                    ++m_numThreads;
                }
            }

            return res; // 返回結果
        }

        ThreadLimiter(const ThreadLimiter&) = delete;
        ThreadLimiter& operator=(const ThreadLimiter&) = delete;
        ThreadLimiter(ThreadLimiter&&) = delete;
        ThreadLimiter& operator=(ThreadLimiter&&) = delete;        
    };

    /*********************************************************************************************** */

    // 建構時已備妥所指定的執行緒數量 
    // NOTWAIT = true 時，解構函數不等待所有執行緒結束 
    template <bool NOTWAIT = false>
    class ThreadPool final
    {
        std::mutex m_task_mutex;

        // 通知所有 wait() 的等待線程
        std::condition_variable cv;

        std::condition_variable m_gate;  // 放入等待任務的執行緒

        std::queue<std::function<void()>> m_tasks; // 待處理的任務

        size_t m_maxThreads;     // 最大執行緒數量，或者說建立的執行緒數量
                                 // ThreadPool 結束前應先等待所有執行
                                 // 緒結束 (m_maxThreads == 0)
        size_t m_numThreads = 0; // 目前多少執行緒在執行

        bool cv_ready = true; // 沒有任務時會設為 true
        bool m_isStop = false; // true 表示 ThreadPool 要結束，不可
                               // 以再加入任務



        // 由 threadProc() 呼叫
        // 取出一個任務，若回覆 false 則 threadProc 會 block
        // 若已是最後一個 threadProc 的呼叫，還會通知所有任務結束
        bool cxxlFASTCALL getTask(std::function<void(void)> &Func)
        {
            std::lock_guard<std::mutex> lock(m_task_mutex);
            if (m_tasks.empty())
            {
                --m_numThreads; // 表示有一個執行緒會 block

                if (m_numThreads == 0) // 所有任務都結束
                {
                    cv_ready = true;
                    cv.notify_all();
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
                {
                    std::unique_lock<std::mutex> lock(m_task_mutex);
                    if(m_tasks.empty()) // 再次確定沒有任務
                    {
                        m_gate.wait(lock, [this](){ return m_tasks.empty() == false || m_isStop; });
                        if(m_tasks.empty() && m_isStop)
                            break;
                    }
                    ++m_numThreads;
                }

                while (true)
                {
                    if (!getTask(Func))
                        break;                    
                    Func();
                }
            }

            std::lock_guard<std::mutex> lock(m_task_mutex);
            --m_maxThreads;
            if(m_maxThreads == 0) // 所有建立的執行緒都結束了
            {
                cv_ready = true;
                cv.notify_all();
            }
        }


    public:
        // Constructor
        // maxThreads = 最大執行緒數量
        explicit ThreadPool(size_t maxThreads = std::thread::hardware_concurrency())
            : m_maxThreads(maxThreads)
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
                waitAllTaskAndClear();
            }

        }

        // ThreadPool 獨有
        // 等待完成所有任務后清除所有執行緒
        void cxxlFASTCALL waitAllTaskAndClear()
        {            
            std::unique_lock<std::mutex> lock(m_task_mutex);
           
            m_isStop = true;
            m_gate.notify_all();  // 一次性通知所有執行緒

            cv.wait(lock, [this]{ return m_maxThreads == 0; });
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
            std::unique_lock<std::mutex> lock(m_task_mutex);
            cv.wait(lock, [this]{ return cv_ready; });
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
                if(m_isStop) // 如果是要結束所有執行緒了，不放入任務
                    return std::nullopt;

                cv_ready = false;

                m_tasks.emplace([task]()
                                { (*task)(); });

                m_gate.notify_one();
            }
            
            return res; // 返回結果
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;        
    };

} // namespace CXXL

#endif // __CXXLCOMMON_THREADMGR_HPP_CxxlMan3
