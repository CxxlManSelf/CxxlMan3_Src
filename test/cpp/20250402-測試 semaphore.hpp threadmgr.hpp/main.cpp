#include <iostream>
#include <threadmgr.hpp>
#include <semaphore.hpp>
#include <chrono>
#include <atomic>
#include <vector>

using namespace CXXL;
using namespace std::chrono_literals;

// 測試結果計數器
struct TestResults {
    int passed = 0;
    int failed = 0;

    void pass(const std::string& testName) {
        ++passed;
        std::cout << "[PASS] " << testName << "\n";
    }

    void fail(const std::string& testName, const std::string& reason = "") {
        ++failed;
        std::cout << "[FAIL] " << testName;
        if (!reason.empty()) {
            std::cout << " - " << reason;
        }
        std::cout << "\n";
    }

    void summary() {
        std::cout << "\n========================================\n";
        std::cout << "測試結果: " << passed << " passed, " << failed << " failed\n";
        std::cout << "總計: " << (passed + failed) << " tests\n";
        std::cout << "========================================\n";
    }
};

TestResults results;

// ========================================
// Semaphore 測試
// ========================================

void testSemaphoreBasic() {
    std::cout << "\n--- 測試 Semaphore 基本功能 ---\n";

    cxxlSemaphore sem(2, 2); // 最多2個線程，初始允許2個
    std::atomic<int> counter{0};
    std::atomic<int> maxConcurrent{0};

    auto task = [&]() {
        sem.wait();
        int current = ++counter;
        if (current > maxConcurrent) {
            maxConcurrent.store(current);
        }
        std::this_thread::sleep_for(100ms);
        --counter;
        sem.release();
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(task);
    }

    for (auto& t : threads) {
        t.join();
    }

    if (maxConcurrent <= 2) {
        results.pass("Semaphore 基本限制功能");
    } else {
        results.fail("Semaphore 基本限制功能", "最大並發數超過限制: " + std::to_string(maxConcurrent.load()));
    }
}

void testSemaphoreBlock() {
    std::cout << "\n--- 測試 Semaphore Block 功能 ---\n";

    cxxlSemaphore sem(1, 1);
    std::atomic<bool> taskStarted{false};

    std::thread t1([&]() {
        sem.wait();
        taskStarted = true;
        std::this_thread::sleep_for(200ms);
        sem.release();
    });

    std::this_thread::sleep_for(50ms); // 確保 t1 已經取得 semaphore
    sem.block(); // 設為 block 狀態

    std::atomic<bool> t2Started{false};
    std::thread t2([&]() {
        t2Started = true;
        sem.wait(); // 這裡會被 block
        sem.release();
    });

    std::this_thread::sleep_for(100ms);

    if (t2Started && taskStarted) {
        results.pass("Semaphore Block 功能");
    } else {
        results.fail("Semaphore Block 功能");
    }

    sem.release(); // 解除 block
    t1.join();
    t2.join();
}

void testSemaphoreHelper() {
    std::cout << "\n--- 測試 SemaphoreHelper ---\n";

    cxxlSemaphore sem(1, 1);
    std::atomic<int> counter{0};
    bool helperWorks = true;

    auto task = [&]() {
        cxxlSemaphoreHelper helper(sem);
        ++counter;
        if (counter > 1) {
            helperWorks = false;
        }
        std::this_thread::sleep_for(50ms);
        --counter;
        // helper 會自動呼叫 release
    };

    std::thread t1(task);
    std::thread t2(task);

    t1.join();
    t2.join();

    if (helperWorks) {
        results.pass("SemaphoreHelper 自動管理");
    } else {
        results.fail("SemaphoreHelper 自動管理");
    }
}

// ========================================
// ThreadLimiter 測試
// ========================================

