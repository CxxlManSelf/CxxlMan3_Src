# PathNode 階層式異質資料庫系統 / PathNode Hierarchical Heterogeneous Database System

[English](#english) | [中文](#chinese)

---

<a name="chinese"></a>
## 中文說明

這是一個基於 PathNode 設計的階層式異質資料庫系統，實現了購物系統的客戶、產品和訂單管理。

### 系統特色

1. **無需修改源碼即可擴充資料類型**
   - 使用 `IDataBase` 介面作為基礎
   - 透過 `PathNode<std::shared_ptr<IDataBase>>` 實現異質資料儲存
   - 不使用 variant，符合開放封閉原則

2. **完整的序列化/反序列化支援**
   - 基於 TreeNode_io 實現永續儲存
   - 自動識別資料類型並正確反序列化
   - 支援 UTF-8 編碼
   - 正確處理特殊字符（包括 `|` 分隔符）

3. **豐富的資料類型**
   - PersonData - 人員資料（年齡、電子郵件）
   - ProductData - 產品資料（名稱、價格、類別）
   - OrderData - 訂單資料
   - OrderItemData - 訂單項目
   - StringData - 通用字串資料
   - DateData - 日期資料（支援日期比較和驗證）
   - FloatData - 浮點數資料（支援數學運算）
   - EmptyData - 空資料（容器節點）

4. **簡單易用的 UI 界面**
   - 命令列互動式界面
   - 支援客戶、產品、訂單的完整管理
   - 可動態新增自訂屬性
   - 日期和數值計算工具

### 檔案結構

- `database_types.hpp` - 資料類型定義
  - `IDataBase` - 資料基礎介面
  - 所有具體資料類型實作
  - `parseFields()` - 字串解析函數（支援 `|` 字符）

- `database_io.hpp` - 序列化/反序列化系統
  - `DataFactory` - 資料類型工廠
  - `DatabaseIO` - 檔案儲存/載入

- `database_ui.hpp` - 使用者介面
  - `DatabaseUI` - 互動式選單系統
  - 資料管理功能
  - 日期/數值工具

- `main.cpp` - 主程式
  - `createSampleDatabase()` - 創建範例資料

### 編譯與執行

#### 編譯
```bash
# 使用支援 C++20 的編譯器
g++ -std=c++20 -IC:/CxxlMan3/include main.cpp -o database_system
```

#### 執行
```bash
./database_system
```

首次執行會自動創建範例資料庫檔案 `shopping_database.txt`

#### 測試程式
```bash
# 編譯並執行 parseFields 測試
g++ -std=c++20 -IC:/CxxlMan3/include test_parseFields.cpp -o test_parseFields
./test_parseFields

# 編譯並執行日期/浮點數測試
g++ -std=c++20 -IC:/CxxlMan3/include test_new_data_types.cpp -o test_new_data_types
./test_new_data_types
```

### 資料庫結構範例

```
[購物系統]
{
    [customers]
    {
        [張三] = "PersonData| age: '30' email: 'zhang.san@email.com'"
        {
            [hobby] = "StringData| value: '閱讀'"
            [preference] = "StringData| value: '喜歡科技產品'"
        }
        [李四] = "PersonData| age: '25' email: 'li.si@email.com'"
        {
            [hobby] = "StringData| value: '運動'"
        }
    }
    [products]
    {
        [LAPTOP001] = "ProductData| name: '筆記型電腦' basePrice: '25000' category: '3C電子'"
        {
            [brand] = "StringData| value: 'ASUS'"
            [model] = "StringData| value: 'ZenBook 14'"
            [specification] = "StringData| value: 'Intel i7, 16GB RAM, 512GB SSD'"
            [price] = "FloatData| value: '25000.00'"
            [discount] = "FloatData| value: '0.85'"
            [launch_date] = "DateData| date: '2025-11-16'"
        }
        [MOUSE001] = "ProductData| name: '無線滑鼠' basePrice: '1200' category: '3C電子'"
    }
    [orders]
    {
        [ORDER001] = "OrderData| orderDate: '2025-08-17' customerPath: '/customers/張三' totalAmount: '26200'"
        {
            [order_date] = "DateData| date: '2025-11-20'"
            [total_amount] = "FloatData| value: '21250.00'"
            [] = "OrderItemData| productPath: '/products/LAPTOP001' quantity: '1' unitPrice: '25000' subtotal: '21250'"
            [] = "OrderItemData| productPath: '/products/MOUSE001' quantity: '1' unitPrice: '1200' subtotal: '1200'"
        }
    }
}
```

### 如何擴充新的資料類型

1. 在 `database_types.hpp` 中定義新類別，繼承自 `IDataBase`：

```cpp
class MyNewData : public IDataBase
{
    std::string m_field1;
    std::string m_field2;

public:
    std::string getTypeName() const override { return "MyNewData"; }

    std::string serialize() const override
    {
        return "MyNewData| field1: '" + escapeValue(m_field1) +
               "' field2: '" + escapeValue(m_field2) + "'";
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("field1") != fields.end())
        {
            m_field1 = fields["field1"];
            m_field2 = fields["field2"];
            return true;
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<MyNewData>(*this);
    }
};
```

2. 在 `database_io.hpp` 的 `DataFactory::createFromString()` 中新增類型識別：

```cpp
else if (typeName == "MyNewData")
    data = std::make_shared<MyNewData>();
```

3. 完成！無需修改其他源碼

### DateData 使用範例

```cpp
// 創建日期
DateData date1("2025-11-16");
DateData date2;
date2.setDate(2025, 12, 25);

// 日期操作
int year = date1.getYear();   // 2025
int month = date1.getMonth(); // 11
int day = date1.getDay();     // 16
bool valid = date1.isValid(); // true

// 日期比較
bool earlier = date1 < date2; // true
bool same = date1 == date2;   // false

// 序列化
std::string str = date1.serialize();
// "DateData| date: '2025-11-16'"
```

### FloatData 使用範例

```cpp
// 創建浮點數
FloatData price(1299.99);
FloatData discount(0.85);

// 數學運算
FloatData final = price * discount; // 1104.9915
FloatData diff = price - discount;  // 1299.14

// 格式化輸出
std::string str = final.toString(2); // "1104.99"

// 比較運算
bool greater = price > discount; // true
bool equal = price == discount;  // false
```

### UI 功能說明

#### 主選單
1. 客戶管理 - 新增/查看/修改客戶資料
2. 產品管理 - 新增/查看/修改產品資料
3. 訂單管理 - 新增/查看訂單及訂單項目
4. 瀏覽資料庫結構 - 以路徑方式瀏覽整個資料庫
5. 資料工具 (日期/數值) - 日期和數值計算工具
   - 新增日期資料
   - 新增浮點數資料
   - 日期比較
   - 浮點數計算
6. 儲存並離開 - 將變更儲存至檔案
0. 離開 (不儲存) - 放棄變更直接離開

#### 路徑系統
- 使用類似檔案系統的路徑
- `/customers/張三` - 客戶張三的節點
- `/products/LAPTOP001` - 產品 LAPTOP001 的節點
- `/orders/ORDER001` - 訂單 ORDER001 的節點

### 技術特點

#### 1. CRTP 設計模式
使用 TreeNodeBase 的 CRTP 設計，實現高效能的多型

#### 2. 執行緒安全
使用 shared_mutex 提供讀寫鎖機制

#### 3. 智慧記憶體管理
- 使用 shared_ptr 管理節點生命週期
- AsyncNodeDeletor 背景刪除避免阻塞

#### 4. 可擴充架構
- 介面導向設計
- 工廠模式創建資料物件
- 開放封閉原則

#### 5. 特殊字符處理
- `parseFields()` 正確處理值中的 `|` 字符
- 支援逸出字符（`\'`, `\\`, `\n` 等）
- 完整的測試覆蓋

### 依賴項目

- `treenode.hpp` - 樹狀結構基礎類別
- `pathnode.hpp` - 路徑操作擴充
- `treenode_io.hpp` - 序列化/反序列化基礎設施
- `sysdef.hpp` - 系統定義
- `commondef.hpp` - 通用定義

### 注意事項

1. 資料庫檔案使用 UTF-8 編碼
2. 節點名稱不可包含 `/` 字元
3. 節點名稱不可使用保留字：`.`、`..`、`{N}` 格式
4. 訂單項目使用空名稱節點（自動編號為 `{0}`, `{1}`, ...）
5. PersonData 的 name 儲存於節點名稱，而非資料欄位中
6. 序列化格式：`TypeName| field1: 'value1' field2: 'value2'`
7. `|` 字符可以安全地出現在字段值中

### 授權

Copyright CxxlMan 2025

---

<a name="english"></a>
## English Documentation

A hierarchical heterogeneous database system based on PathNode, implementing customer, product, and order management for a shopping system.

### Key Features

1. **Extensible Data Types Without Source Modification**
   - Uses `IDataBase` interface as foundation
   - Implements heterogeneous data storage via `PathNode<std::shared_ptr<IDataBase>>`
   - No variant usage, follows Open-Closed Principle

2. **Complete Serialization/Deserialization Support**
   - Persistent storage based on TreeNode_io
   - Automatic type recognition and deserialization
   - UTF-8 encoding support
   - Proper handling of special characters (including `|` delimiter)

3. **Rich Data Types**
   - PersonData - Person information (age, email)
   - ProductData - Product information (name, price, category)
   - OrderData - Order information
   - OrderItemData - Order line items
   - StringData - Generic string data
   - DateData - Date data (with comparison and validation)
   - FloatData - Floating-point data (with mathematical operations)
   - EmptyData - Empty data (container nodes)

4. **User-Friendly UI**
   - Command-line interactive interface
   - Complete management for customers, products, and orders
   - Dynamic custom attribute addition
   - Date and numeric calculation tools

### File Structure

- `database_types.hpp` - Data type definitions
  - `IDataBase` - Base data interface
  - All concrete data type implementations
  - `parseFields()` - String parsing function (supports `|` character)

- `database_io.hpp` - Serialization/deserialization system
  - `DataFactory` - Data type factory
  - `DatabaseIO` - File save/load operations

- `database_ui.hpp` - User interface
  - `DatabaseUI` - Interactive menu system
  - Data management functions
  - Date/numeric tools

- `main.cpp` - Main program
  - `createSampleDatabase()` - Create sample data

### Build and Run

#### Build
```bash
# Using a C++20 compliant compiler
g++ -std=c++20 -IC:/CxxlMan3/include main.cpp -o database_system
```

#### Run
```bash
./database_system
```

First run automatically creates the sample database file `shopping_database.txt`

#### Test Programs
```bash
# Compile and run parseFields tests
g++ -std=c++20 -IC:/CxxlMan3/include test_parseFields.cpp -o test_parseFields
./test_parseFields

# Compile and run date/float tests
g++ -std=c++20 -IC:/CxxlMan3/include test_new_data_types.cpp -o test_new_data_types
./test_new_data_types
```

### Database Structure Example

```
[Shopping System]
{
    [customers]
    {
        [Zhang San] = "PersonData| age: '30' email: 'zhang.san@email.com'"
        {
            [hobby] = "StringData| value: 'Reading'"
            [preference] = "StringData| value: 'Likes tech products'"
        }
        [Li Si] = "PersonData| age: '25' email: 'li.si@email.com'"
        {
            [hobby] = "StringData| value: 'Sports'"
        }
    }
    [products]
    {
        [LAPTOP001] = "ProductData| name: 'Laptop' basePrice: '25000' category: 'Electronics'"
        {
            [brand] = "StringData| value: 'ASUS'"
            [model] = "StringData| value: 'ZenBook 14'"
            [specification] = "StringData| value: 'Intel i7, 16GB RAM, 512GB SSD'"
            [price] = "FloatData| value: '25000.00'"
            [discount] = "FloatData| value: '0.85'"
            [launch_date] = "DateData| date: '2025-11-16'"
        }
        [MOUSE001] = "ProductData| name: 'Wireless Mouse' basePrice: '1200' category: 'Electronics'"
    }
    [orders]
    {
        [ORDER001] = "OrderData| orderDate: '2025-08-17' customerPath: '/customers/Zhang San' totalAmount: '26200'"
        {
            [order_date] = "DateData| date: '2025-11-20'"
            [total_amount] = "FloatData| value: '21250.00'"
            [] = "OrderItemData| productPath: '/products/LAPTOP001' quantity: '1' unitPrice: '25000' subtotal: '21250'"
            [] = "OrderItemData| productPath: '/products/MOUSE001' quantity: '1' unitPrice: '1200' subtotal: '1200'"
        }
    }
}
```

### How to Add New Data Types

1. Define a new class in `database_types.hpp` inheriting from `IDataBase`:

```cpp
class MyNewData : public IDataBase
{
    std::string m_field1;
    std::string m_field2;

public:
    std::string getTypeName() const override { return "MyNewData"; }

    std::string serialize() const override
    {
        return "MyNewData| field1: '" + escapeValue(m_field1) +
               "' field2: '" + escapeValue(m_field2) + "'";
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("field1") != fields.end())
        {
            m_field1 = fields["field1"];
            m_field2 = fields["field2"];
            return true;
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<MyNewData>(*this);
    }
};
```

2. Add type recognition in `DataFactory::createFromString()` in `database_io.hpp`:

```cpp
else if (typeName == "MyNewData")
    data = std::make_shared<MyNewData>();
```

3. Done! No other source code modification needed

### DateData Usage Example

```cpp
// Create dates
DateData date1("2025-11-16");
DateData date2;
date2.setDate(2025, 12, 25);

// Date operations
int year = date1.getYear();   // 2025
int month = date1.getMonth(); // 11
int day = date1.getDay();     // 16
bool valid = date1.isValid(); // true

// Date comparison
bool earlier = date1 < date2; // true
bool same = date1 == date2;   // false

// Serialization
std::string str = date1.serialize();
// "DateData| date: '2025-11-16'"
```

### FloatData Usage Example

```cpp
// Create floating-point numbers
FloatData price(1299.99);
FloatData discount(0.85);

// Mathematical operations
FloatData final = price * discount; // 1104.9915
FloatData diff = price - discount;  // 1299.14

// Formatted output
std::string str = final.toString(2); // "1104.99"

// Comparison operations
bool greater = price > discount; // true
bool equal = price == discount;  // false
```

### UI Features

#### Main Menu
1. Customer Management - Add/view/modify customer data
2. Product Management - Add/view/modify product data
3. Order Management - Add/view orders and order items
4. Browse Database Structure - Navigate the entire database using paths
5. Data Tools (Date/Numeric) - Date and numeric calculation tools
   - Add date data
   - Add floating-point data
   - Compare dates
   - Calculate with floats
6. Save and Exit - Save changes to file
0. Exit (without saving) - Discard changes and exit

#### Path System
- Uses file system-like paths
- `/customers/Zhang San` - Customer node
- `/products/LAPTOP001` - Product node
- `/orders/ORDER001` - Order node

### Technical Highlights

#### 1. CRTP Design Pattern
Uses TreeNodeBase CRTP design for high-performance polymorphism

#### 2. Thread Safety
Uses shared_mutex for read-write locking

#### 3. Smart Memory Management
- shared_ptr for node lifecycle management
- AsyncNodeDeletor for background deletion without blocking

#### 4. Extensible Architecture
- Interface-oriented design
- Factory pattern for data object creation
- Open-Closed Principle

#### 5. Special Character Handling
- `parseFields()` properly handles `|` characters in values
- Supports escape sequences (`\'`, `\\`, `\n`, etc.)
- Comprehensive test coverage

### Dependencies

- `treenode.hpp` - Tree structure base classes
- `pathnode.hpp` - Path operation extensions
- `treenode_io.hpp` - Serialization/deserialization infrastructure
- `sysdef.hpp` - System definitions
- `commondef.hpp` - Common definitions

### Important Notes

1. Database files use UTF-8 encoding
2. Node names cannot contain `/` character
3. Node names cannot use reserved words: `.`, `..`, `{N}` format
4. Order items use empty-name nodes (auto-numbered as `{0}`, `{1}`, ...)
5. PersonData's name is stored in the node name, not in the data fields
6. Serialization format: `TypeName| field1: 'value1' field2: 'value2'`
7. The `|` character can safely appear within field values

### License

Copyright CxxlMan 2025
