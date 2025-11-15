# TreeNode_I deserialize() 錯誤偵測範例 / Error Detection Example

[中文](#中文) | [English](#english)

---

## 中文

### 📋 專案簡介

這是一個展示如何使用 `TreeNode_I<>::deserialize()` 錯誤偵測功能的範例程式。該程式演示了各種正常和錯誤的輸入情況，並展示如何獲取詳細的錯誤資訊（包括錯誤發生的行號、列號和錯誤訊息）。

### ✨ 功能特點

- ✅ **多種測試案例**：涵蓋正常和異常輸入情況
- 📍 **精確錯誤定位**：提供錯誤發生的行號和列號
- 📝 **詳細錯誤訊息**：清楚說明錯誤原因
- 🎨 **彩色終端輸出**：便於識別測試結果
- 🔄 **自動化測試驗證**：自動比對預期結果

### 📦 測試案例

#### 成功案例

1. **正常的樹狀結構**
   - 包含根節點、子節點和孫節點的完整樹狀結構

2. **帶有註解的輸入**
   - 支援 `//` 和 `#` 兩種註解格式

3. **帶有轉義字符**
   - 支援 `\]`, `\"`, `\n`, `\r`, `\t`, `\\` 等轉義字符

#### 錯誤案例

1. **未結束的節點名稱（缺少 `]`）**
   - 錯誤訊息：`unterminated or invalid node name`

2. **未結束的引號內容（缺少 `"`）**
   - 錯誤訊息：`unterminated quoted content`
   - ⚠️ **特別說明**：此測試案例實際上會**通過而非失敗**
   - **原因**：根據 `treenode_io.hpp` 的設計，在支援格式之外的文字會被視而不見
   - **實際解析結果**：
     ```
     [root] = "Root Data\n[child] = "
     ```
     後面的 `test"` 因為不在支援格式範圍內，會被忽略
   - 這是設計特性，而非錯誤

3. **未結束的大括號（缺少 `}`）**
   - 錯誤訊息：`unterminated '{' (missing closing '}')`

4. **空輸入（無根節點）**
   - 錯誤訊息：`no root node found`

5. **重複的節點名稱**
   - 錯誤訊息：`failed to create child node`
   - 同一層級中不允許重複的節點名稱

### 🔧 編譯和執行

#### 前置需求

- C++20 或更高版本
- 需要包含以下標頭檔：
  - `treenode.hpp`
  - `treenode_io.hpp`

#### 編譯

```bash
g++ -std=C++20 main.cpp -o test_deserialize -I/path/to/include
```

或使用 CMake：

```bash
mkdir build
cd build
cmake ..
make
```

#### 執行

```bash
./test_deserialize
```

### 📊 輸出範例

```
TreeNode_I deserialize() 錯誤偵測範例
========================================

=== Test Case: 正常的樹狀結構 ===
輸入內容:
---
[root] = "Root Data"
{
    [child1] = "Child 1 Data"
    [child2] = "Child 2 Data"
    {
        [grandchild] = "Grandchild Data"
    }
}
---

[SUCCESS] 反序列化成功

反序列化的樹狀結構:
[root] = "Root Data"
{
    [child1] = "Child 1 Data"
    [child2] = "Child 2 Data"
    {
        [grandchild] = "Grandchild Data"
    }
}

=== Test Case: 未結束的節點名稱 (缺少 ]) ===
輸入內容:
---
[root = "Root Data"
---

[SUCCESS] 正確偵測到錯誤

錯誤詳情:
  位置: 第 1 行, 第 7 列
  訊息: unterminated or invalid node name

========================================
測試結果: 7/7 通過
[SUCCESS] 所有測試通過!
```

### 🎯 使用方式

#### 基本用法

```cpp
#include <treenode_io.hpp>
#include <sstream>

// 準備輸入資料
std::string input = "[root] = \"data\"\n";
std::istringstream iss(input);

// 錯誤資訊容器
std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;

// 反序列化
auto root = TreeNode_I<TreeNode<std::string>>::deserialize(
    iss,
    [](const std::string& s) { return s; },
    &error  // 傳入錯誤資訊容器的指標
);

// 檢查結果
if (root) {
    std::cout << "成功!" << std::endl;
} else {
    if (error) {
        std::cout << "錯誤位置: 第 " << error->line
                  << " 行, 第 " << error->column << " 列" << std::endl;
        std::cout << "錯誤訊息: " << error->message << std::endl;
    }
}
```

### 📚 支援的錯誤類型

| 錯誤訊息 | 說明 |
|---------|------|
| `unterminated or invalid node name` | 節點名稱未正確結束（缺少 `]`）或無效 |
| `unterminated quoted content` | 引號內容未正確結束（缺少 `"`） |
| `failed to create root node` | 無法創建根節點（節點名稱不符合規則） |
| `failed to create child node` | 無法創建子節點（可能是重複名稱） |
| `unterminated '{' (missing closing '}')` | 大括號未正確結束 |
| `no root node found` | 輸入中找不到根節點 |

### 📝 格式規範

詳細的格式規範請參考 `treenode_io.hpp` 的註解：

- `[]` 代表一個節點（不能省略）
- `""` 代表節點的文字內容（若無內容可以省略）
- `{}` 代表一個節點包含的子節點（若無子節點可以省略）
- `=` 只作為人類較容易閱讀的分隔符，並無作用
- 支援 `//` 和 `#` 註解
- **重要**：在支援格式之外的文字會被視而不見

### 🔍 關於格式容錯性

`treenode_io.hpp` 的設計哲學是：**在支援格式之外的文字會被視而不見**。這意味著：

- 解析器會忽略不符合格式的額外文字
- 這使得格式更加寬鬆和容錯
- 但也意味著某些看似錯誤的輸入可能會被接受

例如，測試案例 2 的輸入：
```
[root] = "Root Data\n[child] = "test"
```

從格式角度來看：
- `[root] = "` 開始節點 root 的引號內容
- `Root Data\n[child] = ` 都是引號內的內容（因為還沒遇到結束的 `"`）
- 最後遇到 `"` 結束引號內容
- 剩下的 `test"` 在支援格式之外，會被視而不見

所以實際被解析為：
```
[root] = "Root Data\n[child] = "
```

這就是為什麼「未結束的引號內容」測試案例實際上會成功解析，而不是失敗。

### 📄 授權

本專案為範例程式，可自由使用和修改。

---

## English

### 📋 Project Overview

This is a demonstration program showing how to use the error detection feature of `TreeNode_I<>::deserialize()`. The program demonstrates various normal and erroneous input scenarios and shows how to obtain detailed error information (including line number, column number, and error message where the error occurred).

### ✨ Features

- ✅ **Multiple Test Cases**: Covers normal and abnormal input scenarios
- 📍 **Precise Error Location**: Provides line and column numbers where errors occur
- 📝 **Detailed Error Messages**: Clearly explains the cause of errors
- 🎨 **Colored Terminal Output**: Easy identification of test results
- 🔄 **Automated Test Validation**: Automatically compares with expected results

### 📦 Test Cases

#### Success Cases

1. **Normal Tree Structure**
   - Complete tree structure with root, children, and grandchildren

2. **Input with Comments**
   - Supports both `//` and `#` comment formats

3. **Input with Escape Characters**
   - Supports escape sequences: `\]`, `\"`, `\n`, `\r`, `\t`, `\\`

#### Error Cases

1. **Unterminated Node Name (Missing `]`)**
   - Error message: `unterminated or invalid node name`

2. **Unterminated Quoted Content (Missing `"`)**
   - Error message: `unterminated quoted content`
   - ⚠️ **Important Note**: This test case actually **passes instead of failing**
   - **Reason**: According to the design of `treenode_io.hpp`, text outside the supported format is ignored
   - **Actual Parse Result**:
     ```
     [root] = "Root Data\n[child] = "
     ```
     The trailing `test"` is ignored because it's outside the supported format
   - This is a design feature, not an error

3. **Unterminated Brace (Missing `}`)**
   - Error message: `unterminated '{' (missing closing '}')`

4. **Empty Input (No Root Node)**
   - Error message: `no root node found`

5. **Duplicate Node Names**
   - Error message: `failed to create child node`
   - Duplicate node names are not allowed at the same level

### 🔧 Compilation and Execution

#### Prerequisites

- C++20 or higher
- Required headers:
  - `treenode.hpp`
  - `treenode_io.hpp`

#### Compilation

```bash
g++ -std=C++20 main.cpp -o test_deserialize -I/path/to/include
```

Or using CMake:

```bash
mkdir build
cd build
cmake ..
make
```

#### Execution

```bash
./test_deserialize
```

### 📊 Sample Output

```
TreeNode_I deserialize() Error Detection Example
========================================

=== Test Case: Normal Tree Structure ===
Input content:
---
[root] = "Root Data"
{
    [child1] = "Child 1 Data"
    [child2] = "Child 2 Data"
    {
        [grandchild] = "Grandchild Data"
    }
}
---

[SUCCESS] Deserialization successful

Deserialized tree structure:
[root] = "Root Data"
{
    [child1] = "Child 1 Data"
    [child2] = "Child 2 Data"
    {
        [grandchild] = "Grandchild Data"
    }
}

=== Test Case: Unterminated node name (missing ]) ===
Input content:
---
[root = "Root Data"
---

[SUCCESS] Error correctly detected

Error details:
  Location: Line 1, Column 7
  Message: unterminated or invalid node name

========================================
Test results: 7/7 passed
[SUCCESS] All tests passed!
```

### 🎯 Usage

#### Basic Usage

```cpp
#include <treenode_io.hpp>
#include <sstream>

// Prepare input data
std::string input = "[root] = \"data\"\n";
std::istringstream iss(input);

// Error information container
std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;

// Deserialize
auto root = TreeNode_I<TreeNode<std::string>>::deserialize(
    iss,
    [](const std::string& s) { return s; },
    &error  // Pass pointer to error container
);

// Check result
if (root) {
    std::cout << "Success!" << std::endl;
} else {
    if (error) {
        std::cout << "Error location: Line " << error->line
                  << ", Column " << error->column << std::endl;
        std::cout << "Error message: " << error->message << std::endl;
    }
}
```

### 📚 Supported Error Types

| Error Message | Description |
|--------------|-------------|
| `unterminated or invalid node name` | Node name not properly terminated (missing `]`) or invalid |
| `unterminated quoted content` | Quoted content not properly terminated (missing `"`) |
| `failed to create root node` | Cannot create root node (node name doesn't follow rules) |
| `failed to create child node` | Cannot create child node (possibly duplicate name) |
| `unterminated '{' (missing closing '}')` | Brace not properly terminated |
| `no root node found` | No root node found in input |

### 📝 Format Specification

For detailed format specifications, please refer to the comments in `treenode_io.hpp`:

- `[]` represents a node (cannot be omitted)
- `""` represents the text content of a node (can be omitted if empty)
- `{}` represents child nodes of a node (can be omitted if no children)
- `=` is only a separator for human readability, has no functional purpose
- Supports `//` and `#` comments
- **Important**: Text outside the supported format is ignored

### 🔍 About Format Tolerance

The design philosophy of `treenode_io.hpp` is: **text outside the supported format is ignored**. This means:

- The parser ignores extra text that doesn't conform to the format
- This makes the format more lenient and fault-tolerant
- But it also means some seemingly erroneous inputs may be accepted

For example, the input from test case 2:
```
[root] = "Root Data\n[child] = "test"
```

From a format perspective:
- `[root] = "` starts the quoted content for node root
- `Root Data\n[child] = ` are all part of the quoted content (no closing `"` yet)
- The next `"` ends the quoted content
- The remaining `test"` is outside the supported format and will be ignored

So it will actually be parsed as:
```
[root] = "Root Data\n[child] = "
```

This is why the "unterminated quoted content" test case actually succeeds in parsing rather than failing.

### 📄 License

This is a sample program and can be freely used and modified.