void testThreadLimiterBasic() {
    std::cout << "\n--- 測試 ThreadLimiter 基本功能 ---\n";

    ThreadLimiter<> limiter(4);
    std::atomic<int> taskCount{0};

    for (int i = 0; i < 10; ++i) {
        auto future = limiter([&taskCount, i]() {
            std::this_thread::sleep_for(50ms);
            taskCount++;
            return i * 2;
        });

        if (!future.has_value()) {
            results.fail("ThreadLimiter 基本功能", "無法提交任務");
            return;
        }
    }

    limiter.waitAllTask();

    if (taskCount == 10) {
        results.pass("ThreadLimiter 基本功能 - 所有任務完成");
    } else {
        results.fail("ThreadLimiter 基本功能", "任務數不符: " + std::to_string(taskCount.load()));
    }
}

void testThreadLimiterReturnValue() {
    std::cout << "\n--- 測試 ThreadLimiter 返回值 ---\n";

    ThreadLimiter<> limiter(2);

    auto future1 = limiter([]() { return 42; });
    auto future2 = limiter([](int x, int y) { return x + y; }, 10, 20);

    if (!future1.has_value() || !future2.has_value()) {
        results.fail("ThreadLimiter 返回值", "無法獲取 future");
        return;
    }

    int result1 = future1.value().get();
    int result2 = future2.value().get();

    if (result1 == 42 && result2 == 30) {
        results.pass("ThreadLimiter 返回值");
    } else {
        results.fail("ThreadLimiter 返回值", "結果不正確");
    }
}

void testThreadLimiterClearTask() {
    std::cout << "\n--- 測試 ThreadLimiter 清除任務 ---\n";

    ThreadLimiter<> limiter(1);
    std::atomic<int> executedCount{0};

    // 提交很多任務
    for (int i = 0; i < 20; ++i) {
        limiter([&executedCount]() {
            std::this_thread::sleep_for(10ms);
            executedCount++;
        });
    }

    std::this_thread::sleep_for(30ms); // 讓一些任務開始執行
    limiter.clearTask(); // 清除待執行的任務
    limiter.waitAllTask();

    if (executedCount < 20) {
        results.pass("ThreadLimiter 清除任務 - 已清除部分任務");
    } else {
        results.fail("ThreadLimiter 清除任務", "所有任務都執行了");
    }
}

// ========================================
// ThreadPool 測試
// ========================================

void testThreadPoolBasic() {
    std::cout << "\n--- 測試 ThreadPool 基本功能 ---\n";

    ThreadPool<> pool(4);
    std::atomic<int> taskCount{0};

    for (int i = 0; i < 10; ++i) {
        auto future = pool([&taskCount, i]() {
            std::this_thread::sleep_for(50ms);
            taskCount++;
            return i * 3;
        });

        if (!future.has_value()) {
            results.fail("ThreadPool 基本功能", "無法提交任務");
            return;
        }
    }

    pool.waitAllTask();

    if (taskCount == 10) {
        results.pass("ThreadPool 基本功能 - 所有任務完成");
    } else {
        results.fail("ThreadPool 基本功能", "任務數不符: " + std::to_string(taskCount.load()));
    }
}

void testThreadPoolReturnValue() {
    std::cout << "\n--- 測試 ThreadPool 返回值 ---\n";

    ThreadPool<> pool(2);

    auto future1 = pool([]() { return 100; });
    auto future2 = pool([](std::string s) { return s + " World"; }, std::string("Hello"));

    if (!future1.has_value() || !future2.has_value()) {
        results.fail("ThreadPool 返回值", "無法獲取 future");
        return;
    }

    int result1 = future1.value().get();
    std::string result2 = future2.value().get();

    if (result1 == 100 && result2 == "Hello World") {
        results.pass("ThreadPool 返回值");
    } else {
        results.fail("ThreadPool 返回值", "結果不正確");
    }
}

