# TreeNode 完整測試套件 / TreeNode Complete Test Suite

[English](#english) | [繁體中文](#繁體中文)

---

## English

### Overview

This is a comprehensive test suite for the `TreeNode` template class - a thread-safe hierarchical tree container implementation using CRTP (Curiously Recurring Template Pattern). The test suite validates all major functionalities without relying on external testing frameworks like Google Test.

### Features Tested

The test suite covers **11 major categories** with **90+ individual test cases**:

1. **Basic Creation** - Root node creation, naming, parent relationships
2. **Add Children** - `addChild`, `addFrontChild`, `addBackChild`, `insertBefore`, `insertAfter`
3. **Remove Children** - `removeChild`, `removeChildByName`, `removeFrontChild`, `removeBackChild`, `clearChildren`
4. **Find Children** - `findChildByName`, `hasChild`, `getChildAt`, `getChildPosition`
5. **Child Navigation** - `getFirstChild`, `getLastChild`, `getNextChild`, `getPreviousChild`
6. **Move Children** - `moveChildToFront`, `moveChildToBack`, `moveChildBefore`, `moveChildAfter`
7. **Data Operations** - `getData`, `setData` (copy and move semantics)
8. **Named Children** - Name uniqueness constraints, unnamed node handling
9. **Iteration** - `forEachChild`, `forEachChildReverse`, iterators, range-based for loops
10. **Thread Safety** - Concurrent node additions, index consistency validation
11. **Complex Scenarios** - Multi-level tree structures, recursive traversal, real-world use cases

### Key Highlights

✅ **No External Dependencies** - Uses a simple custom testing framework
✅ **Visual Output** - Clear test results with emoji indicators
✅ **Comprehensive Coverage** - Tests all major API functions
✅ **Thread Safety** - Validates concurrent operations
✅ **Real-world Example** - Company organizational structure simulation
✅ **Automatic Reporting** - Pass/fail statistics at the end

### Building and Running

```bash
# Compile (adjust include paths as needed)
g++ -std=c++20 main.cpp -o treenode_test -I/path/to/include -pthread

# Run tests
./treenode_test
```

### Expected Output

```
╔════════════════════════════════════════════════╗
║   TreeNode 完整測試套件                         ║
╚════════════════════════════════════════════════╝

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📋 1. 基本建立功能測試
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  ✅ 測試 1 通過: 建立根節點
  ✅ 測試 2 通過: 根節點名稱正確
  ...

╔════════════════════════════════════════════════╗
║   測試結果統計                                  ║
╚════════════════════════════════════════════════╝
  總測試數: 90
  通過: 90 ✅
  失敗: 0 ❌

  🎉 所有測試都通過了！
```

### Test Structure

Each test function follows a consistent pattern:

```cpp
void test_example()
{
    TEST_SECTION("Test Category Name");

    // Setup
    auto root = TreeNode<int>::createRoot(u8"root");

    // Execute & Verify
    TEST_ASSERT(condition, "Description of what is being tested");
}
```

### TreeNode Class Features

The `TreeNode` class being tested provides:

- **O(1) Operations** - Fast child lookup by name or pointer
- **Thread-Safe** - Uses `shared_mutex` for concurrent access
- **Flexible Naming** - Named children (unique) or unnamed children (non-unique)
- **Rich API** - Comprehensive set of tree manipulation methods
- **Async Deletion** - Uses thread pool for safe node destruction
- **Template-Based** - Can store any data type

### Requirements

- c++20 or later
- Thread support (`std::thread`, `std::shared_mutex`)
- Standard library support for `std::u8string`

### License

Refer to the main project license.

---

## 繁體中文

### 概述

這是 `TreeNode` 模板類別的完整測試套件。`TreeNode` 是一個使用 CRTP（奇異遞迴模板模式）實作的執行緒安全階層式樹狀容器。此測試套件驗證所有主要功能，且不依賴 Google Test 等外部測試框架。

### 測試功能

測試套件涵蓋 **11 個主要類別**，包含 **90+ 個獨立測試案例**：

1. **基本建立功能** - 根節點建立、命名、父子關係
2. **新增子節點** - `addChild`、`addFrontChild`、`addBackChild`、`insertBefore`、`insertAfter`
3. **刪除子節點** - `removeChild`、`removeChildByName`、`removeFrontChild`、`removeBackChild`、`clearChildren`
4. **查找子節點** - `findChildByName`、`hasChild`、`getChildAt`、`getChildPosition`
5. **子節點導航** - `getFirstChild`、`getLastChild`、`getNextChild`、`getPreviousChild`
6. **移動子節點** - `moveChildToFront`、`moveChildToBack`、`moveChildBefore`、`moveChildAfter`
7. **資料操作** - `getData`、`setData`（複製與移動語意）
8. **具名子節點** - 名稱唯一性限制、無名節點處理
9. **迭代功能** - `forEachChild`、`forEachChildReverse`、迭代器、範圍 for 迴圈
10. **執行緒安全性** - 並行新增節點、索引一致性驗證
11. **複雜場景** - 多層樹狀結構、遞迴遍歷、實際應用案例

### 主要特色

✅ **無外部相依** - 使用簡單的自訂測試框架
✅ **視覺化輸出** - 清晰的測試結果與 emoji 指示器
✅ **全面涵蓋** - 測試所有主要 API 功能
✅ **執行緒安全** - 驗證並行操作
✅ **實際範例** - 公司組織架構模擬
✅ **自動報告** - 結束時顯示通過/失敗統計

### 編譯與執行

```bash
# 編譯（請依需求調整 include 路徑）
g++ -std=c++20 main.cpp -o treenode_test -I/path/to/include -pthread

# 執行測試
./treenode_test
```

### 預期輸出

```
╔════════════════════════════════════════════════╗
║   TreeNode 完整測試套件                         ║
╚════════════════════════════════════════════════╝

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📋 1. 基本建立功能測試
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  ✅ 測試 1 通過: 建立根節點
  ✅ 測試 2 通過: 根節點名稱正確
  ...

╔════════════════════════════════════════════════╗
║   測試結果統計                                  ║
╚════════════════════════════════════════════════╝
  總測試數: 90
  通過: 90 ✅
  失敗: 0 ❌

  🎉 所有測試都通過了！
```

### 測試結構

每個測試函數遵循一致的模式：

```cpp
void test_example()
{
    TEST_SECTION("測試類別名稱");

    // 設置
    auto root = TreeNode<int>::createRoot(u8"根");

    // 執行與驗證
    TEST_ASSERT(條件, "測試項目描述");
}
```

### TreeNode 類別功能

被測試的 `TreeNode` 類別提供：

- **O(1) 操作** - 快速依名稱或指標查找子節點
- **執行緒安全** - 使用 `shared_mutex` 支援並行存取
- **靈活命名** - 具名子節點（唯一）或無名子節點（可重複）
- **豐富 API** - 完整的樹狀結構操作方法集
- **非同步刪除** - 使用執行緒池安全解構節點
- **基於模板** - 可儲存任何資料類型

### 系統需求

- c++20 或更新版本
- 執行緒支援（`std::thread`、`std::shared_mutex`）
- 標準函式庫支援 `std::u8string`

### 測試類別詳細說明

#### 1. 基本建立功能測試
測試根節點的建立、名稱設定、父節點關係以及基本的資料存取功能。

#### 2. 新增子節點測試
驗證各種新增子節點的方式，包括：
- 從後端新增（預設）
- 從前端新增
- 在特定節點前/後插入
- 無名稱節點的處理

#### 3. 刪除子節點測試
測試各種刪除操作，確保：
- 單一節點刪除正確
- 依名稱刪除有效
- 前端/後端刪除功能正常
- 清空所有子節點後狀態正確

#### 4. 查找子節點測試
驗證查找功能的準確性：
- 依名稱查找
- 檢查節點是否存在
- 依索引取得節點
- 取得節點在列表中的位置

#### 5. 子節點導航測試
測試在子節點間移動的能力：
- 取得第一個/最後一個子節點
- 取得下一個/上一個兄弟節點
- 邊界條件處理

#### 6. 移動子節點測試
驗證節點重新排序功能：
- 移動到最前/最後
- 移動到特定節點之前/之後
- 移動後索引更新正確

#### 7. 資料操作測試
測試節點資料的儲存與讀取：
- 複製語意（copy semantics）
- 移動語意（move semantics）
- 自訂資料結構支援

#### 8. 具名子節點測試
驗證名稱管理規則：
- 同名節點不可重複
- 無名節點可以有多個
- 刪除後可重新使用相同名稱

#### 9. 迭代功能測試
測試各種遍歷方式：
- 正向遍歷（`forEachChild`）
- 反向遍歷（`forEachChildReverse`）
- 迭代器使用
- C++11 範圍 for 迴圈

#### 10. 執行緒安全性測試
驗證多執行緒環境下的正確性：
- 多個執行緒同時新增節點
- 操作後索引一致性檢查
- 無資料競爭（data race）

#### 11. 複雜場景測試
模擬實際應用場景：
- 建立公司組織架構（多層樹狀結構）
- 部門重組（節點移動）
- 部門裁撤（節點刪除）
- 遞迴統計（深度遍歷）
- 組織重建（清空後重建）

### 自訂測試框架說明

此測試套件使用兩個簡單的巨集：

#### TEST_SECTION(name)
用於標示測試類別的開始，會輸出視覺化的分隔線和類別名稱。

```cpp
TEST_SECTION("測試類別名稱");
```

#### TEST_ASSERT(condition, message)
用於驗證測試條件，自動追蹤測試通過/失敗數量。

```cpp
TEST_ASSERT(root != nullptr, "建立根節點");
```

### 擴展測試套件

如果您想新增自己的測試，請遵循以下步驟：

1. **宣告測試函數**
```cpp
void test_my_feature();
```

2. **在 main() 中呼叫**
```cpp
int main() {
    // ...
    test_my_feature();
    // ...
}
```

3. **實作測試函數**
```cpp
void test_my_feature()
{
    TEST_SECTION("我的功能測試");

    auto root = TreeNode<int>::createRoot(u8"測試");
    // 進行測試...
    TEST_ASSERT(/* 條件 */, "測試描述");
}
```

### 效能考量

雖然這是功能測試而非效能測試，但值得注意：

- 大多數操作為 **O(1)** 時間複雜度（得益於雙重索引設計）
- `getChildPosition` 和 `getChildAt` 為 **O(n)** 操作
- 執行緒安全機制會帶來些微效能開銷
- 非同步刪除器避免解構時的效能瓶頸

### 已知限制

- 測試程式不涵蓋所有邊界情況（如記憶體耗盡）
- 執行緒安全測試為基本測試，不包含壓力測試
- 不測試自訂 `canCreateChild` 鉤子的行為

### 授權條款

請參考主專案的授權條款。

### 貢獻

歡迎提交 Issue 或 Pull Request 來改進這個測試套件！

### 作者

- TreeNode 實作：CxxlMan
- 測試套件：CxxlMan (with Claude Code assistance)

---

## 常見問題 / FAQ

### Q: 為什麼不使用 Google Test？
**A:** 為了保持測試套件的輕量化和獨立性，避免額外的相依性。這個簡單的測試框架已足夠驗證所有功能。

### Q: 如何在 Windows 上編譯？
**A:** 使用 MSVC 或 MinGW：
```bash
# MSVC
cl /std:c++20 /EHsc main.cpp /I"path\to\include"

# MinGW
g++ -std=c++20 main.cpp -o treenode_test.exe -I"path/to/include" -pthread
```

### Q: 測試失敗怎麼辦？
**A:**
1. 檢查輸出中的錯誤訊息
2. 確認 `treenode.hpp` 實作正確
3. 確認編譯器支援 c++20
4. 檢查是否有執行緒支援

### Q: 可以用於生產環境嗎？
**A:** 這是一個測試程式，用於驗證 `TreeNode` 的正確性。如果所有測試通過，則 `TreeNode` 類別可以安全地在生產環境中使用。

### Q: 如何調整執行緒安全測試的強度？
**A:** 修改 `test_thread_safety()` 中的常數：
```cpp
const int threadCount = 10;      // 執行緒數量
const int nodesPerThread = 10;   // 每個執行緒新增的節點數
```

---

**Happy Testing! / 測試愉快！** 🎉
