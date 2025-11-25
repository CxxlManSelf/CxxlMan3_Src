# 跨平台動態連結庫載入測試 / Cross-Platform DLL Loader Test

[English](#english) | [中文](#中文)

---

## 中文

### 📋 專案簡介

這是一個展示如何在 C++20 環境中實現跨平台動態連結庫載入的測試項目。該項目演示了如何使用自定義的 `IDllLoader` 介面動態載入 DLL/SO/DYLIB，並在多執行緒環境中安全地調用插件提供的函數。

### ✨ 主要特色

- **🌐 跨平台支援**：支援 Windows (MSVC/MinGW)、Linux 和 macOS
- **🔌 插件架構**：使用 `IDllLoader` 介面實現插件系統
- **🧵 多執行緒計算**：使用線程池進行非同步運算
- **🔒 執行緒安全**：使用信號量確保多執行緒環境下的安全輸出
- **💾 生命週期管理**：正確管理 DLL 生命週期，防止過早卸載
- **✅ 完整測試**：包含正常流程、錯誤處理和生命週期測試

### 📁 專案結構

```
dll_loader_test/
├── CMakeLists.txt          # 根專案配置
├── README.md               # 本文件
├── main/                   # 主程序專案
│   ├── CMakeLists.txt      # 主程序構建配置
│   └── mytest.cpp          # 測試程序原始碼
└── plug/                   # 插件專案
    ├── CMakeLists.txt      # 插件構建配置
    └── plugdll.cpp         # 插件原始碼（四則運算）
```

### 🔧 系統需求

#### 必要條件
- **CMake** 3.10.0 或更高版本
- **C++20** 編譯器：
  - Windows: MSVC 2019+ 或 MinGW-w64
  - Linux: GCC 10+ 或 Clang 10+
  - macOS: Clang 12+ (Xcode 12+)

#### 依賴函數庫
- `cxxlcommon` - 公共功能庫（包含 `dll_loader.hpp`、`threadmgr.hpp`、`semaphore.hpp`）
- `dl` (僅 Unix) - 動態加載函數庫

### 🚀 編譯步驟

#### Windows (MSVC)
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### Windows (MinGW)
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

#### Linux / macOS
```bash
mkdir build
cd build
cmake ..
make
```

### 💡 使用方法

編譯完成後，執行檔案和插件 DLL 會輸出到 `build/bin` 目錄：

```bash
cd build/bin
./MyTest        # Linux/macOS
MyTest.exe      # Windows
```

### 🧪 測試內容

程式執行以下測試：

1. **📥 DLL 載入測試**
   - 驗證 `isValid()` 方法
   - 檢查 DLL 載入狀態

2. **🔍 函數獲取測試**
   - 從 DLL 獲取四個運算函數（add、sub、mul、divide）
   - 驗證所有函數是否成功獲取

3. **⚡ 多執行緒運算測試**
   - 執行 100 次隨機四則運算
   - 使用線程池進行非同步計算
   - 執行緒安全的結果輸出

4. **❌ 錯誤處理測試**
   - 測試獲取不存在的函數
   - 測試載入不存在的 DLL

5. **♻️ 生命週期管理測試**
   - 驗證在釋放 `loader` 後，函數對象是否仍然有效
   - 證明 DLL 生命週期管理的正確性

### 🔑 技術要點

#### 1. extern "C" 導出
插件使用 `extern "C"` 避免 C++ 名稱修飾：
```cpp
extern "C" {
    std::string CXXL_DLLEXPORT add(float a, float b) { ... }
}
```

#### 2. DLL 生命週期管理
`getProc()` 方法返回的 `std::function` 內部持有 `shared_ptr`，確保 DLL 不會過早卸載：
```cpp
return [func, holder = getDllLoader()](auto&&...args) -> decltype(auto) {
    return func(std::forward<decltype(args)>(args)...);
};
```

#### 3. u8string 支援
提供轉換函數處理 C++20 的 `char8_t` 類型：
```cpp
std::string to_output_string(const cxxlSTDSTRING& str);
```

#### 4. 跨平台編譯
使用 CMake 條件編譯，自動適配不同平台：
- Windows: `.dll`
- Linux: `.so`
- macOS: `.dylib`

### 📊 示例輸出

```
插件載入成功: PlugDll.dll
成功獲取所有四則運算函數
1: 42 + 15 = 57
2: 88 - 33 = 55
3: 7 * 8 = 56
4: 100 / 4 = 25
...
100: 36 / 6 = 6

所有運算任務已完成

=== 錯誤處理測試 ===
✓ 正確處理：無法獲取不存在的函數 'nonexistent'
✓ 正確處理：無法載入不存在的 DLL

=== DLL 生命週期管理測試 ===
釋放 loader 前測試: 100 + 200 = 300
已釋放 loader 的 shared_ptr
✓ 釋放 loader 後測試成功: 300 + 400 = 700
✓ DLL 生命週期管理正確：函數對象持有的 shared_ptr 保持了 DLL 的載入狀態

=== 所有測試完成 ===
```

### 📝 作者

CxxlMan - 2025

---

## English

### 📋 Project Overview

This is a test project demonstrating cross-platform dynamic library loading in C++20. It showcases how to use a custom `IDllLoader` interface to dynamically load DLL/SO/DYLIB files and safely invoke plugin functions in a multi-threaded environment.

### ✨ Key Features

- **🌐 Cross-Platform Support**: Windows (MSVC/MinGW), Linux, and macOS
- **🔌 Plugin Architecture**: Plugin system implemented using `IDllLoader` interface
- **🧵 Multi-threaded Computing**: Asynchronous operations using thread pool
- **🔒 Thread Safety**: Semaphore-based synchronization for thread-safe output
- **💾 Lifetime Management**: Proper DLL lifetime management to prevent premature unloading
- **✅ Comprehensive Testing**: Includes normal flow, error handling, and lifetime tests

### 📁 Project Structure

```
dll_loader_test/
├── CMakeLists.txt          # Root project configuration
├── README.md               # This file
├── main/                   # Main program project
│   ├── CMakeLists.txt      # Main program build config
│   └── mytest.cpp          # Test program source code
└── plug/                   # Plugin project
    ├── CMakeLists.txt      # Plugin build config
    └── plugdll.cpp         # Plugin source (arithmetic operations)
```

### 🔧 System Requirements

#### Prerequisites
- **CMake** 3.10.0 or higher
- **C++20** compiler:
  - Windows: MSVC 2019+ or MinGW-w64
  - Linux: GCC 10+ or Clang 10+
  - macOS: Clang 12+ (Xcode 12+)

#### Dependencies
- `cxxlcommon` - Common library (includes `dll_loader.hpp`, `threadmgr.hpp`, `semaphore.hpp`)
- `dl` (Unix only) - Dynamic loading library

### 🚀 Build Instructions

#### Windows (MSVC)
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### Windows (MinGW)
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

#### Linux / macOS
```bash
mkdir build
cd build
cmake ..
make
```

### 💡 Usage

After building, the executable and plugin DLL will be in the `build/bin` directory:

```bash
cd build/bin
./MyTest        # Linux/macOS
MyTest.exe      # Windows
```

### 🧪 Test Contents

The program performs the following tests:

1. **📥 DLL Loading Test**
   - Validates `isValid()` method
   - Checks DLL loading status

2. **🔍 Function Retrieval Test**
   - Retrieves four arithmetic functions (add, sub, mul, divide) from DLL
   - Verifies all functions are successfully obtained

3. **⚡ Multi-threaded Computation Test**
   - Performs 100 random arithmetic operations
   - Uses thread pool for asynchronous computation
   - Thread-safe result output

4. **❌ Error Handling Test**
   - Tests retrieval of non-existent functions
   - Tests loading of non-existent DLL

5. **♻️ Lifetime Management Test**
   - Verifies function objects remain valid after releasing `loader`
   - Demonstrates correct DLL lifetime management

### 🔑 Technical Highlights

#### 1. extern "C" Export
Plugin uses `extern "C"` to avoid C++ name mangling:
```cpp
extern "C" {
    std::string CXXL_DLLEXPORT add(float a, float b) { ... }
}
```

#### 2. DLL Lifetime Management
`std::function` returned by `getProc()` holds a `shared_ptr` internally, ensuring DLL isn't unloaded prematurely:
```cpp
return [func, holder = getDllLoader()](auto&&...args) -> decltype(auto) {
    return func(std::forward<decltype(args)>(args)...);
};
```

#### 3. u8string Support
Provides conversion function for C++20 `char8_t` type:
```cpp
std::string to_output_string(const cxxlSTDSTRING& str);
```

#### 4. Cross-platform Compilation
Uses CMake conditional compilation for different platforms:
- Windows: `.dll`
- Linux: `.so`
- macOS: `.dylib`

### 📊 Example Output

```
Plugin loaded successfully: PlugDll.dll
Successfully retrieved all arithmetic functions
1: 42 + 15 = 57
2: 88 - 33 = 55
3: 7 * 8 = 56
4: 100 / 4 = 25
...
100: 36 / 6 = 6

All computation tasks completed

=== Error Handling Test ===
✓ Correctly handled: Cannot retrieve non-existent function 'nonexistent'
✓ Correctly handled: Cannot load non-existent DLL

=== DLL Lifetime Management Test ===
Test before releasing loader: 100 + 200 = 300
Released loader's shared_ptr
✓ Test after releasing loader succeeded: 300 + 400 = 700
✓ DLL lifetime management is correct: Function object's shared_ptr keeps DLL loaded

=== All Tests Completed ===
```

### 📝 Author

CxxlMan - 2025

---

## 📄 License

This project is part of the CxxlMan3 library test suite.

## 🤝 Contributing

This is a test/example project. For issues or improvements related to the core `IDllLoader` implementation, please refer to the main CxxlMan3 library repository.

## 📚 Related Documentation

- [dll_loader.hpp](../../include/dll_loader.hpp) - Core DLL loader interface
- [threadmgr.hpp](../../include/threadmgr.hpp) - Thread pool management
- [semaphore.hpp](../../include/semaphore.hpp) - Semaphore implementation
