# semaphore.hpp 與 threadmgr.hpp 測試套件

本測試程式用於驗證 CXXL 函式庫中的多執行緒管理元件：`semaphore.hpp` 和 `threadmgr.hpp`。

## 測試內容

### 1. Semaphore 相關測試

#### cxxlSemaphore
- **基本功能測試** (`testSemaphoreBasic`)
  - 驗證 semaphore 能正確限制並發執行緒數量
  - 測試場景：設定最大 2 個並發，提交 10 個任務
  - 驗證最大並發數不超過設定值

- **Block 功能測試** (`testSemaphoreBlock`)
  - 驗證 `block()` 方法能阻止新執行緒獲取 semaphore
  - 測試場景：執行緒持有 semaphore 時設為 block 狀態
  - 驗證 `release()` 能解除 block 狀態

#### cxxlSemaphoreHelper
- **自動管理測試** (`testSemaphoreHelper`)
  - 驗證 RAII 風格的自動 release 機制
  - 測試 helper 物件離開作用域時自動釋放 semaphore
  - 確保不會發生資源洩漏

### 2. ThreadLimiter 相關測試

- **基本功能測試** (`testThreadLimiterBasic`)
  - 驗證能正確限制執行緒池大小並執行所有任務
  - 測試 10 個任務在 4 個執行緒的限制下完成

- **返回值測試** (`testThreadLimiterReturnValue`)
  - 驗證能正確接收任務的返回值
  - 測試無參數和帶參數的 lambda 函式
  - 透過 `std::future` 獲取結果

- **清除任務測試** (`testThreadLimiterClearTask`)
  - 驗證 `clearTask()` 能清除尚未執行的任務
  - 確認已在執行的任務會完成，但待執行的會被取消

- **壓力測試** (`stressTestThreadLimiter`)
  - 提交 100 個任務到 8 個執行緒的 limiter
  - 驗證高負載情況下的穩定性和正確性

### 3. ThreadPool 相關測試

- **基本功能測試** (`testThreadPoolBasic`)
  - 驗證執行緒池能正確執行所有提交的任務
  - 測試 `waitAllTask()` 等待機制

- **返回值測試** (`testThreadPoolReturnValue`)
  - 驗證能正確處理不同類型的返回值（int, string）
  - 測試參數傳遞機制

- **清除任務測試** (`testThreadPoolClearTask`)
  - 驗證 `clearTask()` 能清除待執行任務佇列

- **等待並清理測試** (`testThreadPoolWaitAllTaskAndClear`)
  - 驗證 `waitAllTaskAndClear()` 能等待所有任務完成並清理資源
  - 確認清理後無法提交新任務

- **壓力測試** (`stressTestThreadPool`)
  - 提交 100 個任務到 8 個執行緒的 pool
  - 驗證高負載情況下的穩定性

## 建置與執行

### 前置需求
- C++20 或更高版本
- CXXL 函式庫（包含 `semaphore.hpp` 和 `threadmgr.hpp`）
- 支援多執行緒的編譯器

### 編譯
```bash
g++ -std=C++20 main.cpp -o test -lpthread -I<CXXL_INCLUDE_PATH>
```

或使用 CMake：
```bash
mkdir build && cd build
cmake ..
make
```

### 執行測試
```bash
./test
```

## 測試結果解讀

測試程式會輸出每個測試案例的結果：
- `[PASS]` - 測試通過
- `[FAIL]` - 測試失敗（會附上失敗原因）

最後會顯示測試摘要：
```
========================================
測試結果: X passed, Y failed
總計: Z tests
========================================
```

返回值：
- `0` - 所有測試通過
- `1` - 有測試失敗

## 測試覆蓋功能

| 類別 | 測試項目 | 功能描述 |
|------|---------|---------|
| **cxxlSemaphore** | 並發限制 | 限制同時存取資源的執行緒數量 |
| | Block/Release | 暫停和恢復 semaphore 的使用 |
| **cxxlSemaphoreHelper** | RAII 管理 | 自動化 wait/release 管理 |
| **ThreadLimiter** | 執行緒數限制 | 動態建立執行緒但限制最大數量 |
| | 任務佇列 | 管理待執行任務佇列 |
| | 返回值處理 | 透過 future 獲取任務結果 |
| **ThreadPool** | 執行緒池 | 重用固定數量的執行緒 |
| | 任務排程 | 將任務分配給空閒執行緒 |
| | 生命週期管理 | 控制執行緒池的建立和銷毀 |

## 測試統計

總共 13 個測試案例：
- Semaphore 測試：3 個
- ThreadLimiter 測試：4 個
- ThreadPool 測試：4 個
- 壓力測試：2 個

## 注意事項

- 測試使用了 `std::this_thread::sleep_for` 來模擬耗時操作，實際執行時間可能因系統而異
- 部分測試依賴時序（timing），在高負載系統上可能會有偶發性失敗
- 壓力測試會建立大量執行緒和任務，建議在有足夠資源的環境下執行
- 測試採用原子操作（`std::atomic`）確保多執行緒環境下的正確性

## 相關檔案

- [main.cpp](main.cpp) - 測試主程式
- `semaphore.hpp` - CXXL Semaphore 實作
- `threadmgr.hpp` - CXXL ThreadLimiter 和 ThreadPool 實作

## 授權

本測試程式屬於 CXXL 專案的一部分。
