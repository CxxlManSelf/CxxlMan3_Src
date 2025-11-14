# TreeNode Usage Example | TreeNode 用法示例

[English](#english) | [中文](#中文)

---

## English

### Overview
This project demonstrates the basic usage of the `TreeNode` class from the CXXL library. It shows how to create tree structures with named nodes and associated data.

### Features
- Create root nodes with custom data types
- Add child nodes with names
- Set and manage node data
- Create hierarchical tree structures (parent-child-grandchild relationships)
- Support for unnamed nodes
- Asynchronous node deletion with `AsyncNodeDeletor`

### Code Example
The [main.cpp](main.cpp) demonstrates:

1. **Creating a root node:**
   ```cpp
   std::shared_ptr<NODE> root = NODE::createRoot(u8"root");
   ```

2. **Adding child nodes:**
   ```cpp
   root->addChild(u8"child1")->setData("hello world child1");
   ```

3. **Creating multi-level hierarchies:**
   ```cpp
   child2->addChild(u8"child2-1")->setData("hello world child2-1");
   ```

4. **Cleaning up resources:**
   ```cpp
   AsyncNodeDeletor::wait();
   ```

### Key Concepts

#### TreeNode Type Definition
```cpp
using NODE = CXXL::TreeNode<std::string>;
```
The TreeNode is templated, allowing you to store any data type in the nodes.

#### Asynchronous Deletion
All `TreeNodeBase` objects are destructed asynchronously by the `AsyncNodeDeletor`. The program ensures all TreeNode objects are created and destroyed within a specific scope before calling `AsyncNodeDeletor::wait()`.

### Build Requirements
- C++ compiler with C++20 support or later
- CXXL library with `treenode.hpp`

### Usage
Compile and run the program to see a demonstration of TreeNode operations:
```bash
# Compile (example)
g++ -std=c++20 main.cpp -o treenode_demo

# Run
./treenode_demo
```

---

## 中文

### 概述
本專案展示了 CXXL 庫中 `TreeNode` 類別的基本用法，演示如何創建具有命名節點和關聯資料的樹狀結構。

### 功能特點
- 使用自訂資料型別創建根節點
- 新增具有名稱的子節點
- 設定和管理節點資料
- 創建階層式樹狀結構（父-子-孫關係）
- 支援無名節點
- 使用 `AsyncNodeDeletor` 進行非同步節點刪除

### 程式碼範例
[main.cpp](main.cpp) 展示了以下內容：

1. **創建根節點：**
   ```cpp
   std::shared_ptr<NODE> root = NODE::createRoot(u8"root");
   ```

2. **新增子節點：**
   ```cpp
   root->addChild(u8"child1")->setData("hello world child1");
   ```

3. **創建多層級階層：**
   ```cpp
   child2->addChild(u8"child2-1")->setData("hello world child2-1");
   ```

4. **清理資源：**
   ```cpp
   AsyncNodeDeletor::wait();
   ```

### 關鍵概念

#### TreeNode 型別定義
```cpp
using NODE = CXXL::TreeNode<std::string>;
```
TreeNode 是模板類別，允許您在節點中儲存任何資料型別。

#### 非同步刪除
所有 `TreeNodeBase` 物件都由 `AsyncNodeDeletor` 非同步解構。程式確保所有 TreeNode 物件在呼叫 `AsyncNodeDeletor::wait()` 之前，都在特定範圍內創建和銷毀。

### 編譯需求
- 支援 C++20 或更新版本的 C++ 編譯器
- 包含 `treenode.hpp` 的 CXXL 庫

### 使用方法
編譯並執行程式以查看 TreeNode 操作的演示：
```bash
# 編譯（範例）
g++ -std=c++20 main.cpp -o treenode_demo

# 執行
./treenode_demo
```

### 樹狀結構示意圖
```
root
├── child1 (data: "hello world child1")
├── child2 (data: "hello world child2")
│   ├── child2-1 (data: "hello world child2-1")
│   ├── child2-2 (data: "hello world child2-2")
│   ├── [unnamed] (data: "hello world child2-3")
│   └── [unnamed]
└── child3
    ├── child3-1 (data: "hello world child3-1")
    └── child3-2 (data: "hello world child3-2")
```

### 注意事項
- 所有 TreeNodeBase 物件必須在呼叫 `AsyncNodeDeletor::wait()` 之前完成創建和銷毀
- 使用智慧指標 `std::shared_ptr` 來管理節點的生命週期
- 支援 UTF-8 編碼的節點名稱（使用 `u8` 前綴）
- 節點可以有資料也可以沒有資料
- 支援無名節點（空字串名稱）

---

## License | 授權
Please refer to the CXXL library license.

請參考 CXXL 庫的授權條款。
