# UniBase 系統架構說明

## 🏗️ 系統架構概覽

```
┌─────────────────────────────────────────────────────────────┐
│                      使用者層 (User Layer)                   │
├─────────────────────────────────────────────────────────────┤
│  MyClass : UniBase<ALL>                                     │
│    ├─ UniOwner<Resource>    (生命週期管理)                  │
│    ├─ UniObserver<Resource> (觀察者)                        │
│    └─ UniPtr<Resource>      (臨時持有)                      │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    核心層 (Core Layer)                       │
├─────────────────────────────────────────────────────────────┤
│  _UniBase (基礎實作)                                        │
│    ├─ _UniBaseAll (ALL 策略)                                │
│    ├─ _UniBaseOne (ONE 策略)                                │
│    └─ _Holder (UniOwner/Observer 基類)                      │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  銷毀管理層 (Destructor Layer)               │
├─────────────────────────────────────────────────────────────┤
│  UniBaseDestructor (單執行緒處理器)                         │
│    ├─ 銷毀佇列 (m_list)                                     │
│    ├─ 循環檢測 (checkNoHost)                               │
│    └─ 異步處理 (threadProc)                                │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔄 執行緒模型

### 執行緒分工

```
┌─────────────────────┐        ┌──────────────────────┐
│   使用者執行緒群     │        │  Destructor 執行緒    │
│   (多個)            │        │  (單一)              │
├─────────────────────┤        ├──────────────────────┤
│                     │        │                      │
│ • attach/detach     │        │ • LD_shouldDestroy() │
│ • setUniBase()      │───────>│ • checkNoHost()      │
│ • getUniBase()      │ 通知   │ • LD_destroy()       │
│                     │<───────│ • LD_clearFFlag()    │
│ • 使用者回調         │ 回調   │                      │
│   (在此執行緒)      │        │                      │
└─────────────────────┘        └──────────────────────┘
```

### 重要特性

1. **使用者執行緒**
   - 處理所有公開 API 調用
   - 執行使用者提供的回調
   - 透過 mutex 保護共享數據

2. **Destructor 執行緒**
   - 單一執行緒，避免競態條件
   - 處理所有 `LD_*` 前綴的內部方法
   - 負責循環檢測和銷毀邏輯

---

## 🔐 鎖策略

### 鎖的層級

```
使用者的 m_mutex (MyClass)
    └─ 保護：成員變數、業務邏輯

UniBase 的 m_UniBaseMutex
    └─ 保護：m_holderSet, m_isDestroy, flags

全局鎖 (在 Destructor 內部)
    └─ 保護：銷毀佇列 (m_list)
```

### 避免死鎖的設計

```cpp
// LD_destroy() 的鎖管理
void LD_destroy()
{
    std::unique_lock<std::mutex> lock(m_UniBaseMutex);  // 鎖定
    m_isDestroy = true;

    while (!m_holderSet.empty())
    {
        auto it = m_holderSet.begin();

        lock.unlock();  // ⭐ 解鎖後才調用回調
        (*it)->detachUniBase(this);
        lock.lock();    // ⭐ 重新鎖定
    }
}

// 使用者回調（必須遵守約定）
[this](auto *owner, void *pChk) {
    std::lock_guard<std::mutex> lock(m_mutex);  // ⭐ 必須使用 host class 的 mutex（不是 UniBase 的 mutex）
    if (owner->chkUniBase(pChk))                // ⭐ 用 chkUniBase(pChk) 檢查
        owner->destroy();                        // ⭐ 匹配後調用 destroy()
}
```

**關鍵點**：
- UniBase 的鎖在調用回調前**釋放**
- 使用者必須使用 **host class 的 mutex**（不是 UniBase 的 mutex）
- 兩把鎖**永不同時持有** → 無死鎖
- 回調中必須：1. 上鎖 2. 檢查 chkUniBase(pChk) 3. 調用 destroy()

---

## 🧬 類別關係圖

```
                    IDestroyable
                         ▲
                         │
                    _UniBase
                    │      │
          ┌─────────┴──────┴─────────┐
          │                          │
    _UniBaseAll               _UniBaseOne
          ▲                          ▲
          │                          │
  UniBase<ALL>                UniBase<ONE>
          ▲                          ▲
          │                          │
      (使用者類別繼承)


                    _Holder
                    │      │
          ┌─────────┴──────┴─────────┐
          │                          │
    UniOwner<T>                UniObserver<T>
    (參與生命週期)              (只觀察)
