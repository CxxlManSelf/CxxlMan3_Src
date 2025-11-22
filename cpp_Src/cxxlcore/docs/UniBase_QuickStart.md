# UniBase 快速入門

## 🚀 5 分鐘上手

### 步驟 1：初始化系統

```cpp
#include <unibase.hpp>
#include <unibasedestructor.hpp>

int main()
{
    // ⭐ 必須：初始化 Destructor
    auto destructor = CXXL::getDestructor();

    // 你的程式碼...

    return 0;  // destructor 自動等待清理完成
}
```

### 步驟 2：定義資源類

```cpp
// 繼承 UniBase<ALL> 或 UniBase<ONE>
class MyResource : public CXXL::UniBase<CXXL::UniBaseType::ALL>
{
public:
    MyResource() { std::cout << "Resource created\n"; }
    ~MyResource() { std::cout << "Resource destroyed\n"; }

    void doWork() { std::cout << "Working...\n"; }
};
```

### 步驟 3：使用 UniOwner 持有資源

```cpp
class MyClass : public CXXL::UniBase<CXXL::UniBaseType::ALL>
{
    std::mutex m_mutex;  // ⭐ 必須：每個 class 都要有 mutex
    CXXL::UniOwner<MyResource> m_resource;

public:
    MyClass()
        : m_resource(this,  // ⭐ 第一個參數：this
            // ⭐ 第二個參數：銷毀回調（複製這個模板）
            [this](CXXL::UniOwner<MyResource> *owner, void *pChk) {
                std::lock_guard<std::mutex> lock(m_mutex);  // 1. 上鎖
                if (owner->chkUniBase(pChk))                // 2. 檢查
                    owner->destroy();                        // 3. 銷毀
            })
    {
        // 設置資源
        auto res = std::make_shared<MyResource>();
        m_resource.setUniBase(CXXL::UniPtr<MyResource>(res.get()));
    }

    void work() {
        auto res = m_resource.getUniBase();
        if (res) {
            res->doWork();
        }
    }
};
```

### 步驟 4：使用

```cpp
{
    auto obj = std::make_shared<MyClass>();
    obj->work();
}  // 離開作用域，自動清理
```

---

## 📝 關鍵要點（必讀）

### ✅ 必須做的事

1. **初始化 Destructor**
   ```cpp
   auto destructor = CXXL::getDestructor();  // 在 main() 開頭
   ```

2. **每個 class 都要有 mutex**
   ```cpp
   class MyClass : public UniBase<...> {
       std::mutex m_mutex;  // ⭐ 必須
   };
   ```

3. **回調必須遵守三步驟**（重要！）
   ```cpp
   [this](auto *owner, void *pChk) {
       std::lock_guard<std::mutex> lock(m_mutex);  // 1. 使用 host class 的 mutex 上鎖（不是 UniBase 的 mutex）
       if (owner->chkUniBase(pChk))                // 2. 用 chkUniBase(pChk) 檢查是否為對應的 UniBase
           owner->destroy();                        // 3. 如果匹配，調用 destroy()
   }
   ```

### ❌ 不要做的事

1. ❌ **不要**忘記初始化 Destructor
2. ❌ **不要**在回調中使用 UniBase 的 mutex（必須使用 host class 的 mutex）
3. ❌ **不要**多重繼承 `UniBase<ALL>` 和 `UniBase<ONE>`
4. ❌ **不要**在回調中省略 `chkUniBase(pChk)` 檢查

---

## 🎯 選擇策略

### 使用 `UniBase<ALL>`（推薦新手先用這個）

```cpp
class MyResource : public UniBase<UniBaseType::ALL>
{
    // 所有 UniOwner 都放棄才會銷毀
    // 類似 std::shared_ptr
};
```

**適合**：配置物件、緩存、共享資源

### 使用 `UniBase<ONE>`

```cpp
class MySession : public UniBase<UniBaseType::ONE>
{
    // 任一 UniOwner 放棄就銷毀
    // 類似 std::unique_ptr
};
```

**適合**：網絡連接、文件句柄、獨佔資源

---

## 📚 完整範例

