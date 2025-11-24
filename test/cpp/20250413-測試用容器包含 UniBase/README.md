# UniBase Container Management Test | UniBase 容器管理測試

[English](#english) | [中文](#中文)

---

## English

### Overview

This test program demonstrates how to store `UniBase` objects in standard C++ STL containers (specifically `std::unordered_set`). It showcases the lifecycle management mechanisms of the UniBase system, including automatic cleanup, manual removal, and forced detachment.

### Core Concepts

#### 1. **UniBase Management**
- `UniBase` objects must be managed through `UniObserver` or `UniOwner`
- When placing `UniBase` in containers, store `UniOwner` instances, not raw `UniBase` objects
- This ensures proper lifecycle management and automatic cleanup

#### 2. **ONE Mode Behavior**
The test uses `UniBase<UniBaseType::ONE>`, which implements a "fragile sharing" mechanism:
- **Multiple owners allowed**: A single `UniBase` can be held by multiple `UniOwner` instances
- **One-detach destruction**: If **any one** `UniOwner` releases (detaches) the `UniBase`, the object is **immediately destroyed**
- This differs from `shared_ptr`'s reference counting, where the object persists until all references are released

#### 3. **Key Technical Features**
- **Custom Hash & Equal functors**: Enable `UniOwner` to work with `std::unordered_set`
- **detachUniBaseFunc callback**: Handles automatic container cleanup when `UniBase` is destroyed
- **Mutex protection**: Ensures thread-safe container access

### Test Scenarios

The program demonstrates four different lifecycle management approaches:

#### Stage 1: Initialization
```cpp
MyRoot root;
UniPtr<MyUniBase> uniBase1_ptr(new MyUniBase1());
// ... create more objects
root.addUniBase(uniBase1_ptr);  // Add to container
```

#### Stage 2: Reset UniPtr (no effect on container)
```cpp
uniBase1_ptr.reset();  // Releases external UniPtr
// UniPtr does NOT participate in lifecycle management
// Only UniOwner in the container manages the object
// UniBase1 continues to exist in the container
// This demonstrates that UniPtr is only for transport, not ownership
```

#### Stage 3: Manual Removal (removeUniBase)
```cpp
root.removeUniBase(uniBase2_ptr);  // Remove from container only
// UniBase2 object still exists, just not in container
```

#### Stage 4: Forced Kick (KickUniBase)
```cpp
KickUniBase kickUniBase(uniBase3_ptr);  // Force detachment
// Guarantees complete detach flow and container removal
```

### Expected Output

1. **First call** to `doSomething()`: Shows 3 objects (Base1, Base2, Base3)
2. **Second call**: Shows 3 objects (Base1, Base2, Base3) - reset() has no effect, UniPtr doesn't manage lifecycle
3. **Third call**: Shows 2 objects (Base1, Base3) - Base2 manually removed from container (but not destroyed)
4. **Fourth call**: Shows 1 object (Base1) - Base3 kicked out and destroyed

### Code Structure

```
main.cpp
├── MyUniBase (abstract base class)
│   ├── MyUniBase1 (test derived class)
│   ├── MyUniBase2 (test derived class)
│   └── MyUniBase3 (test derived class)
├── Hash (custom hash functor for UniOwner)
├── Equal (custom equality functor for UniOwner)
├── MyRoot (container manager class)
│   ├── addUniBase()      - Add UniBase to container
│   ├── removeUniBase()   - Manually remove from container
│   └── doSomething()     - Iterate and invoke all objects
├── KickUniBase (utility for forced detachment)
└── main() (test program entry point)
```

### Key Implementation Details

#### Hash Functor
```cpp
struct Hash {
    size_t operator()(const UniOwner<MyUniBase> &uniOwner) const {
        return reinterpret_cast<size_t>(uniOwner.getUniBase().get());
    }
};
```
Uses the memory address of the `UniBase` object as the hash value, ensuring consistent hashing.

#### detachUniBaseFunc Callback
```cpp
auto detachUniBaseFunc = [this](UniOwner<MyUniBase> *pSender, void *pChkUniBase) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (pSender->chkUniBase(pChkUniBase)) {
        m_uniOwnerSet.erase(*pSender);  // Auto-remove from container
    }
};
```
Automatically removes the `UniOwner` from the container when the `UniBase` is destroyed.

#### KickUniBase Mechanism
```cpp
template <typename UNIBASE>
class KickUniBase : public UniBase<UniBaseType::ONE> {
public:
    KickUniBase(UniPtr<UNIBASE> &uniBase_ptr) {
        UniOwner<UNIBASE> uniOwner(this, [](/*...*/) {});
        uniOwner.setUniBase(uniBase_ptr);
        uniBase_ptr.reset();
        // When constructor exits, uniOwner is destroyed
        // Triggers complete detach flow
    }
};
```

### Building and Running

```bash
# Compile (assuming c++20 or later)
g++ -std=c++20 main.cpp -o unibase_test -I/path/to/UniBase/include

# Run
./unibase_test
```

### Dependencies

- **CxxlMan3::UniBase**: The UniBase smart pointer library
- **c++20 or later**: For modern C++ features
- **Standard library**: `<iostream>`, `<unordered_set>`, `<mutex>`

### Learning Points

1. **Container Design**: How to design containers that hold smart-pointer-managed objects
2. **Lifecycle Management**: Different approaches to managing object lifecycles
3. **Callback Mechanisms**: Using callbacks for automatic cleanup
4. **Thread Safety**: Protecting shared resources with mutexes
5. **ONE Mode Semantics**: Understanding fragile sharing vs. reference counting

---

## 中文

### 概述

本測試程式展示如何將 `UniBase` 物件放入標準 C++ STL 容器（具體使用 `std::unordered_set`）。它展示了 UniBase 系統的生命週期管理機制，包括自動清理、手動移除和強制分離。

### 核心概念

#### 1. **UniBase 管理**
- `UniBase` 物件必須透過 `UniObserver` 或 `UniOwner` 來管理
- 將 `UniBase` 放入容器時，應存放 `UniOwner` 實例，而非原始 `UniBase` 物件
- 這確保了正確的生命週期管理和自動清理

#### 2. **ONE 模式行為**
本測試使用 `UniBase<UniBaseType::ONE>`，實現了「脆弱共享」機制：
- **允許多個擁有者**：單一 `UniBase` 可被多個 `UniOwner` 實例持有
- **單一分離即銷毀**：只要**任何一個** `UniOwner` 釋放（detach）該 `UniBase`，物件就會**立即被銷毀**
- 這與 `shared_ptr` 的引用計數不同，後者在所有引用釋放前物件都會持續存在

#### 3. **關鍵技術特點**
- **自訂 Hash 與 Equal 函數物件**：使 `UniOwner` 能在 `std::unordered_set` 中使用
- **detachUniBaseFunc 回呼**：在 `UniBase` 被銷毀時自動清理容器
- **互斥鎖保護**：確保容器的執行緒安全存取

### 測試情境

程式展示了四種不同的生命週期管理方式：

#### 階段 1：初始化
```cpp
MyRoot root;
UniPtr<MyUniBase> uniBase1_ptr(new MyUniBase1());
// ... 建立更多物件
root.addUniBase(uniBase1_ptr);  // 加入容器
```

#### 階段 2：重置 UniPtr（對容器無影響）
```cpp
uniBase1_ptr.reset();  // 釋放外部的 UniPtr
// UniPtr 不參與生命週期管理
// 只有容器中的 UniOwner 管理物件
// UniBase1 繼續存在於容器中
// 這展示了 UniPtr 只用於運輸，不具擁有權
```

#### 階段 3：手動移除 (removeUniBase)
```cpp
root.removeUniBase(uniBase2_ptr);  // 僅從容器移除
// UniBase2 物件仍然存在，只是不在容器中
```

#### 階段 4：強制踢除 (KickUniBase)
```cpp
KickUniBase kickUniBase(uniBase3_ptr);  // 強制分離
// 保證完整的 detach 流程和容器移除
```

### 預期輸出

1. **第一次**呼叫 `doSomething()`：顯示 3 個物件 (Base1, Base2, Base3)
2. **第二次**呼叫：顯示 3 個物件 (Base1, Base2, Base3) - reset() 無影響，UniPtr 不管理生命週期
3. **第三次**呼叫：顯示 2 個物件 (Base1, Base3) - Base2 已手動從容器移除（但未銷毀）
4. **第四次**呼叫：顯示 1 個物件 (Base1) - Base3 已被踢除並銷毀

### 程式碼結構

```
main.cpp
├── MyUniBase（抽象基底類別）
│   ├── MyUniBase1（測試衍生類別）
│   ├── MyUniBase2（測試衍生類別）
│   └── MyUniBase3（測試衍生類別）
├── Hash（UniOwner 的自訂雜湊函數物件）
├── Equal（UniOwner 的自訂相等比較函數物件）
├── MyRoot（容器管理類別）
│   ├── addUniBase()      - 將 UniBase 加入容器
│   ├── removeUniBase()   - 手動從容器移除
│   └── doSomething()     - 遍歷並呼叫所有物件
├── KickUniBase（強制分離的工具類別）
└── main()（測試程式進入點）
```

### 關鍵實作細節

#### Hash 函數物件
```cpp
struct Hash {
    size_t operator()(const UniOwner<MyUniBase> &uniOwner) const {
        return reinterpret_cast<size_t>(uniOwner.getUniBase().get());
    }
};
```
使用 `UniBase` 物件的記憶體位址作為雜湊值，確保一致的雜湊結果。

#### detachUniBaseFunc 回呼
```cpp
auto detachUniBaseFunc = [this](UniOwner<MyUniBase> *pSender, void *pChkUniBase) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (pSender->chkUniBase(pChkUniBase)) {
        m_uniOwnerSet.erase(*pSender);  // 自動從容器移除
    }
};
```
當 `UniBase` 被銷毀時，自動從容器中移除對應的 `UniOwner`。

#### KickUniBase 機制
```cpp
template <typename UNIBASE>
class KickUniBase : public UniBase<UniBaseType::ONE> {
public:
    KickUniBase(UniPtr<UNIBASE> &uniBase_ptr) {
        UniOwner<UNIBASE> uniOwner(this, [](/*...*/) {});
        uniOwner.setUniBase(uniBase_ptr);
        uniBase_ptr.reset();
        // 建構函數結束時，uniOwner 被銷毀
        // 觸發完整的 detach 流程
    }
};
```

### 編譯與執行

```bash
# 編譯（假設使用 c++20 或更新版本）
g++ -std=c++20 main.cpp -o unibase_test -I/path/to/UniBase/include

# 執行
./unibase_test
```

### 相依性

- **CxxlMan3::UniBase**：UniBase 智慧指標函式庫
- **c++20 或更新版本**：現代 C++ 特性
- **標準函式庫**：`<iostream>`、`<unordered_set>`、`<mutex>`

### 學習要點

1. **容器設計**：如何設計持有智慧指標管理物件的容器
2. **生命週期管理**：管理物件生命週期的不同方法
3. **回呼機制**：使用回呼實現自動清理
4. **執行緒安全**：使用互斥鎖保護共享資源
5. **ONE 模式語意**：理解脆弱共享與引用計數的差異

### 進階說明

#### ONE 模式 vs. 引用計數

| 特性 | UniBase ONE 模式 | std::shared_ptr |
|------|-----------------|-----------------|
| 多重持有 | ✓ 可以 | ✓ 可以 |
| 銷毀條件 | **任一**持有者放棄 | **所有**持有者放棄 |
| 適用場景 | 嚴格生命週期控制 | 彈性資源共享 |
| 特性描述 | 脆弱共享 | 強引用計數 |

#### UniPtr vs UniOwner 的關鍵差異

| 特性 | UniPtr | UniOwner/UniObserver |
|------|--------|---------------------|
| 用途 | 函數參數、運算時使用 | 作為成員變數 |
| 能否作為成員變數 | ✗ 不可以 | ✓ 可以（必須） |
| 參與生命週期管理 | ✗ 不參與 | ✓ 參與 |
| ONE 模式影響 | ✗ 不影響 | ✓ 影響 |
| 轉換 | - | getUniBase() → UniPtr |

#### 三種操作方式的差異

1. **reset()** - 釋放 UniPtr（無實際作用）
   - 外部 UniPtr 放棄持有
   - **但 UniPtr 不參與生命週期管理**
   - 容器中的 UniOwner 仍然持有物件
   - 物件不會被銷毀
   - **結果：物件繼續存在於容器中，reset() 無效果**

2. **removeUniBase()** - 手動從容器移除
   - 移除容器中的 UniOwner
   - 觸發 ONE 模式：UniOwner 放棄，物件被銷毀
   - 外部 UniPtr 變為懸空指標（如果還存在）
   - **結果：物件從容器移除並被銷毀**

3. **KickUniBase** - 強制踢除
   - 建立臨時 UniOwner 接管物件（第二個 UniOwner）
   - 重置外部 UniPtr
   - 臨時 UniOwner 立即銷毀（觸發 ONE 模式）
   - 所有 UniOwner 收到 detach 通知
   - **結果：物件被銷毀，從所有容器移除**

### 實際應用場景

這種容器管理機制適用於：
- **事件監聽器管理**：任一監聽器失效時立即清理
- **臨時資源池**：資源一旦被某處釋放就立即回收
- **依賴關係鏈**：任一依賴斷開即中止整個鏈
- **觀察者模式**：嚴格的觀察者生命週期控制

### 常見問題

**Q: 為什麼不直接使用 `std::shared_ptr`？**
A: `shared_ptr` 是引用計數，所有持有者都釋放才銷毀。ONE 模式提供更嚴格的控制，適合需要精確管理生命週期的場景。

**Q: 容器為什麼要用 `UniOwner` 而不是 `UniPtr`？**
A: `UniOwner` 提供 detach 回呼機制，可在物件銷毀時自動清理容器，避免懸空指標。

**Q: 如果不想讓物件被立即銷毀怎麼辦？**
A: 使用 `removeUniBase()` 手動移除，而不是 `reset()`。這樣物件會從容器移除但不會被銷毀。

---

### License

This is a test/example program. Refer to your UniBase library license for terms.

### Author

Generated as part of UniBase container management testing.

### See Also

- [UniBase Documentation](https://github.com/your-repo/unibase)
- [CxxlMan3 Project](https://github.com/your-repo/cxxlman3)