```

### 繼承關係說明

1. **IDestroyable**
   - 介面，定義銷毀相關方法
   - 供 Destructor 調用

2. **_UniBase**
   - 核心實作，管理 holder 集合
   - 提供 attach/detach 機制

3. **_UniBaseAll / _UniBaseOne**
   - 實作不同的生命週期策略
   - `_UniBaseAll`: `m_OwnerCount == 0` 時銷毀
   - `_UniBaseOne`: 任一 Owner detach 就銷毀

4. **_Holder**
   - UniOwner 和 UniObserver 的共同基類
   - 提供 `detachUniBase()` 虛擬介面

---

## 📊 數據結構

### _UniBase 的核心數據

```cpp
class _UniBase
{
    // 狀態標記 (Union)
    union {
        struct {
            bool cFlag       : 1;  // 已放入銷毀佇列
            bool fFlag       : 1;  // 已被檢查過（循環檢測用）
            bool rFlag       : 1;  // 是否為 root (無 holder)
            bool ldFlag      : 1;  // 已判定應銷毀
            bool m_isDestroy : 1;  // 正在銷毀中
            bool justAddFlag : 1;  // 只加入佇列，不檢查
        };
        uint8_t allFlags = 0;  // 快速清零
    };

    // Holder 集合
    std::unordered_set<const _Holder *> m_holderSet;

    // 互斥鎖
    mutable std::mutex m_UniBaseMutex;
};
```

### 狀態轉換圖

```
[創建]
  │
  ├─ rFlag = false (root)
  │
  ▼
[被持有] (attach)
  │
  ├─ rFlag = true
  ├─ m_holderSet.insert()
  │
  ▼
[被檢查] (detachOwner)
  │
  ├─ cFlag = true (放入佇列)
  │
  ▼
[檢測階段] (LD_shouldDestroy)
  │
  ├─ fFlag = true (開始檢查)
  ├─ checkNoHost() 遞迴
  ├─ ldFlag = true/false (檢查結果)
  │
  ▼
[銷毀階段] (LD_destroy)
  │
  ├─ m_isDestroy = true
  ├─ 通知所有 holder
  │
  ▼
[完成]
```

---

## 🔍 循環檢測算法

### 算法原理

使用**深度優先搜尋 (DFS)** + **訪問標記**：

```cpp
bool checkNoHost()
{
    // 已訪問過？（檢測到循環）
    if (fFlag)
        return true;  // 表示找不到 root

    fFlag = true;  // 標記為已訪問
    g_pUniBaseDestructor->reset_fFlag(this);  // 記錄以便稍後清除

    // 是 root？（沒有 holder）
    if (rFlag == false)
        return false;  // 找到 root！

    // 已在銷毀佇列？
    if (cFlag || m_holderSet.size() == 0)
        return false;

    // 遞迴檢查所有 holder 的 host
    bool fNoRootUniBase = true;
    for (auto &it : m_holderSet)
    {
        auto pHost = it->m_pHost;
        fNoRootUniBase = pHost->checkNoHost();
        if (!fNoRootUniBase)
            break;  // 找到 root，提前退出
    }

    return fNoRootUniBase;  // true = 沒有 root (需銷毀)
}
```

### 範例：檢測循環 A→B→C→A

```
Step 1: 檢查 A
  ├─ fFlag(A) = true
  ├─ 檢查 A 的 holder → B
  │
Step 2: 檢查 B
  ├─ fFlag(B) = true
  ├─ 檢查 B 的 holder → C
  │
Step 3: 檢查 C
  ├─ fFlag(C) = true
  ├─ 檢查 C 的 holder → A
  │
Step 4: 檢查 A (再次)
  ├─ fFlag(A) == true  ⭐ 已訪問！
  └─ return true (檢測到循環)

