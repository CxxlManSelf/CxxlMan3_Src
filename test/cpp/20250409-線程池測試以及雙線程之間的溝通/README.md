# Thread Pool Test and Inter-thread Communication

[中文](#中文說明) | [English](#english-description)

---

## 中文說明

### 專案簡介

本專案展示如何使用 C++ 執行緒池（Thread Pool）進行多執行緒計算和執行緒間通訊。程式實現了一個簡單的四則運算計算器，透過執行緒池非同步執行運算並輸出結果。

### 主要特色

- **執行緒池管理**：使用 `ThreadPool<true>` 進行高效的執行緒管理
- **非同步計算**：四則運算（加減乘除）在執行緒池中非同步執行
- **執行緒安全輸出**：使用 Semaphore 確保多執行緒輸出不會互相干擾
- **智能等待機制**：結果輸出採用非阻塞式等待，提升效能

### 技術亮點

#### 1. 執行緒池架構
```cpp
ThreadPool<true> g_threadPool;
```
使用模板化的執行緒池，支援任務排程和 future 結果回傳。

#### 2. 執行緒安全的輸出機制
```cpp
void print(std::string s)
{
    static cxxlSemaphore sem(1,1);
    static size_t count = 0;

    cxxlSemaphoreHelper helper(sem);
    std::cout << ++count << ": " << s << '\n';
    std::cout.flush();
}
```
使用 Semaphore 確保多個執行緒輸出時不會產生競態條件（Race Condition）。

#### 3. 非阻塞式結果處理
```cpp
void print(std::future<std::string> s)
{
    std::future_status status = s.wait_for(std::chrono::milliseconds(100));

    if(status == std::future_status::ready)
        print(s.get());
    else
        asyncPrint(std::move(s));
}
```
- 先嘗試等待 100ms 取得結果
- 若結果未就緒，則將等待工作委派給另一個執行緒
- 避免主執行緒阻塞，提升整體效能

#### 4. 浮點數除零保護
```cpp
if (std::abs(b) > 1e-6f)
    ss << a / b;
else
    ss << "Inf";
```
使用 epsilon 比較而非直接比較 `b != 0`，避免浮點數精度問題。

### 程式流程

1. **初始化**：建立執行緒池和隨機數生成器
2. **迴圈執行 100 次**：
   - 隨機選擇一個運算符（加減乘除）
   - 隨機生成兩個數值
   - 將運算任務提交給執行緒池
   - 將結果輸出任務提交給執行緒池
3. **等待完成**：等待所有任務執行完畢並清理執行緒池

### 編譯與執行

#### 前置需求
- C++20 或更高版本的編譯器
- CxxlMan3 函式庫（包含 `semaphore.hpp` 和 `threadmgr.hpp`）

#### 編譯指令
```bash
g++ -std=C++20 main.cpp -o thread_pool_test -lpthread
```

#### 執行
```bash
./thread_pool_test
```

### 輸出範例
```
1: 42.0 + 73.0 = 115.0
2: 56.0 - 23.0 = 33.0
3: 12.0 * 8.0 = 96.0
4: 84.0 / 7.0 = 12.0
5: 45.0 / 0.0 = Inf
...
100: 91.0 + 34.0 = 125.0
Hello, from mytest!
```

### 關鍵函數說明

| 函數 | 說明 |
|------|------|
| `add(float a, float b)` | 加法運算 |
| `sub(float a, float b)` | 減法運算 |
| `mul(float a, float b)` | 乘法運算 |
| `divide(float a, float b)` | 除法運算（含除零保護） |
| `print(std::string s)` | 執行緒安全的字串輸出 |
| `print(std::future<std::string> s)` | 非阻塞式 future 結果輸出 |
| `asyncPrint(std::future<std::string> s)` | 非同步結果輸出輔助函數 |

### 學習重點

本專案適合學習以下主題：
- C++ 執行緒池的使用
- `std::future` 和 `std::optional` 的應用
- 執行緒同步機制（Semaphore）
- 非阻塞式程式設計
- Lambda 表達式與移動語意

---

## English Description

### Project Overview

This project demonstrates how to use a C++ Thread Pool for multi-threaded computation and inter-thread communication. The program implements a simple four-operation calculator that performs calculations asynchronously using a thread pool.

### Key Features

- **Thread Pool Management**: Efficient thread management using `ThreadPool<true>`
- **Asynchronous Computation**: Four arithmetic operations (add, subtract, multiply, divide) executed asynchronously
- **Thread-Safe Output**: Semaphore-based synchronization ensures no output interference
- **Smart Waiting Mechanism**: Non-blocking result output for improved performance

### Technical Highlights

#### 1. Thread Pool Architecture
```cpp
ThreadPool<true> g_threadPool;
```
Template-based thread pool supporting task scheduling and future result returns.

#### 2. Thread-Safe Output Mechanism
```cpp
void print(std::string s)
{
    static cxxlSemaphore sem(1,1);
    static size_t count = 0;

    cxxlSemaphoreHelper helper(sem);
    std::cout << ++count << ": " << s << '\n';
    std::cout.flush();
}
```
Uses Semaphore to prevent race conditions during multi-threaded output.

#### 3. Non-Blocking Result Processing
```cpp
void print(std::future<std::string> s)
{
    std::future_status status = s.wait_for(std::chrono::milliseconds(100));

    if(status == std::future_status::ready)
        print(s.get());
    else
        asyncPrint(std::move(s));
}
```
- Attempts to wait 100ms for the result
- If not ready, delegates the waiting task to another thread
- Prevents main thread blocking, improving overall performance

#### 4. Floating-Point Division by Zero Protection
```cpp
if (std::abs(b) > 1e-6f)
    ss << a / b;
else
    ss << "Inf";
```
Uses epsilon comparison instead of direct `b != 0` to avoid floating-point precision issues.

### Program Flow

1. **Initialization**: Create thread pool and random number generator
2. **Loop 100 times**:
   - Randomly select an operator (add/subtract/multiply/divide)
   - Generate two random numbers
   - Submit calculation task to thread pool
   - Submit result output task to thread pool
3. **Wait for Completion**: Wait for all tasks to finish and clean up thread pool

### Build and Run

#### Prerequisites
- C++20 or higher compiler
- CxxlMan3 library (includes `semaphore.hpp` and `threadmgr.hpp`)

#### Compilation
```bash
g++ -std=C++20 main.cpp -o thread_pool_test -lpthread
```

#### Execution
```bash
./thread_pool_test
```

### Sample Output
```
1: 42.0 + 73.0 = 115.0
2: 56.0 - 23.0 = 33.0
3: 12.0 * 8.0 = 96.0
4: 84.0 / 7.0 = 12.0
5: 45.0 / 0.0 = Inf
...
100: 91.0 + 34.0 = 125.0
Hello, from mytest!
```

### Key Functions

| Function | Description |
|----------|-------------|
| `add(float a, float b)` | Addition operation |
| `sub(float a, float b)` | Subtraction operation |
| `mul(float a, float b)` | Multiplication operation |
| `divide(float a, float b)` | Division operation (with zero-division protection) |
| `print(std::string s)` | Thread-safe string output |
| `print(std::future<std::string> s)` | Non-blocking future result output |
| `asyncPrint(std::future<std::string> s)` | Asynchronous result output helper function |

### Learning Points

This project is ideal for learning:
- C++ Thread Pool usage
- `std::future` and `std::optional` applications
- Thread synchronization mechanisms (Semaphore)
- Non-blocking programming
- Lambda expressions and move semantics

---

## License

This is a test/demonstration project. Feel free to use and modify as needed.

## Dependencies

- [CxxlMan3](https://github.com/your-repo/CxxlMan3) - Custom C++ utility library
  - `semaphore.hpp` - Semaphore synchronization primitives
  - `threadmgr.hpp` - Thread pool management utilities
