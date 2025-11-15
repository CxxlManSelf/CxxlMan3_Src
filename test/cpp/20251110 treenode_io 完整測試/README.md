# TreeNode_IO 完整測試程式 / TreeNode_IO Comprehensive Test Suite

[繁體中文](#繁體中文) | [English](#english)

---

## 繁體中文

這是針對 `treenode_io.hpp` 的全面測試套件，包含 28 個測試案例，涵蓋所有主要功能和邊界情況。

### 📊 測試概覽

✅ **總測試數**: 28
✅ **通過率**: 100%
✅ **程式碼註解**: 完整繁體中文註解

### 🧪 測試類別

#### 1. 基本序列化測試（5 個測試）
- ✓ `serialize_simple_root` - 序列化單一根節點
- ✓ `serialize_with_children` - 序列化包含子節點的樹
- ✓ `serialize_nested_tree` - 序列化多層嵌套的樹
- ✓ `serialize_empty_data` - 序列化空資料的節點
- ✓ `serialize_with_indent` - 序列化帶縮排的輸出

#### 2. 跳脫字元測試（3 個測試）
- ✓ `escape_special_chars_in_name` - 節點名稱中的特殊字元跳脫（如 `]`）
- ✓ `escape_special_chars_in_data` - 資料中的特殊字元跳脫（如 `"`, `\`）
- ✓ `escape_newline_tab_chars` - 換行符和 Tab 字元的跳脫（`\n`, `\t`, `\r`）

#### 3. 基本反序列化測試（5 個測試）
- ✓ `deserialize_simple_root` - 反序列化單一根節點
- ✓ `deserialize_with_children` - 反序列化包含子節點的樹
- ✓ `deserialize_nested_tree` - 反序列化多層嵌套的樹
- ✓ `deserialize_empty_data` - 反序列化空資料
- ✓ `deserialize_unescape_chars` - 反序列化跳脫字元

#### 4. 註解處理測試（3 個測試）
- ✓ `deserialize_with_line_comments` - 處理 `//` 單行註解
- ✓ `deserialize_with_hash_comments` - 處理 `#` 單行註解
- ✓ `deserialize_mixed_comments` - 混合使用不同註解

#### 5. 錯誤處理測試（4 個測試）
- ✓ `deserialize_error_unterminated_name` - 未結束的節點名稱
- ✓ `deserialize_error_unterminated_quote` - 未結束的引號
- ✓ `deserialize_error_unbalanced_braces` - 不匹配的大括號
- ✓ `deserialize_error_no_root` - 沒有根節點

#### 6. 往返測試（2 個測試）
- ✓ `roundtrip_simple_tree` - 簡單樹的序列化→反序列化往返
- ✓ `roundtrip_complex_tree` - 複雜樹（含特殊字元）的往返測試

#### 7. 自訂資料型別轉換測試（1 個測試）
- ✓ `custom_data_type_int` - 使用整數型別的序列化與反序列化

#### 8. 邊界情況測試（5 個測試）
- ✓ `edge_case_very_long_name` - 非常長的節點名稱（1000 字元）
- ✓ `edge_case_very_long_data` - 非常長的資料（10000 字元）
- ✓ `edge_case_deep_nesting` - 深層嵌套（100 層）
- ✓ `edge_case_many_siblings` - 大量兄弟節點（1000 個）
- ✓ `edge_case_unicode_content` - Unicode 內容（中文、日文、Emoji）

### 🎯 測試的功能覆蓋

#### 序列化功能
- ✅ 基本節點序列化
- ✅ 嵌套結構序列化
- ✅ 空資料處理
- ✅ 縮排格式化
- ✅ 特殊字元跳脫
- ✅ 自訂資料型別轉換

#### 反序列化功能
- ✅ 基本節點解析
- ✅ 嵌套結構解析
- ✅ 註解處理（`//` 和 `#`）
- ✅ 跳脫字元還原
- ✅ 錯誤檢測與報告（行號、列號、錯誤訊息）
- ✅ 自訂資料型別轉換

#### 支援的跳脫字元

**在節點名稱中**：
| 原字元 | 跳脫後 | 說明 |
|-------|--------|------|
| `]` | `\]` | 右中括號 |
| `\` | `\\` | 反斜線 |
| 換行 | `\n` | 換行符號 |
| 歸位 | `\r` | 歸位符號 |
| Tab | `\t` | Tab 字元 |

**在資料內容中**：
| 原字元 | 跳脫後 | 說明 |
|-------|--------|------|
| `"` | `\"` | 雙引號 |
| `\` | `\\` | 反斜線 |
| 換行 | `\n` | 換行符號 |
| 歸位 | `\r` | 歸位符號 |
| Tab | `\t` | Tab 字元 |

### 🔧 建置與執行

#### 建置測試
```bash
cd build
cmake ..
cmake --build .
```

#### 執行測試
```bash
# Windows
./MyTest.exe

# Linux/macOS
./MyTest
```

### 📝 測試範例輸出

#### 成功案例
```
=== 測試 serialize_simple_root ===
序列化結果:
[root] = "根節點資料"

✓ 通過
```

#### 錯誤處理案例
```
=== 測試 deserialize_error_unterminated_name ===
錯誤: unterminated or invalid node name (第 1 行, 第 1 列)
✓ 通過
```

#### 複雜結構案例
```
=== 測試 roundtrip_complex_tree ===
序列化結果:
[文件] = "這是\"文件\"內容\\包含\\特殊\n字元\t和Tab"
{
  [章節\]1] = "第一章\n內容"
  {
    [小節1.1] = "細節\"資料\""
    [小節1.2]
  }
  [章節2] = "第二章"
}

