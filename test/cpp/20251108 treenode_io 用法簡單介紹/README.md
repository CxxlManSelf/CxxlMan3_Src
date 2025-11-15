# TreeNode I/O Usage Examples

[English](#english) | [中文](#中文)

---

## English

### Overview

This project demonstrates the usage of `treenode_io.hpp` for serializing and deserializing tree structures in C++. The `TreeNode_O` (Output) and `TreeNode_I` (Input) classes provide powerful functionality for converting tree data structures to/from a human-readable text format.

### Features

- **Serialization/Deserialization**: Convert tree structures to text format and back
- **Special Character Handling**: Proper escaping for special characters like `\n`, `\t`, `"`, `[`, `]`, etc.
- **Flexible Indentation**: Support for pretty-printed output with customizable indentation
- **Comment Support**: Parse files with `#` and `//` style comments
- **Error Reporting**: Detailed error messages with line and column information
- **Unicode Support**: Full support for UTF-8 node names and data

### File Format

The serialization format uses a simple, readable syntax:

```
[NodeName] = "NodeData"
{
    [ChildNode1] = "Child1 Data"
    [ChildNode2] = "Child2 Data"
    {
        [GrandchildNode] = "Grandchild Data"
    }
}
```

### Test Cases

The demo includes 6 comprehensive test cases:

#### Test 1: Basic Serialize/Deserialize
Demonstrates basic tree creation, serialization to a string stream, and deserialization back to a tree structure.

#### Test 2: Special Characters and Escaping
Tests handling of special characters including:
- Newlines (`\n`)
- Tabs (`\t`)
- Quotes (`"`)
- Brackets (`[`, `]`)
- Backslashes (`\`)

#### Test 3: Complex Tree Structure
Creates a multi-level organizational tree (company → departments → teams → members) to demonstrate handling of complex hierarchies.

#### Test 4: Indentation Settings
Compares serialization with no indentation (`indent=0`) versus pretty-printed format with custom indentation width.

#### Test 5: Comments in Input
Demonstrates parsing of files containing comments:
- `#` style comments
- `//` style comments
- Comments can appear at end of lines or on separate lines

#### Test 6: Error Handling
Tests various error conditions:
- Unclosed brackets
- Unclosed quotes
- Mismatched braces
- Provides detailed error messages with line/column information

### Usage Example

```cpp
#include <treenode.hpp>
#include <treenode_io.hpp>

// Serialization
auto root = TreeNode<std::string>::createRoot(u8"Root");
root->setData("Root Data");
auto child = root->addBackChild(u8"Child");

std::stringstream ss;
TreeNode_O<TreeNode<std::string>>::serialize(ss, root,
    [](const std::string& s) { return s; }, 2);  // indent=2

// Deserialization
ss.seekg(0);
std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
auto root2 = TreeNode_I<TreeNode<std::string>>::deserialize(ss,
    [](const std::string& s) { return s; }, &error);

if (root2) {
    // Success
} else if (error.has_value()) {
    std::cout << "Error at line " << error->line
              << ", column " << error->column
              << ": " << error->message << std::endl;
}
```

### Building and Running

```bash
# Compile (adjust paths as needed)
g++ -std=c++20 main.cpp -I/path/to/treenode/include -o treenode_io_demo

# Run
./treenode_io_demo
```

### Dependencies

- C++20 or later
- `treenode.hpp` - Core tree node implementation
- `treenode_io.hpp` - I/O serialization functionality

### Key Features of the API

- **Lambda for Data Conversion**: Both serialize and deserialize accept lambda functions for custom data type conversion
- **Async Cleanup**: Uses `AsyncNodeDeletor` for efficient memory management
- **Error Handling**: Optional error parameter provides detailed parsing failure information
- **Stream-based**: Works with any `std::istream`/`std::ostream` implementation

---

## 中文

### 概述

本專案展示了 `treenode_io.hpp` 的使用方法，用於在 C++ 中序列化和反序列化樹狀結構。`TreeNode_O`（輸出）和 `TreeNode_I`（輸入）類別提供了強大的功能，可以將樹狀資料結構轉換為/從人類可讀的文本格式。

### 功能特性

- **序列化/反序列化**：將樹狀結構轉換為文本格式並還原
- **特殊字符處理**：正確轉義特殊字符，如 `\n`、`\t`、`"`、`[`、`]` 等
- **靈活的縮排**：支援美化輸出，可自訂縮排寬度
- **註解支援**：解析包含 `#` 和 `//` 風格註解的文件
- **錯誤報告**：提供詳細的錯誤訊息，包含行號和列號資訊
- **Unicode 支援**：完整支援 UTF-8 節點名稱和資料

### 檔案格式

序列化格式使用簡單、易讀的語法：

```
[節點名稱] = "節點資料"
{
    [子節點1] = "子節點1 資料"
    [子節點2] = "子節點2 資料"
    {
        [孫節點] = "孫節點資料"
    }
}
```

### 測試案例

示範程式包含 6 個完整的測試案例：

#### 測試 1：基本序列化和反序列化
展示基本的樹創建、序列化到字串流，以及反序列化回樹狀結構。

#### 測試 2：特殊字符和轉義
測試特殊字符的處理，包括：
- 換行符號（`\n`）
- 製表符號（`\t`）
- 引號（`"`）
- 方括號（`[`、`]`）
- 反斜杠（`\`）

#### 測試 3：複雜的樹結構
創建多層級的組織樹（公司 → 部門 → 團隊 → 成員），展示處理複雜層次結構的能力。

#### 測試 4：縮排設定
比較無縮排（`indent=0`）和美化格式（自訂縮排寬度）的序列化輸出。

#### 測試 5：含註解的反序列化
展示解析包含註解的檔案：
- `#` 風格註解
- `//` 風格註解
- 註解可以出現在行尾或單獨一行

#### 測試 6：錯誤處理
測試各種錯誤情況：
- 未結束的方括號
- 未結束的引號
- 不匹配的大括號
- 提供詳細的錯誤訊息，包含行號和列號

### 使用範例

```cpp
#include <treenode.hpp>
#include <treenode_io.hpp>

// 序列化
auto root = TreeNode<std::string>::createRoot(u8"根");
root->setData("根節點資料");
auto child = root->addBackChild(u8"子節點");

std::stringstream ss;
TreeNode_O<TreeNode<std::string>>::serialize(ss, root,
    [](const std::string& s) { return s; }, 2);  // 縮排=2

// 反序列化
ss.seekg(0);
std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
auto root2 = TreeNode_I<TreeNode<std::string>>::deserialize(ss,
    [](const std::string& s) { return s; }, &error);

if (root2) {
    // 成功
} else if (error.has_value()) {
    std::cout << "錯誤在第 " << error->line << " 行，"
              << "第 " << error->column << " 列："
              << error->message << std::endl;
}
```

### 編譯和執行

```bash
# 編譯（根據需要調整路徑）
g++ -std=c++20 main.cpp -I/path/to/treenode/include -o treenode_io_demo

# 執行
./treenode_io_demo
```

### 依賴項目

- C++20 或更高版本
- `treenode.hpp` - 核心樹節點實作
- `treenode_io.hpp` - I/O 序列化功能

### API 的主要特性

- **Lambda 資料轉換**：serialize 和 deserialize 都接受 lambda 函數進行自訂資料類型轉換
- **非同步清理**：使用 `AsyncNodeDeletor` 進行高效的記憶體管理
- **錯誤處理**：可選的錯誤參數提供詳細的解析失敗資訊
- **基於串流**：可與任何 `std::istream`/`std::ostream` 實作配合使用

### 輸出範例

執行程式後，您將看到各種測試的輸出，包括：

1. 序列化後的文本格式
2. 反序列化的驗證結果
3. 特殊字符的正確處理
4. 複雜樹結構的完整還原
5. 含註解檔案的解析結果
6. 錯誤情況的詳細報告

### 適用場景

- 配置檔案的讀寫
- 資料結構的持久化
- 樹狀資料的文本表示
- 層次化資訊的儲存和載入
- 需要人類可讀格式的資料交換

---

## License

Please refer to the main project license for usage terms.

## Contributing

This is a demonstration/test file. For issues or suggestions related to the TreeNode library, please contact the library maintainers.
