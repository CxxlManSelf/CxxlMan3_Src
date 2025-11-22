# ThreadPool in DLL Experiment / 線程池置於 DLL 實驗

[English](#english) | [中文](#中文)

---

## English

### Overview

This project demonstrates how to properly manage a thread pool (`ThreadPool`) that resides in a dynamic link library (DLL) on Windows. It addresses the critical issue of thread cleanup when the main program exits, ensuring threads created in the DLL are properly terminated through the normal destruction process.

### Problem Statement

When a program terminates, threads created within a DLL are typically killed abruptly without going through proper cleanup procedures. This can lead to resource leaks and undefined behavior. This experiment provides a solution using the `waitAllTaskAndClear()` function.

### Key Features

- **Thread Pool in DLL**: Demonstrates placing a `ThreadPool<true>` instance in a shared library
- **Proper Cleanup**: Uses `waitAllTaskAndClear()` to ensure all threads complete before program exit
- **Thread-Safe Output**: Implements semaphore-based synchronization for concurrent output operations
- **Asynchronous Task Processing**: Shows how to handle both quick and long-running asynchronous operations

### Project Structure

```
.
├── CMakeLists.txt          # Root CMake configuration
├── dll/                    # DLL project
│   ├── CMakeLists.txt     # DLL build configuration
│   ├── dll.hpp            # ThreadPool declaration with export macros
│   └── dll.cpp            # ThreadPool definition
└── exe/                    # Executable project
    ├── CMakeLists.txt     # Executable build configuration
    └── main.cpp           # Main program demonstrating thread pool usage
```

### Technical Details

#### DLL Configuration

The DLL exports a global `ThreadPool<true>` instance:

```cpp
// dll.hpp
extern MYDLL_DLLEXPORT CxxlMan3::ThreadPool<true> g_threadPool;

// dll.cpp
ThreadPool<true> g_threadPool;
```

**Important**: The template parameter must be `true` to enable proper DLL integration.

#### Thread Pool Usage

The example program:
1. Performs 100 random arithmetic operations (add, subtract, multiply, divide)
2. Executes calculations asynchronously using the thread pool
3. Outputs results in a thread-safe manner
4. Properly cleans up all threads before exit

#### Critical Cleanup

Before `main()` exits, you **must** call:

```cpp
g_threadPool.waitAllTaskAndClear();
```

This ensures all threads spawned in the DLL are properly terminated and cleaned up.

### Building the Project

#### Prerequisites

- CMake 3.10.0 or higher
- C++20 compatible compiler
- CxxlMan3 library (cxxlcore, cxxlcommon)

#### Build Steps

**Windows (MinGW):**

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

**Windows (MSVC):**

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build .
```

#### Running

```bash
cd build/bin
./MyTest.exe
```

### Example Output

```
1: 42 + 17 = 59
2: 83 - 25 = 58
3: 7 * 12 = 84
4: 90 / 5 = 18
...
100: 33 + 44 = 77
Hello, from mytest!
```

### Key Concepts

1. **DLL Export/Import**: Uses `CXXL_DLLEXPORT`/`CXXL_DLLIMPORT` macros for cross-platform compatibility
2. **Thread Pool Template Parameter**: `ThreadPool<true>` enables DLL-safe operation
3. **Semaphore Synchronization**: `cxxlSemaphore` prevents garbled output from concurrent threads
4. **Future-based Async**: Demonstrates modern C++ async programming with `std::future`
5. **Graceful Shutdown**: `waitAllTaskAndClear()` ensures clean thread termination

### Related Examples

This example is functionally identical to the `20250409` example, with the key difference being that `g_threadPool` is placed in the DLL's `.cpp` file rather than the main executable.

### Dependencies

- **CxxlMan3 Library**: Provides `ThreadPool`, `cxxlSemaphore`, and core utilities
  - `cxxlcore`: Core functionality
  - `cxxlcommon`: Common utilities

### Platform Support

- Windows (MinGW, MSVC)
- Linux/Unix (with `-ldl` flag)

---

## 中文

### 概述

本專案示範如何在 Windows 動態連結函式庫 (DLL) 中正確管理線程池 (`ThreadPool`)。它解決了當主程式結束時，DLL 中產生的線程被強制終止而未經過正常解構程序的關鍵問題。

### 問題陳述

當程式結束時，在 DLL 中產生的線程通常會被直接砍掉，而不會走正常的解構程序。這可能導致資源洩漏和未定義行為。本實驗提供了使用 `waitAllTaskAndClear()` 函數的解決方案。

### 主要特色

- **DLL 中的線程池**：展示如何將 `ThreadPool<true>` 實例放置在共享函式庫中
- **正確清理**：使用 `waitAllTaskAndClear()` 確保所有線程在程式結束前完成
- **線程安全輸出**：實作基於信號量的同步機制來處理並發輸出操作
- **非同步任務處理**：展示如何處理快速和長時間執行的非同步操作

### 專案結構

```
.
├── CMakeLists.txt          # 根目錄 CMake 配置
├── dll/                    # DLL 專案
│   ├── CMakeLists.txt     # DLL 建置配置
│   ├── dll.hpp            # ThreadPool 宣告與匯出巨集
│   └── dll.cpp            # ThreadPool 定義
└── exe/                    # 執行檔專案
    ├── CMakeLists.txt     # 執行檔建置配置
    └── main.cpp           # 主程式，示範線程池使用方式
```

### 技術細節

#### DLL 配置

DLL 匯出一個全域的 `ThreadPool<true>` 實例：

```cpp
// dll.hpp
extern MYDLL_DLLEXPORT CxxlMan3::ThreadPool<true> g_threadPool;

// dll.cpp
ThreadPool<true> g_threadPool;
```

**重要**：模板參數必須使用 `true` 以啟用正確的 DLL 整合。

#### 線程池使用方式

範例程式：
1. 執行 100 次隨機四則運算（加、減、乘、除）
2. 使用線程池非同步執行計算
3. 以線程安全的方式輸出結果
4. 在結束前正確清理所有線程

#### 關鍵清理步驟

在離開 `main()` 之前，**必須**呼叫：

```cpp
g_threadPool.waitAllTaskAndClear();
```

這確保所有在 DLL 中產生的線程都被正確終止和清理。

### 建置專案

#### 前置需求

- CMake 3.10.0 或更高版本
- 支援 C++20 的編譯器
- CxxlMan3 函式庫（cxxlcore、cxxlcommon）

#### 建置步驟

**Windows (MinGW)：**

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

**Windows (MSVC)：**

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build .
```

#### 執行

```bash
cd build/bin
./MyTest.exe
```

### 範例輸出

```
1: 42 + 17 = 59
2: 83 - 25 = 58
3: 7 * 12 = 84
4: 90 / 5 = 18
...
100: 33 + 44 = 77
Hello, from mytest!
```

### 核心概念

1. **DLL 匯出/匯入**：使用 `CXXL_DLLEXPORT`/`CXXL_DLLIMPORT` 巨集以達到跨平台相容性
2. **線程池模板參數**：`ThreadPool<true>` 啟用 DLL 安全操作
3. **信號量同步**：`cxxlSemaphore` 防止並發線程輸出時產生混亂
4. **基於 Future 的非同步**：展示現代 C++ 使用 `std::future` 的非同步程式設計
5. **優雅關閉**：`waitAllTaskAndClear()` 確保線程乾淨地終止

### 相關範例

本範例與 `20250409` 範例在功能上是相同的，主要差異在於 `g_threadPool` 被放置在 DLL 的 `.cpp` 檔案中，而非主執行檔。

### 相依套件

- **CxxlMan3 函式庫**：提供 `ThreadPool`、`cxxlSemaphore` 和核心工具
  - `cxxlcore`：核心功能
  - `cxxlcommon`：通用工具

### 平台支援

- Windows（MinGW、MSVC）
- Linux/Unix（需使用 `-ldl` 旗標）

---

## License / 授權

Please refer to the CxxlMan3 library license.

請參考 CxxlMan3 函式庫的授權條款。

## Author / 作者

CxxlMan3 Development Team