✓ 通過
```

### 🛠️ 測試框架說明

本測試程式使用自訂的簡單測試框架，包含：

#### 測試巨集
```cpp
TEST(name)                        // 定義測試函數
ASSERT(condition, message)        // 斷言條件
ASSERT_EQ(a, b, message)          // 斷言相等
ASSERT_EQ_U8(a, b, message)       // u8string 專用斷言
```

#### 測試統計
程式會自動統計：
- 總測試數
- 通過數量
- 失敗數量

### ⚠️ 錯誤類型覆蓋

反序列化時可能遇到的錯誤：

| 錯誤訊息 | 說明 | 提供資訊 |
|---------|------|---------|
| `unterminated or invalid node name` | 節點名稱未正確結束 | 行號、列號 |
| `unterminated quoted content` | 引號內容未結束 | 行號、列號 |
| `failed to create root node` | 無法建立根節點（名稱不合法） | 行號、列號 |
| `failed to create child node` | 無法建立子節點（名稱不合法） | 行號、列號 |
| `unterminated '{' (missing closing '}')` | 大括號不匹配 | 行號、列號 |
| `no root node found` | 找不到根節點 | - |

### 🚀 效能考量

測試中包含了效能相關的邊界案例：

| 測試項目 | 測試值 | 狀態 |
|---------|-------|------|
| **深度** | 100 層深的嵌套 | ✅ 通過 |
| **廣度** | 1000 個兄弟節點 | ✅ 通過 |
| **長度** | 10000 字元的資料 | ✅ 通過 |
| **名稱** | 1000 字元的節點名稱 | ✅ 通過 |
| **Unicode** | 中文、日文、Emoji | ✅ 通過 |

### 📚 檔案格式範例

#### 基本格式
```
// 這是註解
[根節點] = "節點資料"
{
  [子節點1] = "資料1"
  {
    [孫節點] = "嵌套資料"
  }
  [子節點2] = "資料2"
}
```

#### 格式外文字處理

**重要提示**：在支援格式之外的文字會被視而不見，可直接用於註解。

但是，如果註解文字包含支援格式的識別字（如 `[`, `]`, `{`, `}`, `"`），為了避免干擾解析，請使用 `//` 或 `#` 正規註解起來。

**建議做法**：為了清楚起見，建議一律採用正規註解（`//` 或 `#`）。

```
這是格式外文字，會被忽略
[root] = "資料"

// 這樣更清楚 - 使用正規註解
# 井號註解也可以
[child] = "子資料"

這裡的文字包含 [括號] 可能會干擾
應該改用：
// 這裡的文字包含 [括號] 就不會干擾了
```

