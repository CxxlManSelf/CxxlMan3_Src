# UniBase Circular Reference Test | UniBase 循環參照測試

[English](#english) | [中文](#中文)

---

## English

### Overview

This test program demonstrates how the `UniOwner` smart pointer from the CxxlMan3 library handles circular references without memory leaks. It showcases the ability to properly destroy objects that reference each other in a cycle.

### Purpose

The primary goal is to verify that:
- `UniOwner` can manage circular references between objects
- Objects in a circular reference chain are properly destroyed when the root owner is released
- No memory leaks occur even with complex ownership hierarchies

### Test Scenario

The test creates a reference structure:
```
MyRoot → MyClassA ⇄ MyClassB
```

Where:
- `MyClassA` and `MyClassB` hold `UniOwner` references to each other (circular reference)
- `MyRoot` holds a `UniOwner` reference to `MyClassA` (root owner)
- When `MyRoot` is destroyed, the entire chain should be properly cleaned up

### Key Concepts

#### UniPtr vs UniOwner

- **UniPtr**: External smart pointer for referencing UniBase objects
- **UniOwner**: Ownership pointer that actively destroys owned objects when the owner is destroyed

#### Destructor Callbacks

Each `UniOwner` is initialized with a destructor callback that:
1. Checks if the owned object is still valid using `chkUniBase()`
2. Calls `destroy()` on the owned object to break the circular reference
3. Uses mutex locks to ensure thread safety

#### getDestructor() Singleton

⚠️ **Important**: `getDestructor()` can only be successfully called **once** during the program's lifetime:
- Second call will fail or return `nullptr`
- Must be called at the beginning of `main()`
- The returned `IDestrWaiter` must remain valid throughout all UniBase objects' lifetimes

### Test Flow

1. **Get Core Destructor Controller**: Call `getDestructor()` (can only succeed once)
2. **Create Objects**: Instantiate `MyClassA` and `MyClassB`
3. **Establish Circular Reference**: Make the two objects reference each other
4. **Create Root**: Instantiate `MyRoot` and have it own `MyClassA`
5. **Release External Pointers**: Reset `myA_ptr` and `myB_ptr` (objects still held by `UniOwner`)
6. **Trigger Cascade Destruction**: Reset `myRoot_ptr`, which triggers:
   - `MyRoot` destructor called
   - `MyRoot`'s callback destroys `MyClassA`
   - `MyClassA` destructor called
   - `MyClassA`'s callback destroys `MyClassB`
   - `MyClassB` destructor called
   - Circular reference successfully broken

### Expected Output

When `myRoot_ptr.reset()` is called, you should see destructor messages in order:
```
MyRoot destructor
MyClassA destructor
MyClassB destructor
```

No memory leaks should occur.

### Class Hierarchy

#### MyClassA
- Inherits from `UniBase<UniBaseType::ALL>` (supports multiple UniPtr references)
- Holds a `UniOwner<MyClassB>` member
- Provides `addMyB()` to set the reference

#### MyClassB
- Inherits from `UniBase<UniBaseType::ALL>` (supports multiple UniPtr references)
- Holds a `UniOwner<MyClassA>` member
- Provides `addMyA()` to set the reference

#### MyRoot
- Inherits from `UniBase<UniBaseType::ONE>` (supports single UniPtr reference only)
- Holds a `UniOwner<MyClassA>` member
- Acts as the root node of the ownership tree
- Provides `addMyA()` to set the reference

### Thread Safety

All member access is protected by `std::mutex` to ensure thread-safe operations.

### Building and Running

```bash
# Compile (adjust compiler and paths as needed)
g++ -std=c++20 main.cpp -o circular_ref_test -I/path/to/CxxlMan3/include -lpthread

# Run
./circular_ref_test
```

The program uses interactive prompts - press Enter to proceed through each step and observe the behavior.

### Dependencies

- CxxlMan3 library (`uniptr.hpp`)
- c++20 or later
- Standard library: `<iostream>`, `<mutex>`, `<memory>`

---

## 中文

### 概述

此測試程式展示了 CxxlMan3 函式庫中的 `UniOwner` 智慧指標如何處理循環參照而不會造成記憶體洩漏。它展現了正確銷毀互相參照的物件的能力。

### 目的

主要目標是驗證：
- `UniOwner` 能夠管理物件之間的循環參照
- 循環參照鏈中的物件在根擁有者被釋放時能正確銷毀
- 即使在複雜的擁有權階層中也不會發生記憶體洩漏

### 測試情境

測試建立以下參照結構：
```
MyRoot → MyClassA ⇄ MyClassB
```

其中：
- `MyClassA` 和 `MyClassB` 互相持有對方的 `UniOwner` 參照（循環參照）
- `MyRoot` 持有 `MyClassA` 的 `UniOwner` 參照（根擁有者）
- 當 `MyRoot` 被銷毀時，整個鏈應該被正確清理

### 核心概念

#### UniPtr vs UniOwner

- **UniPtr**：外部智慧指標，用於參照 UniBase 物件
- **UniOwner**：擁有權指標，當擁有者被銷毀時會主動銷毀所擁有的物件

#### 銷毀回呼函式

每個 `UniOwner` 在初始化時設定銷毀回呼函式，它會：
1. 使用 `chkUniBase()` 檢查擁有的物件是否仍然有效
2. 呼叫 `destroy()` 銷毀物件以打破循環參照
3. 使用互斥鎖確保執行緒安全

#### getDestructor() 單例

⚠️ **重要**：`getDestructor()` 在程式生命週期中**只能成功呼叫一次**：
- 第二次呼叫會失敗或回傳 `nullptr`
- 必須在 `main()` 函式一開始就呼叫
- 回傳的 `IDestrWaiter` 必須在所有 UniBase 物件的生命週期內保持有效

### 測試流程

1. **取得核心銷毀控制器**：呼叫 `getDestructor()`（只能成功一次）
2. **建立物件**：實例化 `MyClassA` 和 `MyClassB`
3. **建立循環參照**：讓兩個物件互相參照
4. **建立根節點**：實例化 `MyRoot` 並讓它擁有 `MyClassA`
5. **釋放外部指標**：重置 `myA_ptr` 和 `myB_ptr`（物件仍被 `UniOwner` 持有）
6. **觸發連鎖銷毀**：重置 `myRoot_ptr`，觸發：
   - `MyRoot` 解構函式被呼叫
   - `MyRoot` 的回呼銷毀 `MyClassA`
   - `MyClassA` 解構函式被呼叫
   - `MyClassA` 的回呼銷毀 `MyClassB`
   - `MyClassB` 解構函式被呼叫
   - 循環參照成功打破

### 預期輸出

當 `myRoot_ptr.reset()` 被呼叫時，應該看到依序的解構訊息：
```
MyRoot destructor
MyClassA destructor
MyClassB destructor
```

不應該有任何記憶體洩漏。

### 類別階層

#### MyClassA
- 繼承自 `UniBase<UniBaseType::ALL>`（支援多個 UniPtr 參照）
- 持有一個 `UniOwner<MyClassB>` 成員
- 提供 `addMyB()` 方法設定參照

#### MyClassB
- 繼承自 `UniBase<UniBaseType::ALL>`（支援多個 UniPtr 參照）
- 持有一個 `UniOwner<MyClassA>` 成員
- 提供 `addMyA()` 方法設定參照

#### MyRoot
- 繼承自 `UniBase<UniBaseType::ONE>`（只支援單一 UniPtr 參照）
- 持有一個 `UniOwner<MyClassA>` 成員
- 作為擁有權樹的根節點
- 提供 `addMyA()` 方法設定參照

### 執行緒安全

所有成員存取都受到 `std::mutex` 保護，確保執行緒安全操作。

### 建置與執行

```bash
# 編譯（依需要調整編譯器和路徑）
g++ -std=c++20 main.cpp -o circular_ref_test -I/path/to/CxxlMan3/include -lpthread

# 執行
./circular_ref_test
```

程式使用互動式提示 - 按 Enter 鍵逐步進行並觀察行為。

### 相依套件

- CxxlMan3 函式庫（`uniptr.hpp`）
- c++20 或更新版本
- 標準函式庫：`<iostream>`、`<mutex>`、`<memory>`

---

## License | 授權

Please refer to the CxxlMan3 library license.

請參考 CxxlMan3 函式庫的授權條款。

## Contributing | 貢獻

This is a test program for the CxxlMan3 library. For issues or improvements related to the core library, please refer to the main CxxlMan3 repository.

這是 CxxlMan3 函式庫的測試程式。關於核心函式庫的問題或改進，請參考 CxxlMan3 主要儲存庫。