結論：沒有 root → 應該銷毀
```

### 複雜度分析

- **時間複雜度**: O(N)，N 為引用鏈長度
- **空間複雜度**: O(N)，用於標記訪問狀態
- **最壞情況**: 深層嵌套的引用鏈

---

## ⚡ 銷毀流程詳解

### 完整流程時序圖

```
[使用者執行緒]                    [Destructor 執行緒]
      │                                  │
      │ 1. owner.destroy()               │
      │    或超出作用域                   │
      ▼                                  │
 detachOwner()                           │
      │                                  │
      ├─ lock(m_UniBaseMutex)            │
      ├─ m_holderSet.erase()             │
      ├─ m_isDestroy = removeOwner()     │
      ├─ unlock()                        │
      │                                  │
      │ 2. checkDestroy()                │
      ├──────────────────────────────────>│
      │     加入佇列                      │
      │                                  ▼
      │                          3. threadProc()
      │                                  │
      │                                  ├─ 從佇列取出
      │                                  │
      │                          4. LD_shouldDestroy()
      │                                  │
      │                                  ├─ lock(m_UniBaseMutex)
      │                                  ├─ fFlag = true
      │                                  ├─ checkNoHost() 遞迴
      │                                  ├─ ldFlag = result
      │                                  ├─ unlock()
      │                                  │
      │                                  ▼
      │                          5. LD_destroy()
      │                                  │
      │                                  ├─ lock(m_UniBaseMutex)
      │                                  ├─ m_isDestroy = true
      │                                  │
      │                                  ├─ for each holder:
      │                                  │   ├─ unlock()
      │                                  │   ├─ holder->detachUniBase(this)
      │  6. 使用者回調                    │   │
      │<─────────────────────────────────┤   │
      │                                  │   │
      │ [this](auto *owner, void *pChk)  │   │
      │ {                                │   │
      │   lock(m_mutex);                 │   │
      │   if (chkUniBase(pChk))          │   │
      │     owner->destroy();            │   │
      │ }                                │   │
      │──────────────────────────────────>   │
      │                                  │   ├─ lock()
      │                                  │   └─ 繼續下一個
      │                                  │
      │                                  ├─ unlock()
      │                                  │
      │                          7. LD_clearFFlag()
      │                                  │
      │                                  ├─ 清除所有 fFlag
      │                                  │
      │                                  ▼
      │                              [完成]
```

### 關鍵步驟說明

1. **觸發銷毀** (detachOwner)
   - 從 `m_holderSet` 移除
   - 調用 `removeOwner()` 判斷是否應銷毀

2. **加入佇列** (checkDestroy)
   - 設置 `cFlag` 標記
   - 加入 Destructor 的處理佇列

3. **檢測階段** (LD_shouldDestroy)
   - 設置 `fFlag` 開始檢查
   - 透過 `checkNoHost()` 遞迴搜尋 root
   - 設置 `ldFlag` 記錄結果

4. **銷毀階段** (LD_destroy)
   - 設置 `m_isDestroy` 阻止新的 attach
   - 遍歷所有 holder，調用回調
   - 解鎖後調用，避免死鎖

5. **清理標記** (LD_clearFFlag)
   - 清除檢測時設置的 `fFlag`
   - 為下次檢測做準備

---

## 🎨 設計模式

### 觀察者模式 (Observer Pattern)

```cpp
// Subject
class _UniBase {
    std::unordered_set<const _Holder *> m_holderSet;  // 觀察者列表

    void LD_destroy() {
        // 通知所有觀察者
        for (auto &holder : m_holderSet)
            holder->detachUniBase(this);
    }
};

// Observer
class _Holder {
    virtual void detachUniBase(...) = 0;  // 接收通知
};
```

### 策略模式 (Strategy Pattern)

```cpp
// 策略介面
class _UniBase {
    virtual void addOwner() = 0;
    virtual bool removeOwner() = 0;
};

// 策略 1: ALL
class _UniBaseAll : public _UniBase {
    size_t m_OwnerCount;
    bool removeOwner() { return --m_OwnerCount == 0; }
};

// 策略 2: ONE
class _UniBaseOne : public _UniBase {
    bool removeOwner() { return true; }  // 總是銷毀
};
```

### RAII (資源獲取即初始化)

```cpp
class UniOwner {
    ~UniOwner() {
        destroy();  // 自動釋放資源
    }
};

class UniPtr {
    ~UniPtr() {
        destroyUniBase();  // 自動通知
    }
};
```

### 單例模式 (Singleton)

```cpp
// Destructor 全局單例
UniBaseDestructor g_Destructor;