### ✅ 結論

此測試套件全面驗證了 `treenode_io.hpp` 的功能，確保：
- ✅ 序列化與反序列化的正確性
- ✅ 特殊字元的正確處理
- ✅ 錯誤情況的妥善處理
- ✅ 大規模資料的穩定性
- ✅ Unicode 字元的完整支援

**所有 28 個測試全部通過**，證明 `treenode_io.hpp` 實作完善可靠。

---

## English

This is a comprehensive test suite for `treenode_io.hpp`, containing 28 test cases covering all major features and edge cases.

### 📊 Test Overview

✅ **Total Tests**: 28
✅ **Pass Rate**: 100%
✅ **Code Comments**: Full Traditional Chinese comments

### 🧪 Test Categories

#### 1. Basic Serialization Tests (5 tests)
- ✓ `serialize_simple_root` - Serialize a single root node
- ✓ `serialize_with_children` - Serialize tree with child nodes
- ✓ `serialize_nested_tree` - Serialize multi-level nested tree
- ✓ `serialize_empty_data` - Serialize nodes with empty data
- ✓ `serialize_with_indent` - Serialize with indentation

#### 2. Escape Character Tests (3 tests)
- ✓ `escape_special_chars_in_name` - Escape special chars in node names (e.g., `]`)
- ✓ `escape_special_chars_in_data` - Escape special chars in data (e.g., `"`, `\`)
- ✓ `escape_newline_tab_chars` - Escape newlines and tabs (`\n`, `\t`, `\r`)

#### 3. Basic Deserialization Tests (5 tests)
- ✓ `deserialize_simple_root` - Deserialize a single root node
- ✓ `deserialize_with_children` - Deserialize tree with children
- ✓ `deserialize_nested_tree` - Deserialize multi-level nested tree
- ✓ `deserialize_empty_data` - Deserialize empty data
- ✓ `deserialize_unescape_chars` - Unescape escape sequences

#### 4. Comment Handling Tests (3 tests)
- ✓ `deserialize_with_line_comments` - Handle `//` line comments
- ✓ `deserialize_with_hash_comments` - Handle `#` hash comments
- ✓ `deserialize_mixed_comments` - Handle mixed comment styles

#### 5. Error Handling Tests (4 tests)
- ✓ `deserialize_error_unterminated_name` - Unterminated node name
- ✓ `deserialize_error_unterminated_quote` - Unterminated quote
- ✓ `deserialize_error_unbalanced_braces` - Unbalanced braces
- ✓ `deserialize_error_no_root` - No root node found

#### 6. Round-trip Tests (2 tests)
- ✓ `roundtrip_simple_tree` - Simple tree serialize→deserialize round-trip
- ✓ `roundtrip_complex_tree` - Complex tree with special characters

#### 7. Custom Data Type Tests (1 test)
- ✓ `custom_data_type_int` - Integer type serialization/deserialization

#### 8. Edge Case Tests (5 tests)
- ✓ `edge_case_very_long_name` - Very long node name (1000 chars)
- ✓ `edge_case_very_long_data` - Very long data (10000 chars)
- ✓ `edge_case_deep_nesting` - Deep nesting (100 levels)
- ✓ `edge_case_many_siblings` - Many siblings (1000 nodes)
- ✓ `edge_case_unicode_content` - Unicode content (Chinese, Japanese, Emoji)

### 🎯 Feature Coverage

#### Serialization Features
- ✅ Basic node serialization
- ✅ Nested structure serialization
- ✅ Empty data handling
- ✅ Indentation formatting
- ✅ Special character escaping
- ✅ Custom data type conversion

#### Deserialization Features
- ✅ Basic node parsing
- ✅ Nested structure parsing
- ✅ Comment handling (`//` and `#`)
- ✅ Escape sequence restoration
- ✅ Error detection with line/column reporting
- ✅ Custom data type conversion

#### Supported Escape Sequences

**In Node Names**:
| Character | Escaped | Description |
|-----------|---------|-------------|
| `]` | `\]` | Right bracket |
| `\` | `\\` | Backslash |
| Newline | `\n` | Line feed |
| Return | `\r` | Carriage return |
| Tab | `\t` | Tab character |

**In Data Content**:
| Character | Escaped | Description |
|-----------|---------|-------------|
| `"` | `\"` | Double quote |
| `\` | `\\` | Backslash |
| Newline | `\n` | Line feed |
| Return | `\r` | Carriage return |
| Tab | `\t` | Tab character |

### 🔧 Build and Run

#### Build the Tests
```bash
cd build
cmake ..
cmake --build .
```

#### Run the Tests
```bash
# Windows
./MyTest.exe

# Linux/macOS
./MyTest
```

### 📝 Sample Output

#### Success Case
```
=== Test serialize_simple_root ===
Serialization result:
[root] = "Root node data"

✓ Passed
```

#### Error Handling Case
```
=== Test deserialize_error_unterminated_name ===
Error: unterminated or invalid node name (Line 1, Column 1)
✓ Passed
```

#### Complex Structure Case
```
=== Test roundtrip_complex_tree ===
Serialization result:
[Document] = "This is \"document\" content\\with\\special\nchars\tand Tab"
{
  [Section\]1] = "Chapter 1\nContent"
  {
    [Subsection1.1] = "Detail\"data\""
    [Subsection1.2]
  }
  [Section2] = "Chapter 2"
}

