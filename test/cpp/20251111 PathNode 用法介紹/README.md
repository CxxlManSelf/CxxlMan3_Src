# PathNode 用法介紹 / PathNode Usage Introduction

[中文](#中文) | [English](#english)

---

## 中文

### 專案簡介

這是一個展示 `pathnode.hpp` 核心功能的示範程式。`PathNode` 是基於 `TreeNodeBase` 的路徑操作節點類別，提供類似檔案系統的樹狀結構操作能力。

### 核心特性

- **路徑導向操作**：使用類似檔案系統的路徑語法（如 `/a/b/c`）進行節點操作
- **無名節點支援**：允許創建無名稱的節點，並使用位置標記 `{N}` 來引用
- **靈活的節點創建**：支援自動創建中間路徑節點
- **執行緒安全**：使用 `shared_mutex` 提供讀寫鎖機制
- **非同步記憶體管理**：使用執行緒池進行節點的非同步刪除，避免深層遞迴問題

### 程式碼示範

[main.cpp](main.cpp) 包含 8 個完整的示範章節，涵蓋 PathNode 的所有核心功能：

#### 示範 1: 單層無名節點 (//)
展示使用 `//` 創建一層無名節點的基本用法。

#### 示範 2: 多層無名節點 (///)
展示使用 `///` 創建兩層無名節點，理解 `{0}/{0}` 的含義。

#### 示範 3: {N} 索引的真實含義 ⭐
**重點示範**：展示 `{N}` 索引包含所有子節點（有名和無名），但只有無名節點能用 `{N}` 訪問。

#### 示範 4: // vs {N} 的重要差異 ⭐⭐
**核心概念**：
- `//` 是創建指令，可以創建不存在的節點
- `{N}` 是引用指令，只能引用已存在的無名節點

#### 示範 5: createIntermediates 參數控制
展示 `createIntermediates` 參數如何控制中間節點的創建行為。

#### 示範 6: // 和 createIntermediates 的交互
展示兩個概念的組合使用，理解「只能創建最後一個節點」的限制。

#### 示範 7: 尾端無名節點
展示使用單個 `/` 結尾創建尾端無名節點。

#### 示範 8: 綜合範例
展示複雜的樹狀結構，混合使用有名和無名節點。

### 路徑語法規則

| 語法 | 說明 | 範例 |
|------|------|------|
| `/` | 絕對路徑（從根節點開始） | `/a/b/c` |
| `//` | 創建**一層**無名中間節點 | `/a//c` → `/a/{0}/c` |
| `///` | 創建**兩層**無名中間節點 | `/a///c` → `/a/{0}/{0}/c` |
| `////` | 創建**三層**無名中間節點（以此類推） | `/a////c` → `/a/{0}/{0}/{0}/c` |
| 尾端 `/` | 創建尾端無名節點 | `/a/b/` → `/a/b/{0}` |
| `{N}` | 引用位置 N 的無名節點 | `/a/{0}/d` |
| `.` | 當前節點 | `./a` |
| `..` | 父節點 | `../sibling` |

### 關鍵行為

#### 連續斜線創建多層無名節點

連續的斜線會創建對應數量的無名節點層級：

```cpp
// 一層無名節點
auto node1 = root->createNodeByPath(u8"/a//c");
// 結果：/a/{0}/c
// 結構：a → 無名節點{0} → c

// 兩層無名節點
auto node2 = root->createNodeByPath(u8"/a///c");
// 結果：/a/{0}/{0}/c
// 結構：a → 無名節點(a下的{0}) → 無名節點(前一個{0}下的{0}) → c

// 三層無名節點
auto node3 = root->createNodeByPath(u8"/a////c");
// 結果：/a/{0}/{0}/{0}/c
// 結構：a → 無名節點{0} → 無名節點{0} → 無名節點{0} → c
```

**規則：**
- `//` = 1 層無名節點
- `///` = 2 層無名節點
- `////` = 3 層無名節點
- 以此類推...

**重要：{N} 索引的真實含義**
```cpp
// 錯誤理解：{0} 和 {1} 是不同層級的標記
// 正確理解：{N} 是該父節點下所有子節點的位置索引（包含有名和無名）

// 範例：理解索引的實際意義
auto node1 = root->createNodeByPath(u8"/parent/named1");
auto node2 = root->createNodeByPath(u8"/parent/");      // 創建無名節點
auto node3 = root->createNodeByPath(u8"/parent/named2");

// 此時 /parent 下的子節點索引為：
// 位置 0: named1   (有名節點，不能用 /parent/{0} 訪問)
// 位置 1: 無名節點  (可以用 /parent/{1} 訪問)
// 位置 2: named2   (有名節點，不能用 /parent/{2} 訪問)

// 多層無名節點的例子：
auto deep = root->createNodeByPath(u8"/app///settings");
// 結果：/app/{0}/{0}/settings
//
// 結構分析：
// - app 下第 0 個子節點是無名節點 (在路徑中顯示為 {0})
// - 這個無名節點下第 0 個子節點又是無名節點 (在路徑中也顯示為 {0})
// - 第二個無名節點下有一個命名子節點 settings
//
// 關鍵理解：getCurrentPath() 中的 {N} 是相對於各自父節點的索引
// 所以兩個 {0} 實際上是不同節點，只是它們都是各自父節點的第 0 個子節點
```

#### `//` vs `{N}` 的重要差異

這是 PathNode 最容易混淆的地方，需要特別注意：

- ✅ **`//` 可以創建不存在的父節點**：`/no//z` 會自動創建 `/no` 和無名節點
  ```cpp
  // 即使 /no 不存在，這個操作也會成功
  auto node = root->createNodeByPath(u8"/no//z");
  // 結果：/no/{0}/z (自動創建了 /no 節點和一個無名節點)
  ```

- ❌ **`{N}` 不會創建節點**：`/no/{0}/z` 如果 `/no/{0}` 不存在則會失敗
  ```cpp
  // 如果 /no 或 /no/{0} 不存在，這個操作會失敗
  auto node = root->createNodeByPath(u8"/no/{0}/z");
  // 返回 nullptr，因為 {0} 必須已經存在
  ```

**為什麼有這個差異？**
- `//`（雙斜線）是一個**創建指令**，明確告訴系統「在這裡創建一個無名節點」
- `{N}`（位置標記）是一個**引用指令**，用於引用已經存在的無名節點，而不是創建新的

**實際應用場景：**
```cpp
// 場景 1: 使用 // 創建結構
auto step1 = root->createNodeByPath(u8"/config//database");
// ✅ 成功：/config/{0}/database

// 場景 2: 使用 {0} 繼續在同一無名節點下工作
auto step2 = root->createNodeByPath(u8"/config/{0}/username");
// ✅ 成功：/config/{0}/username (因為 {0} 在 step1 已創建)

// 場景 3: 錯誤使用 {0} 在不存在的路徑
auto step3 = root->createNodeByPath(u8"/newpath/{0}/data");
// ❌ 失敗：返回 nullptr (因為 /newpath/{0} 不存在)

// 場景 4: 正確做法是先用 // 創建
auto step4 = root->createNodeByPath(u8"/newpath//data");
// ✅ 成功：/newpath/{0}/data
```

#### `createIntermediates` 參數控制

`createNodeByPath()` 函數有第二個參數 `createIntermediates`（預設為 `true`），用於控制是否自動創建中間路徑節點：

```cpp
NODE_PTR createNodeByPath(const std::u8string &path,
                          bool createIntermediates = true)
```

**當 `createIntermediates = true`（預設）：**
```cpp
// 自動創建所有不存在的中間節點
auto node = root->createNodeByPath(u8"/level1/level2/level3", true);
// ✅ 成功：即使 level1 和 level2 都不存在，也會全部創建
// 結果：/level1/level2/level3
```

**當 `createIntermediates = false`：**
```cpp
// 只創建最後一個節點，中間節點必須已經存在
auto node = root->createNodeByPath(u8"/level1/level2/level3", false);
// ❌ 失敗：如果 /level1 或 /level1/level2 不存在，返回 nullptr
// ✅ 成功：只有當 /level1/level2 已經存在時，才會創建 level3
```

**實際應用：**
```cpp
// 情況 1: 快速建立深層結構（推薦用於初始化）
auto deep = root->createNodeByPath(u8"/app/config/database/mysql", true);
// ✅ 一次性創建所有層級

// 情況 2: 嚴格模式（推薦用於運行時安全檢查）
auto safe = root->createNodeByPath(u8"/app/runtime/cache", false);
// ⚠️ 如果 /app/runtime 不存在，會返回 nullptr
// 這可以防止意外創建錯誤的路徑結構
```

**與 `//` 的交互：**
```cpp
// 場景 A: createIntermediates = true，// 在中間位置
auto node1 = root->createNodeByPath(u8"/new//data", true);
// ✅ 成功：創建 /new、無名節點和 data

// 場景 B: createIntermediates = false，// 在中間位置
auto node2 = root->createNodeByPath(u8"/new2//data", false);
// ❌ 失敗：因為 // 不是最後一個節點，即使 /new2 存在也會失敗
// createIntermediates = false 只允許創建路徑的最後一個節點

// 場景 C: createIntermediates = false，// 在最後位置
auto node3 = root->createNodeByPath(u8"/existing//", false);
// ✅ 成功：如果 /existing 存在，可以在最後創建無名節點
// 因為無名節點是路徑的最後一個節點
```

**關鍵理解：**
- `createIntermediates = false` 的限制是「只能創建最後一個節點」
- 這個限制對 `//` 和普通節點名稱都適用
- 如果 `//` 是路徑的最後部分，則不受影響

#### 其他重要行為

- 🔒 **同名節點不可重複**：相同路徑下不能有兩個同名的命名節點
- ♾️ **無名節點可重複**：可以在同一父節點下創建多個無名節點

### 編譯與執行

```bash
# 編譯（需要 C++17 或更高版本）
g++ -std=c++17 -pthread main.cpp -o pathnode_demo

# 執行
./pathnode_demo
```

### 輸出範例

執行 main.cpp 會產生詳細的輸出，展示每個示範的結果：

```
========== 1. Single slash // creates one unnamed node ==========
Created: /a//c
Result:  /a/{0}/c
說明：a 下有一個無名節點（位置 0），無名節點下有 c

========== 2. Triple slash /// creates two unnamed nodes ==========
Created: /b///d
Result:  /b/{0}/{0}/d
說明：兩個 {0} 是不同節點，都是各自父節點的第 0 個子節點

========== 3. Understanding {N} index (includes named and unnamed) ==========
Created nodes under /parent:
  - named1 (位置 0，有名節點，不能用 {0} 訪問)
  - unnamed (位置 1，可用 /parent/{1} 訪問)
  - named2 (位置 2，有名節點，不能用 {2} 訪問)
  - unnamed (位置 3，可用 /parent/{3} 訪問)

Children of /parent:
  - named1
  - {1}
  - named2
  - {3}

/parent/{0} -> Not found (correct)
/parent/{1} -> /parent/{1} (correct)

========== 4. Critical difference: // (create) vs {N} (reference) ==========
使用 // 創建: /newpath//data
結果: /newpath/{0}/data (成功)
說明：即使 /newpath 不存在，// 也會自動創建

使用 {N} 創建: /another/{0}/data
結果: 失敗 (正確)
說明：{0} 必須已經存在，否則失敗

先用 // 創建: /config//settings -> /config/{0}/settings
再用 {0} 引用: /config/{0}/username -> /config/{0}/username (成功)
說明：{0} 在上一步已創建，所以可以引用

... (更多示範輸出)

========== All tests completed ==========
```

### 相依性

- `pathnode.hpp`：核心路徑節點實作
- `treenode.hpp`：基礎樹狀節點容器
- C++17 標準庫（`<memory>`, `<string>`, `<vector>`, 等）

### 授權

請參考專案根目錄的授權文件。

---

## English

### Project Overview

This is a demonstration program showcasing the core functionality of `pathnode.hpp`. `PathNode` is a path-based tree node class built on top of `TreeNodeBase`, providing file-system-like hierarchical structure operations.

### Key Features

- **Path-Oriented Operations**: Use file-system-like path syntax (e.g., `/a/b/c`) for node operations
- **Unnamed Node Support**: Create unnamed nodes and reference them using position markers `{N}`
- **Flexible Node Creation**: Support automatic creation of intermediate path nodes
- **Thread-Safe**: Uses `shared_mutex` for read-write locking mechanism
- **Asynchronous Memory Management**: Employs thread pool for asynchronous node deletion to avoid deep recursion issues

### Code Demonstrations

[main.cpp](main.cpp) contains 8 comprehensive demonstration sections covering all core PathNode features:

#### Demo 1: Single-layer Unnamed Node (//)
Demonstrates the basic usage of `//` to create one unnamed node.

#### Demo 2: Multi-layer Unnamed Nodes (///)
Demonstrates using `///` to create two unnamed nodes, understanding the meaning of `{0}/{0}`.

#### Demo 3: True Meaning of {N} Index ⭐
**Key Demo**: Shows that `{N}` index includes all children (named and unnamed), but only unnamed nodes can be accessed via `{N}`.

#### Demo 4: Critical Difference: // vs {N} ⭐⭐
**Core Concepts**:
- `//` is a creation directive, can create non-existent nodes
- `{N}` is a reference directive, can only reference existing unnamed nodes

#### Demo 5: createIntermediates Parameter Control
Demonstrates how the `createIntermediates` parameter controls intermediate node creation behavior.

#### Demo 6: Interaction of // with createIntermediates
Shows the combined usage of both concepts, understanding the "can only create the last node" restriction.

#### Demo 7: Trailing Unnamed Node
Demonstrates using a single trailing `/` to create a trailing unnamed node.

#### Demo 8: Complex Example
Demonstrates complex tree structures mixing named and unnamed nodes.

### Path Syntax Rules

| Syntax | Description | Example |
|--------|-------------|---------|
| `/` | Absolute path (from root) | `/a/b/c` |
| `//` | Create **one layer** of unnamed intermediate node | `/a//c` → `/a/{0}/c` |
| `///` | Create **two layers** of unnamed intermediate nodes | `/a///c` → `/a/{0}/{0}/c` |
| `////` | Create **three layers** of unnamed nodes (and so on) | `/a////c` → `/a/{0}/{0}/{0}/c` |
| Trailing `/` | Create trailing unnamed node | `/a/b/` → `/a/b/{0}` |
| `{N}` | Reference unnamed node at position N | `/a/{0}/d` |
| `.` | Current node | `./a` |
| `..` | Parent node | `../sibling` |

### Key Behaviors

#### Multiple Consecutive Slashes Create Multiple Layers

Consecutive slashes create corresponding numbers of unnamed node layers:

```cpp
// One layer of unnamed node
auto node1 = root->createNodeByPath(u8"/a//c");
// Result: /a/{0}/c
// Structure: a → unnamed{0} → c

// Two layers of unnamed nodes
auto node2 = root->createNodeByPath(u8"/a///c");
// Result: /a/{0}/{0}/c
// Structure: a → unnamed({0} under a) → unnamed({0} under previous) → c

// Three layers of unnamed nodes
auto node3 = root->createNodeByPath(u8"/a////c");
// Result: /a/{0}/{0}/{0}/c
// Structure: a → unnamed{0} → unnamed{0} → unnamed{0} → c
```

**Rules:**
- `//` = 1 layer of unnamed node
- `///` = 2 layers of unnamed nodes
- `////` = 3 layers of unnamed nodes
- And so on...

**Important: True Meaning of {N} Index**
```cpp
// Wrong understanding: {0} and {1} are markers for different layers
// Correct understanding: {N} is the position index of all children under the parent (including named and unnamed)

// Example: Understanding the actual meaning of indices
auto node1 = root->createNodeByPath(u8"/parent/named1");
auto node2 = root->createNodeByPath(u8"/parent/");      // Create unnamed node
auto node3 = root->createNodeByPath(u8"/parent/named2");

// The child node indices under /parent are:
// Position 0: named1      (named node, cannot access via /parent/{0})
// Position 1: unnamed node (can access via /parent/{1})
// Position 2: named2      (named node, cannot access via /parent/{2})

// Example with multiple layers of unnamed nodes:
auto deep = root->createNodeByPath(u8"/app///settings");
// Result: /app/{0}/{0}/settings
//
// Structure analysis:
// - The 0th child under 'app' is an unnamed node (shown as {0} in path)
// - The 0th child under that unnamed node is another unnamed node (also shown as {0})
// - Under the second unnamed node is a named child 'settings'
//
// Key understanding: {N} in getCurrentPath() is the index relative to each parent
// So the two {0}s are actually different nodes, they're just both the 0th child
// of their respective parents
```

#### Critical Difference: `//` vs `{N}`

This is the most important distinction in PathNode that users often confuse:

- ✅ **`//` can create non-existent parents**: `/no//z` auto-creates `/no` and unnamed node
  ```cpp
  // This succeeds even if /no doesn't exist
  auto node = root->createNodeByPath(u8"/no//z");
  // Result: /no/{0}/z (automatically created /no and an unnamed node)
  ```

- ❌ **`{N}` does not create nodes**: `/no/{0}/z` fails if `/no/{0}` doesn't exist
  ```cpp
  // This fails if /no or /no/{0} doesn't exist
  auto node = root->createNodeByPath(u8"/no/{0}/z");
  // Returns nullptr because {0} must already exist
  ```

**Why this difference?**
- `//` (double slash) is a **creation directive**, explicitly telling the system "create an unnamed node here"
- `{N}` (position marker) is a **reference directive**, used to reference existing unnamed nodes, not create new ones

**Practical Use Cases:**
```cpp
// Scenario 1: Use // to create structure
auto step1 = root->createNodeByPath(u8"/config//database");
// ✅ Success: /config/{0}/database

// Scenario 2: Use {0} to continue working under the same unnamed node
auto step2 = root->createNodeByPath(u8"/config/{0}/username");
// ✅ Success: /config/{0}/username (because {0} was created in step1)

// Scenario 3: Wrong - using {0} on non-existent path
auto step3 = root->createNodeByPath(u8"/newpath/{0}/data");
// ❌ Failure: returns nullptr (because /newpath/{0} doesn't exist)

// Scenario 4: Correct approach - use // to create first
auto step4 = root->createNodeByPath(u8"/newpath//data");
// ✅ Success: /newpath/{0}/data
```

#### `createIntermediates` Parameter Control

The `createNodeByPath()` function has a second parameter `createIntermediates` (defaults to `true`), which controls whether to automatically create intermediate path nodes:

```cpp
NODE_PTR createNodeByPath(const std::u8string &path,
                          bool createIntermediates = true)
```

**When `createIntermediates = true` (default):**
```cpp
// Automatically creates all non-existent intermediate nodes
auto node = root->createNodeByPath(u8"/level1/level2/level3", true);
// ✅ Success: Creates all levels even if level1 and level2 don't exist
// Result: /level1/level2/level3
```

**When `createIntermediates = false`:**
```cpp
// Only creates the final node, intermediate nodes must already exist
auto node = root->createNodeByPath(u8"/level1/level2/level3", false);
// ❌ Failure: Returns nullptr if /level1 or /level1/level2 doesn't exist
// ✅ Success: Only creates level3 if /level1/level2 already exists
```

**Practical Applications:**
```cpp
// Use Case 1: Quickly build deep structures (recommended for initialization)
auto deep = root->createNodeByPath(u8"/app/config/database/mysql", true);
// ✅ Creates all levels in one call

// Use Case 2: Strict mode (recommended for runtime safety checks)
auto safe = root->createNodeByPath(u8"/app/runtime/cache", false);
// ⚠️ Returns nullptr if /app/runtime doesn't exist
// This prevents accidentally creating incorrect path structures
```

**Interaction with `//`:**
```cpp
// Scenario A: createIntermediates = true, // in middle position
auto node1 = root->createNodeByPath(u8"/new//data", true);
// ✅ Success: Creates /new, unnamed node, and data

// Scenario B: createIntermediates = false, // in middle position
auto node2 = root->createNodeByPath(u8"/new2//data", false);
// ❌ Failure: Because // is not the last node, fails even if /new2 exists
// createIntermediates = false only allows creating the final node in the path

// Scenario C: createIntermediates = false, // at end position
auto node3 = root->createNodeByPath(u8"/existing//", false);
// ✅ Success: If /existing exists, can create unnamed node at the end
// Because the unnamed node is the last node in the path
```

**Key Understanding:**
- `createIntermediates = false` restriction is "can only create the last node"
- This restriction applies to both `//` and regular node names
- If `//` is the last part of the path, it's not affected by this restriction

#### Other Important Behaviors

- 🔒 **Named nodes must be unique**: Cannot have duplicate named nodes under same parent
- ♾️ **Unnamed nodes can repeat**: Multiple unnamed nodes allowed under same parent

### Build & Run

```bash
# Compile (requires C++17 or higher)
g++ -std=c++17 -pthread main.cpp -o pathnode_demo

# Run
./pathnode_demo
```

### Sample Output

Running main.cpp produces detailed output demonstrating each feature:

```
========== 1. Single slash // creates one unnamed node ==========
Created: /a//c
Result:  /a/{0}/c
說明：a 下有一個無名節點（位置 0），無名節點下有 c

========== 2. Triple slash /// creates two unnamed nodes ==========
Created: /b///d
Result:  /b/{0}/{0}/d
說明：兩個 {0} 是不同節點，都是各自父節點的第 0 個子節點

========== 3. Understanding {N} index (includes named and unnamed) ==========
Created nodes under /parent:
  - named1 (位置 0，有名節點，不能用 {0} 訪問)
  - unnamed (位置 1，可用 /parent/{1} 訪問)
  - named2 (位置 2，有名節點，不能用 {2} 訪問)
  - unnamed (位置 3，可用 /parent/{3} 訪問)

Children of /parent:
  - named1
  - {1}
  - named2
  - {3}

/parent/{0} -> Not found (correct)
/parent/{1} -> /parent/{1} (correct)

========== 4. Critical difference: // (create) vs {N} (reference) ==========
使用 // 創建: /newpath//data
結果: /newpath/{0}/data (成功)
說明：即使 /newpath 不存在，// 也會自動創建

使用 {N} 創建: /another/{0}/data
結果: 失敗 (正確)
說明：{0} 必須已經存在，否則失敗

先用 // 創建: /config//settings -> /config/{0}/settings
再用 {0} 引用: /config/{0}/username -> /config/{0}/username (成功)
說明：{0} 在上一步已創建，所以可以引用

... (more demo outputs)

========== All tests completed ==========
```

### Dependencies

- `pathnode.hpp`: Core path node implementation
- `treenode.hpp`: Base tree node container
- C++17 Standard Library (`<memory>`, `<string>`, `<vector>`, etc.)

### License

Please refer to the license file in the project root directory.

---

## 作者 / Author

**CxxlMan**

---

## 相關資源 / Related Resources

- [pathnode.hpp Documentation](pathnode.hpp)
- [treenode.hpp Documentation](treenode.hpp)