// 只能獲取一次
std::shared_ptr<IDestrWaiter> getDestructor() {
    static std::atomic<bool> f{false};
    if (!f.compare_exchange_strong(...))
        return nullptr;
    return std::make_shared<DestrWaiter>();
}
```

---

## 🔬 技術細節

### 為什麼使用 Union？

```cpp
union {
    struct {
        bool cFlag : 1;
        bool fFlag : 1;
        // ... 其他 flags
    };
    uint8_t allFlags = 0;  // 快速清零
};

// 用法
allFlags = 0;  // 一次清除所有 flag（高效）
```

**優點**：
- ✅ 記憶體緊湊（只佔 1 byte）
- ✅ 快速清零
- ⚠️ 技術上是 UB（但實務上可行）

### 為什麼使用 std::unique_lock？

```cpp
void LD_destroy() {
    std::unique_lock<std::mutex> lock(m_UniBaseMutex);

    while (...) {
        lock.unlock();  // ⭐ 需要手動解鎖
        callback();
        lock.lock();    // ⭐ 需要重新鎖定
    }
}
```

**原因**：
- `std::lock_guard` 不支援手動 lock/unlock
- 需要在調用回調時釋放鎖，避免死鎖
- 仍享有 RAII 的好處

### 為什麼需要 virtual 繼承？

```cpp
class _UniBaseAll : virtual public _UniBase { };
class _UniBaseOne : virtual public _UniBase { };

// 使用者可能這樣繼承（菱形繼承）
class MyClass : public UniBase<ALL>, public SomeOtherBase { };
```

**原因**：
- 避免菱形繼承的二義性
- 確保只有一個 `_UniBase` 實例
- 強制正確的繼承結構

---

## 📈 性能考量

### 時間複雜度

| 操作 | 複雜度 | 說明 |
|------|--------|------|
| `attach/detach` | O(1) | unordered_set 操作 |
| `checkNoHost` | O(N) | N = 引用鏈深度 |
| `LD_destroy` | O(M) | M = holder 數量 |
| `setUniBase` | O(1) | 簡單賦值 |

### 空間複雜度

| 數據結構 | 大小 | 說明 |
|---------|------|------|
| `_UniBase` | ~80 bytes | mutex + set + flags |
| `UniOwner` | ~32 bytes | shared_ptr + 函數指標 |
| `UniPtr` | ~16 bytes | shared_ptr |

### 優化建議

1. **減少循環檢測深度**
   - 避免過深的嵌套持有
   - 建議最多 5 層

2. **批次操作**
   - 一次設置多個資源
   - 減少頻繁的 attach/detach

3. **使用 UniPtr 傳遞**
   - 支援 move 語義
   - 避免不必要的複製

---

## 🛡️ 安全性保證

### 線程安全

- ✅ 所有公開 API 都有 mutex 保護
- ✅ 使用者回調在持鎖狀態執行
- ✅ Destructor 使用獨立執行緒，避免競態

### 異常安全

- ✅ RAII 確保鎖自動釋放
- ✅ Destructor 在單獨執行緒，異常不會傳播
- ✅ 使用 `noexcept` 標記不拋異常的函數

### 記憶體安全

- ✅ 使用 `std::shared_ptr` 管理生命週期
- ✅ 自動檢測循環引用
- ✅ 無懸空指標（透過 `m_isDestroy` 標記）

---

## 📚 總結

### 核心優勢

1. **自動化** - 無需手動管理複雜的生命週期
2. **安全** - 完整的線程安全和異常安全
3. **智能** - 自動檢測和處理循環引用
4. **靈活** - ALL/ONE 策略適應不同場景
5. **可觀察** - 完整的銷毀通知機制

### 適用場景

- ✅ 複雜的物件關係網
- ✅ 可能出現循環引用
- ✅ 需要銷毀通知
- ✅ 多執行緒環境
- ✅ 資源生命週期管理

### 不適用場景

- ❌ 簡單的資源（用 `std::shared_ptr`）
- ❌ 性能極致要求（有小幅開銷）
- ❌ 即時系統（異步銷毀）

---

**作者**: CxxlMan
**版本**: v1.1.26
**最後更新**: 2025