✓ Passed
```

### 🛠️ Test Framework

Custom lightweight test framework with:

#### Test Macros
```cpp
TEST(name)                        // Define test function
ASSERT(condition, message)        // Assert condition
ASSERT_EQ(a, b, message)          // Assert equality
ASSERT_EQ_U8(a, b, message)       // Assert u8string equality
```

#### Test Statistics
Automatically tracks:
- Total test count
- Passed count
- Failed count

### ⚠️ Error Coverage

Deserialization errors:

| Error Message | Description | Info Provided |
|--------------|-------------|---------------|
| `unterminated or invalid node name` | Node name not properly terminated | Line, Column |
| `unterminated quoted content` | Quote not closed | Line, Column |
| `failed to create root node` | Invalid root node name | Line, Column |
| `failed to create child node` | Invalid child node name | Line, Column |
| `unterminated '{' (missing closing '}')` | Mismatched braces | Line, Column |
| `no root node found` | No root node in input | - |

### 🚀 Performance Testing

Edge cases tested:

| Test Item | Test Value | Status |
|-----------|-----------|--------|
| **Depth** | 100-level nesting | ✅ Passed |
| **Width** | 1000 sibling nodes | ✅ Passed |
| **Length** | 10000 char data | ✅ Passed |
| **Name** | 1000 char node name | ✅ Passed |
| **Unicode** | Chinese, Japanese, Emoji | ✅ Passed |

### 📚 File Format Example

#### Basic Format
```
// This is a comment
[RootNode] = "Node data"
{
  [ChildNode1] = "Data 1"
  {
    [GrandChild] = "Nested data"
  }
  [ChildNode2] = "Data 2"
}
```

#### Text Outside Format

**Important Note**: Text outside the supported format is ignored and can be used as comments.

However, if comment text contains format identifiers (such as `[`, `]`, `{`, `}`, `"`), use `//` or `#` formal comments to avoid interfering with parsing.

**Best Practice**: For clarity, it's recommended to always use formal comments (`//` or `#`).

```
This text outside format will be ignored
[root] = "data"

// This is clearer - using formal comments
# Hash comments work too
[child] = "child data"

Text here contains [brackets] which might interfere
Should use instead:
// Text here contains [brackets] won't interfere now
```

### ✅ Conclusion

This test suite comprehensively validates `treenode_io.hpp`, ensuring:
- ✅ Correct serialization and deserialization
- ✅ Proper special character handling
- ✅ Robust error handling
- ✅ Stability with large datasets
- ✅ Full Unicode support

**All 28 tests pass**, proving `treenode_io.hpp` is well-implemented and reliable.

---

## 📄 License

This test suite is part of the CxxlMan3 project.

## 🤝 Contributing

Feel free to submit issues or pull requests to improve the test coverage.

## 📧 Contact

For questions or feedback about this test suite, please open an issue on GitHub.
