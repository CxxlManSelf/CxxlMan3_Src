/***********************************************************************
 * locktime.hpp v0.1.0
 * 
 * mutex 的時間鎖定機制，給予指定的時間嘗試鎖定
 * 
 * Author: CxxlMan
 * Date: 2025 -
***********************************************************************/
#ifndef __CXXLCOMMON_LOCKTIME_HPP_CxxlMan3
#define __CXXLCOMMON_LOCKTIME_HPP_CxxlMan3

#include <chrono>
#include <mutex>
#include "sysdef.hpp"

namespace CXXL
{
    // 嘗試在指定時間之內鎖定 mutex
    // mutex: 可能是 mutex 或 recursive_mutex
    // timeout: 指定等待鎖定的最大時間
    // retry: 在指定時間內要嘗試幾次
    template <typename MUTEX>
    bool cxxlFASTCALL lockTime(MUTEX &mutex, 
        std::chrono::microseconds timeout = std::chrono::microseconds(10000), 
        unsigned int retry = 10)
    {
        const auto checkInterval = timeout / retry;    // 每次檢查的間隔
        const auto startTime = std::chrono::steady_clock::now();
        
        do
        {
            if (mutex.try_lock())
                return true; // 成功鎖定

            // 休眠一小段時間，讓出 CPU
            std::this_thread::sleep_for(checkInterval);
        }while (std::chrono::steady_clock::now() - startTime < timeout)

        return false;
    }

}

#endif // __CXXLCOMMON_LOCKTIME_HPP_CxxlMan3