```cpp
#include <iostream>
#include <memory>
#include <unibase.hpp>
#include <unibasedestructor.hpp>

using namespace CXXL;

// 1. 定義資源
class Database : public UniBase<UniBaseType::ALL>
{
public:
    Database() { std::cout << "DB: Connected\n"; }
    ~Database() { std::cout << "DB: Disconnected\n"; }

    void query(const std::string &sql) {
        std::cout << "DB: " << sql << "\n";
    }
};

// 2. 定義使用者
class Service : public UniBase<UniBaseType::ALL>
{
    std::mutex m_mutex;
    UniOwner<Database> m_db;

public:
    Service()
        : m_db(this,
            [this](UniOwner<Database> *owner, void *pChk) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (owner->chkUniBase(pChk))
                    owner->destroy();
            })
    {
        std::cout << "Service: Created\n";
    }

    ~Service() {
        std::cout << "Service: Destroyed\n";
    }

    void setDB(UniPtr<Database> db) {
        m_db.setUniBase(db);
    }

    void doQuery() {
        auto db = m_db.getUniBase();
        if (db) {
            db->query("SELECT * FROM users");
        }
    }
};

// 3. 主程式
int main()
{
    // ⭐ 初始化
    auto destructor = getDestructor();

    std::cout << "=== Program Start ===\n";

    {
        // 創建資源
        auto db = std::make_shared<Database>();

        {
            // 創建服務
            auto service = std::make_shared<Service>();
            service->setDB(UniPtr<Database>(db.get()));
            service->doQuery();

            std::cout << "--- Service scope end ---\n";
        }  // service 銷毀，但 db 還活著

        std::cout << "--- DB scope end ---\n";
    }  // db 銷毀

    std::cout << "=== Program End ===\n";
    return 0;
}

/* 輸出：
=== Program Start ===
DB: Connected
Service: Created
DB: SELECT * FROM users
--- Service scope end ---
Service: Destroyed
--- DB scope end ---
DB: Disconnected
=== Program End ===
*/
```

---

## 🆚 與 std::shared_ptr 的差異

| 場景 | std::shared_ptr | UniBase |
|------|----------------|---------|
| **基本使用** | `auto p = std::make_shared<T>()` | `UniOwner<T> owner(this, callback)` |
| **循環引用** | 需要 `weak_ptr` 手動處理 | ✅ 自動檢測和處理 |
| **銷毀通知** | 無 | ✅ 透過回調通知 |
| **生命週期控制** | 僅計數 | ✅ ALL/ONE 策略 |
| **線程安全** | 計數安全，內容需自己處理 | ✅ 完整的線程安全 |

---

## ❓ 常見錯誤

### 錯誤 1：忘記初始化 Destructor

```cpp
// ❌ 錯誤
int main() {
    auto obj = std::make_shared<MyClass>();
    return 0;  // 可能記憶體洩漏
}

// ✅ 正確
int main() {
    auto destructor = CXXL::getDestructor();  // ⭐ 必須
    auto obj = std::make_shared<MyClass>();
    return 0;
}
```

### 錯誤 2：回調使用錯誤的 mutex

```cpp
// ❌ 錯誤（會死鎖）
UniOwner<T> m_owner(this,
    [this](auto *owner, void *pChk) {
        // ❌ 不要嘗試鎖定 UniBase 的內部 mutex
        // ❌ 也不要完全不上鎖
        if (owner->chkUniBase(pChk))
            owner->destroy();
    });

// ✅ 正確
UniOwner<T> m_owner(this,
    [this](auto *owner, void *pChk) {
        std::lock_guard<std::mutex> lock(m_mutex);  // ⭐ 必須使用 host class 的 mutex
        if (owner->chkUniBase(pChk))
            owner->destroy();
    });
```

### 錯誤 3：省略 chkUniBase() 檢查

```cpp
// ❌ 錯誤（可能銷毀錯誤的物件）
UniOwner<T> m_owner(this,
    [this](auto *owner, void *pChk) {
        std::lock_guard<std::mutex> lock(m_mutex);
        owner->destroy();  // ❌ 沒檢查
    });

// ✅ 正確
UniOwner<T> m_owner(this,
    [this](auto *owner, void *pChk) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (owner->chkUniBase(pChk))  // ⭐ 必須檢查
            owner->destroy();
    });
```

---

## 📖 下一步

看完這份快速入門後，建議閱讀：

1. **UniBase_Guide.md** - 完整使用指南
   - 深入理解處理機制
   - 進階範例和優化技巧

2. **原始碼註解**
   - `unibase.hpp` - 核心介面
   - `unibasedestructor.hpp` - Destructor 介面

---

**有問題？**
- 檢查是否遵守「必須做的事」
- 參考完整範例
- 閱讀 UniBase_Guide.md

**祝你使用愉快！** 🎉
