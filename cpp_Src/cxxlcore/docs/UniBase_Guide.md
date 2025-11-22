# UniBase 系統完整使用指南

## 📚 目錄

1. [系統概述](#系統概述)
2. [核心組件](#核心組件)
3. [處理機制](#處理機制)
4. [使用範例](#使用範例)
5. [進階主題](#進階主題)
6. [常見問題](#常見問題)

---

## 系統概述

### 🎯 設計目標

UniBase 系統是一個**智能資源共享與生命週期管理系統**，解決以下問題：

1. ✅ **自動管理複雜的物件生命週期**
   - 不需要手動追蹤誰在使用資源
   - 自動在適當時機銷毀物件

2. ✅ **處理循環引用問題**
   - 自動檢測循環引用（A→B→C→A）
   - 避免記憶體洩漏

3. ✅ **線程安全的資源共享**
   - 多執行緒環境下安全使用
   - 避免競態條件和死鎖

4. ✅ **優雅的銷毀通知**
   - 資源銷毀前通知所有持有者
   - 允許清理和善後處理

### 📊 與 std::shared_ptr 的比較

| 特性 | `std::shared_ptr` | UniBase 系統 |
|------|-------------------|--------------|
| **引用計數** | ✅ 自動計數 | ✅ 自動管理 |
| **循環引用處理** | ❌ 需要 weak_ptr | ✅ 自動檢測和處理 |
| **生命週期控制** | 🟡 僅計數歸零時銷毀 | ✅ 靈活的策略（ALL/ONE） |
| **銷毀通知** | ❌ 無通知機制 | ✅ 完整的通知系統 |
| **異步銷毀** | ❌ 同步銷毀 | ✅ 獨立執行緒處理 |
| **使用複雜度** | 🟢 簡單 | 🟡 需要理解機制 |

---

## 核心組件

### 1️⃣ UniBase<UniBaseType>

**基底類**，定義了資源的生命週期策略。

```cpp
// 策略 1: 所有 Owner 都放棄才銷毀（適合共享資源）
class MyResource : public UniBase<UniBaseType::ALL>
{
    // 只有當所有 UniOwner 都放棄持有時才會被銷毀
};

// 策略 2: 任一 Owner 放棄就銷毀（適合獨佔資源）
class MySession : public UniBase<UniBaseType::ONE>
{
    // 任何一個 UniOwner 放棄持有就立即銷毀
};
```

**關鍵特性**：
- ⚠️ `UniBase<ALL>` 和 `UniBase<ONE>` **不可多重繼承**
- ✅ 必須是 `virtual` 繼承（系統已強制）
- ✅ 每個 class 都需要自己的 `std::mutex`

---

### 2️⃣ UniOwner<UNIBASE>

**生命週期管理器**，參與 UniBase 的生命週期決策。

```cpp
class MyClass : public UniBase<UniBaseType::ALL>
{
    std::mutex m_mutex;
    UniOwner<MyResource> m_resource;

public:
    MyClass()
        : m_resource(this,  // ⭐ 第一個參數：host 指標
            [this](UniOwner<MyResource> *owner, void *pChk) {
                // ⭐ 銷毀通知回調（必須遵守以下約定）
                std::lock_guard<std::mutex> lock(m_mutex);  // 1. 使用 host class 的 mutex 上鎖（不是 UniBase 的 mutex）
                if (owner->chkUniBase(pChk))                // 2. 用 chkUniBase(pChk) 檢查是否為對應的 UniBase
                    owner->destroy();                        // 3. 如果匹配，調用 destroy()
            })
    {}

    void setResource(UniPtr<MyResource> res) {
        m_resource.setUniBase(res);  // 設置持有的資源
    }

    UniPtr<MyResource> getResource() {
        return m_resource.getUniBase();  // 獲取資源
    }
};
```

**重要方法**：
- `setUniBase(UniPtr<T>)` - 設置持有的 UniBase（會放棄舊的）
- `getUniBase()` - 獲取當前持有的 UniBase
- `destroy()` - 主動放棄持有
- `chkUniBase(void*)` - 檢查指標是否匹配

---

### 3️⃣ UniObserver<UNIBASE>

**觀察者**，只觀察不參與生命週期管理。

```cpp
class Logger : public UniBase<UniBaseType::ALL>
{
    std::mutex m_mutex;
    UniObserver<MyResource> m_observed;  // ⭐ 使用 Observer

public:
    Logger()
        : m_observed(this,
            [this](UniObserver<MyResource> *obs, void *pChk) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (obs->chkUniBase(pChk)) {
                    obs->destroy();
                    onResourceDestroyed();  // 可以處理銷毀事件
                }
            })
    {}

    void observe(UniPtr<MyResource> res) {
        m_observed.setUniBase(res);
    }

    void onResourceDestroyed() {
        std::cout << "Resource was destroyed!" << std::endl;
    }
};
```

**與 UniOwner 的差異**：
- ✅ 不影響 UniBase 的生命週期
- ✅ 會收到銷毀通知
- ✅ 適合日誌、監控等場景

---

### 4️⃣ UniPtr<UNIBASE>

**智能指標**，類似 `std::shared_ptr`，用於臨時持有。

```cpp
void processResource(UniPtr<MyResource> res) {
    if (res) {
        res->doSomething();
    }
    // 離開作用域時自動處理
}

// 創建 UniPtr
auto res = std::make_shared<MyResource>();
UniPtr<MyResource> ptr(res.get());

// 或從 UniOwner 獲取
UniPtr<MyResource> ptr = owner.getUniBase();
```

**特點**：
- ✅ 支援 move 語義（高效）
- ✅ 自動通知 Destructor
- ✅ 適合函數參數和臨時變數
- ⚠️ 不應該作為成員變數（使用 UniOwner/Observer）

---

## 處理機制

### 🔄 異步銷毀流程

```
[使用者執行緒]                    [Destructor 執行緒]
      │                                  │
      │  detachOwner()                   │
      ├─────────────────────────────────>│
      │                                  │ checkDestroy()
      │                                  ├──> LD_shouldDestroy()
      │                                  │      └─> checkNoHost()
      │                                  │            └─> 遞迴檢查
      │                                  │
      │                                  │ LD_destroy()
      │                                  ├──> 遍歷 m_holderSet
      │  detachUniBaseFunc 回調          │
      │<─────────────────────────────────┤
      │  [在 Destructor 執行緒中執行]     │
      │                                  │
      └──────────────────────────────────┘
```

### 🔍 循環引用檢測

**問題場景**：
```cpp
// A 持有 B，B 持有 C，C 持有 A (循環！)
A → B → C → A
```

**檢測機制**：
1. **標記追蹤**：使用 `fFlag` 標記已訪問的節點
2. **遞迴搜尋**：`checkNoHost()` 遞迴尋找 root UniBase
3. **循環判斷**：如果回到已標記的節點 → 找到循環
4. **自動銷毀**：找不到 root 時，判定為循環並銷毀

**範例**：
```cpp
class Node : public UniBase<UniBaseType::ALL>
{
    std::mutex m_mutex;
    UniOwner<Node> m_next;

public:
    Node() : m_next(this, [this](auto *owner, void *pChk) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (owner->chkUniBase(pChk)) owner->destroy();
    }) {}

    void setNext(UniPtr<Node> next) { m_next.setUniBase(next); }
};

// 創建循環
auto a = std::make_shared<Node>();
auto b = std::make_shared<Node>();
auto c = std::make_shared<Node>();
a->setNext(UniPtr<Node>(b.get()));
b->setNext(UniPtr<Node>(c.get()));
c->setNext(UniPtr<Node>(a.get()));  // 循環！

// 當它們超出作用域，系統會自動檢測並銷毀
```

### 🔒 線程安全保證

**鎖的層級設計**：
```
Level 1: 使用者的 m_mutex (MyClass::m_mutex)
         └─ 保護 class 的成員變數

Level 2: UniBase 的 m_UniBaseMutex
         └─ 保護內部狀態 (m_holderSet, m_isDestroy 等)
```

**避免死鎖的規則**：
1. ✅ `LD_destroy()` 在調用回調前**先解鎖**
2. ✅ 使用者回調使用**不同的 mutex**（自己的 m_mutex）
3. ✅ 兩把鎖**永不同時持有**

---

## 使用範例

### 範例 1：基本資源管理

```cpp
// 定義資源
class DatabaseConnection : public UniBase<UniBaseType::ALL>
{
    std::string m_connectionString;

public:
    DatabaseConnection(const std::string &connStr)
        : m_connectionString(connStr)
    {
        std::cout << "Connecting to " << connStr << std::endl;
    }

    ~DatabaseConnection()
    {
        std::cout << "Disconnecting..." << std::endl;
    }

    void query(const std::string &sql) {
        std::cout << "Executing: " << sql << std::endl;
    }
};

// 使用資源的類
class DataService : public UniBase<UniBaseType::ALL>
{
    std::mutex m_mutex;
    UniOwner<DatabaseConnection> m_dbConn;

public:
    DataService()
        : m_dbConn(this,
            [this](UniOwner<DatabaseConnection> *owner, void *pChk) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (owner->chkUniBase(pChk))
                    owner->destroy();
            })
    {}

    void setConnection(UniPtr<DatabaseConnection> conn) {
        m_dbConn.setUniBase(conn);
    }

    void fetchData() {
        auto conn = m_dbConn.getUniBase();
        if (conn) {
            conn->query("SELECT * FROM users");
        }
    }
};

// 主程式
int main()
{
    // 初始化 Destructor
    auto destructor = CXXL::getDestructor();

    // 創建連接
    auto conn = std::make_shared<DatabaseConnection>("localhost:5432");

    {
        // 創建服務
        auto service = std::make_shared<DataService>();
        service->setConnection(UniPtr<DatabaseConnection>(conn.get()));
        service->fetchData();

        // service 超出作用域...
    }

    // conn 還活著（因為是 UniBase<ALL>）
    conn.reset();  // 現在才會銷毀

    return 0;  // destructor 自動等待所有銷毀完成
}
```

### 範例 2：觀察者模式

```cpp
class EventSource : public UniBase<UniBaseType::ALL>
{
    std::string m_name;

public:
    EventSource(const std::string &name) : m_name(name) {}

    void fireEvent(const std::string &event) {
        std::cout << m_name << " fired: " << event << std::endl;
    }
};

class EventLogger : public UniBase<UniBaseType::ALL>
{
    std::mutex m_mutex;
    UniObserver<EventSource> m_source;
    std::string m_loggerName;

public:
    EventLogger(const std::string &name)
        : m_loggerName(name)
        , m_source(this,
            [this](UniObserver<EventSource> *obs, void *pChk) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (obs->chkUniBase(pChk)) {
                    onSourceDestroyed();
                    obs->destroy();
                }
            })
    {}

    void observeSource(UniPtr<EventSource> src) {
        m_source.setUniBase(src);
        std::cout << m_loggerName << " now observing source" << std::endl;
    }

    void onSourceDestroyed() {
        std::cout << m_loggerName << " detected source destroyed!" << std::endl;
    }
};

// 使用
auto source = std::make_shared<EventSource>("MySource");
auto logger1 = std::make_shared<EventLogger>("Logger1");
auto logger2 = std::make_shared<EventLogger>("Logger2");

logger1->observeSource(UniPtr<EventSource>(source.get()));
logger2->observeSource(UniPtr<EventSource>(source.get()));

source->fireEvent("test");
source.reset();  // 兩個 logger 都會收到通知
```

### 範例 3：樹狀結構（自動處理循環）

```cpp
class TreeNode : public UniBase<UniBaseType::ALL>
{
    std::mutex m_mutex;
    std::string m_name;
    UniOwner<TreeNode> m_parent;
    std::vector<UniOwner<TreeNode>> m_children;

public:
    TreeNode(const std::string &name)
        : m_name(name)
        , m_parent(this, [this](auto *owner, void *pChk) {
              std::lock_guard<std::mutex> lock(m_mutex);
              if (owner->chkUniBase(pChk)) owner->destroy();
          })
    {}

    void setParent(UniPtr<TreeNode> parent) {
        m_parent.setUniBase(parent);
    }

    void addChild(UniPtr<TreeNode> child) {
        m_children.emplace_back(this, [this](auto *owner, void *pChk) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (owner->chkUniBase(pChk)) owner->destroy();
        });
        m_children.back().setUniBase(child);
    }

    ~TreeNode() {
        std::cout << "Destroying node: " << m_name << std::endl;
    }
};

// 創建樹
auto root = std::make_shared<TreeNode>("root");
auto child1 = std::make_shared<TreeNode>("child1");
auto child2 = std::make_shared<TreeNode>("child2");

root->addChild(UniPtr<TreeNode>(child1.get()));
root->addChild(UniPtr<TreeNode>(child2.get()));
child1->setParent(UniPtr<TreeNode>(root.get()));
child2->setParent(UniPtr<TreeNode>(root.get()));

// 形成循環：root ↔ child1, root ↔ child2
// 系統會自動檢測並正確銷毀
```

---

## 進階主題

### 🎛️ ONE vs ALL 策略選擇

**使用 UniBase<ONE>**：
- ✅ 資源有唯一擁有者
- ✅ 類似 `std::unique_ptr` 的語義
- ✅ 例如：網絡連接、文件句柄

**使用 UniBase<ALL>**：
- ✅ 資源可被多方共享
- ✅ 類似 `std::shared_ptr` 的語義
- ✅ 例如：配置物件、緩存數據

### ⚡ 性能優化

**建議**：
1. ✅ 使用 `UniPtr` 傳遞參數（支援 move）
2. ✅ 避免頻繁的 `setUniBase/destroy`
3. ✅ 盡量減少循環引用的深度
4. ✅ 在回調中只做必要的清理

**不建議**：
1. ❌ 在熱路徑上創建/銷毀 UniBase
2. ❌ 過深的嵌套持有關係（超過 5 層）
3. ❌ 在回調中做耗時操作

### 🐛 調試技巧

**常見問題檢查清單**：

1. **記憶體洩漏**
   - □ 檢查是否正確初始化 `getDestructor()`
   - □ 檢查回調是否正確調用 `destroy()`
   - □ 使用記憶體分析工具檢查

2. **死鎖**
   - □ 確認回調使用的是 host 的 mutex
   - □ 確認沒有在回調中持有其他鎖
   - □ 檢查鎖的獲取順序

3. **崩潰**
   - □ 檢查 `chkUniBase()` 是否正確調用
   - □ 檢查是否有懸空指標
   - □ 使用 ASAN 或 Valgrind 檢測

---

## 常見問題

### Q1: 為什麼回調必須用 host class 的 mutex？

**A**: 避免死鎖和確保線程安全。`LD_destroy()` 在調用回調前會釋放 UniBase 的內部 mutex，回調必須使用 host class 自己的 mutex 來保護成員變數。使用者絕對不能嘗試鎖定 UniBase 的內部 mutex，這會造成死鎖。

### Q2: UniPtr 和 std::shared_ptr 可以混用嗎？

**A**: 可以，但要小心。建議：
- 創建時用 `std::shared_ptr`
- 傳遞時用 `UniPtr`
- UniOwner 內部會正確處理

### Q3: 多執行緒環境下安全嗎？

**A**: 完全安全。所有公開 API 都有適當的鎖保護。

### Q4: 可以在回調中訪問其他 UniOwner 嗎？

**A**: 可以，但必須已經持有 mutex。回調在持鎖的情況下執行。

### Q5: 如何處理多個 UniOwner 的情況？

**A**: 每個都需要檢查：
```cpp
class MyClass : public UniBase<UniBaseType::ALL>
{
    std::mutex m_mutex;
    UniOwner<ResA> m_resA;
    UniOwner<ResB> m_resB;

public:
    MyClass()
        : m_resA(this, [this](auto *owner, void *pChk) {
              std::lock_guard<std::mutex> lock(m_mutex);
              if (owner->chkUniBase(pChk)) owner->destroy();
          })
        , m_resB(this, [this](auto *owner, void *pChk) {
              std::lock_guard<std::mutex> lock(m_mutex);
              if (owner->chkUniBase(pChk)) owner->destroy();
          })
    {}
};
```

### Q6: 效能如何？

**A**:
- Destructor 使用獨立執行緒，不阻塞主邏輯
- 循環檢測複雜度：O(N)，N 為引用深度
- 通常情況下開銷很小

---

## 📖 參考資料

- `unibase.hpp` - 核心介面定義
- `unibasedestructor.hpp` - Destructor 介面
- `uniptr.hpp` - UniPtr 實作

**作者**: CxxlMan
**版本**: v1.1.26
**最後更新**: 2025