void testThreadPoolClearTask() {
    std::cout << "\n--- 測試 ThreadPool 清除任務 ---\n";

    ThreadPool<> pool(1);
    std::atomic<int> executedCount{0};

    // 提交很多任務
    for (int i = 0; i < 20; ++i) {
        pool([&executedCount]() {
            std::this_thread::sleep_for(10ms);
            executedCount++;
        });
    }

    std::this_thread::sleep_for(30ms); // 讓一些任務開始執行
    pool.clearTask(); // 清除待執行的任務
    pool.waitAllTask();

    if (executedCount < 20) {
        results.pass("ThreadPool 清除任務 - 已清除部分任務");
    } else {
        results.fail("ThreadPool 清除任務", "所有任務都執行了");
    }
}

void testThreadPoolWaitAllTaskAndClear() {
    std::cout << "\n--- 測試 ThreadPool waitAllTaskAndClear ---\n";

    ThreadPool<> pool(2);
    std::atomic<int> taskCount{0};

    for (int i = 0; i < 5; ++i) {
        pool([&taskCount]() {
            std::this_thread::sleep_for(50ms);
            taskCount++;
        });
    }

    pool.waitAllTaskAndClear();

    // 清理後不應該能提交新任務
    auto future = pool([]() { return 1; });

    if (taskCount == 5 && !future.has_value()) {
        results.pass("ThreadPool waitAllTaskAndClear");
    } else {
        results.fail("ThreadPool waitAllTaskAndClear");
    }
}

// ========================================
// 壓力測試
// ========================================

void stressTestThreadLimiter() {
    std::cout << "\n--- ThreadLimiter 壓力測試 ---\n";

    ThreadLimiter<> limiter(8);
    std::atomic<int> successCount{0};
    const int TASK_COUNT = 100;

    for (int i = 0; i < TASK_COUNT; ++i) {
        auto future = limiter([&successCount, i]() {
            std::this_thread::sleep_for(5ms);
            successCount++;
            return i;
        });

        if (future.has_value()) {
            // 不需要立即獲取結果
        }
    }

    limiter.waitAllTask();

    if (successCount == TASK_COUNT) {
        results.pass("ThreadLimiter 壓力測試");
    } else {
        results.fail("ThreadLimiter 壓力測試", std::to_string(successCount.load()) + "/" + std::to_string(TASK_COUNT));
    }
}

void stressTestThreadPool() {
    std::cout << "\n--- ThreadPool 壓力測試 ---\n";

    ThreadPool<> pool(8);
    std::atomic<int> successCount{0};
    const int TASK_COUNT = 100;

    for (int i = 0; i < TASK_COUNT; ++i) {
        auto future = pool([&successCount, i]() {
            std::this_thread::sleep_for(5ms);
            successCount++;
            return i;
        });

        if (future.has_value()) {
            // 不需要立即獲取結果
        }
    }

    pool.waitAllTask();

    if (successCount == TASK_COUNT) {
        results.pass("ThreadPool 壓力測試");
    } else {
        results.fail("ThreadPool 壓力測試", std::to_string(successCount.load()) + "/" + std::to_string(TASK_COUNT));
    }
}

// ========================================
// 主程式
// ========================================

int main(int, char **)
{
    std::cout << "========================================\n";
    std::cout << "開始測試 semaphore.hpp 和 threadmgr.hpp\n";
    std::cout << "========================================\n";

    try {
        // Semaphore 測試
        testSemaphoreBasic();
        testSemaphoreBlock();
        testSemaphoreHelper();

        // ThreadLimiter 測試
        testThreadLimiterBasic();
        testThreadLimiterReturnValue();
        testThreadLimiterClearTask();

        // ThreadPool 測試
        testThreadPoolBasic();
        testThreadPoolReturnValue();
        testThreadPoolClearTask();
        testThreadPoolWaitAllTaskAndClear();

        // 壓力測試
        stressTestThreadLimiter();
        stressTestThreadPool();

    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] 發生異常: " << e.what() << "\n";
    }

    results.summary();

    return results.failed > 0 ? 1 : 0;
}
