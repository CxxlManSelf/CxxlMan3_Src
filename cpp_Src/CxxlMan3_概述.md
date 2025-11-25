# CxxlMan3 程式庫概述

**版本：** 0.1.0
**作者：** CxxlMan
**C++ 標準：** C++20
**年份：** 2025

## 簡介

CxxlMan3 是一個現代化的 C++20 程式庫，提供了一系列高效、線程安全的資料結構和實用工具。程式庫設計上注重記憶體管理、線程安全以及跨平台兼容性。

## 程式庫架構

CxxlMan3 分為兩個主要模組：

### 1. cxxlcore（核心模組）

提供資源管理的核心功能：

#### UniBase 資源管理系統

一個創新的智能資源共享系統，可精確控制物件的生命週期：

- **UniBase&lt;ALL&gt;**: 所有持有者都放棄持有後才結束共用
- **UniBase&lt;ONE&gt;**: 只要有一個持有者放棄持有就結束共用
- **UniOwner**: 資源管理器，參與物件生命週期管理
- **UniObserver**: 資源觀察者，不參與生命週期管理
- **UniPtr**: 封裝 `std::shared_ptr`，作為暫時性的資源傳輸指標

**特色：**
- 自動檢測循環引用並妥善處理
- 線程安全的資源管理
- 靈活的生命週期控制策略

### 2. cxxlcommon（通用模組）

提供各種實用的資料結構和工具：

#### TreeNode - 線程安全的樹狀容器

一個高效能的階層式樹狀容器，採用 CRTP（Curiously Recurring Template Pattern）架構：

**特色：**
- **O(1) 時間複雜度**的子節點插入、刪除、移動操作
- 使用 `shared_mutex` 提供讀寫鎖機制，確保線程安全
- 支援有名和無名子節點
- 雙重索引系統：名稱索引和位置索引
- 非同步節點刪除，使用線程池避免深度遞迴造成堆疊溢出

**主要功能：**
- `addChild()`, `addFrontChild()`, `addBackChild()`: 新增子節點
- `insertBefore()`, `insertAfter()`: 在指定位置插入節點
- `removeChild()`, `clearChildren()`: 刪除節點
- `moveChildBefore()`, `moveChildAfter()`: 移動節點
- `findChildByName()`: 按名稱查找
- `forEachChild()`: 遍歷子節點

#### PathNode - 路徑操作節點

基於 TreeNode 實作，提供類似檔案系統的路徑操作功能：

**特色：**
- 支援絕對路徑（以 "/" 開頭）和相對路徑
- 特殊路徑標記：`"."` (當前節點)、`".."` (父節點)
- 無名節點使用位置標記：`{0}`, `{1}`, `{2}`...
- 路徑創建時可自動創建中間節點

**主要功能：**
- `getCurrentPath()`: 獲取節點的完整路徑
- `findNodeByPath()`: 根據路徑查找節點
- `createNodeByPath()`: 根據路徑創建節點
- `removeNodeByPath()`: 根據路徑刪除節點
- `listChildren()`: 列出所有子節點
- `getRootNode()`: 獲取根節點

**應用場景：**
- 階層式配置管理
- 異質資料庫結構
- 樹狀選單系統
- 檔案系統模擬

#### TreeNode I/O - 序列化與反序列化

提供樹節點的資料持久化功能：

**特色：**
- 支援序列化為字串格式
- 完整的錯誤檢測機制
- 可自訂資料序列化方式
- 保持樹狀結構完整性

#### ThreadMgr - 線程管理

提供兩種線程池實作，適用不同場景：

**ThreadLimiter**：動態線程池
- 有任務時才創建執行緒
- 可指定最大線程數量
- 任務減少時自動結束空閒線程
- 適合：不定期的任務處理

**ThreadPool**：固定線程池
- 建構時預先創建指定數量的線程
- 線程持續運行直到池銷毀
- 節省線程創建時間
- 適合：持續性的高頻任務

**共同特色：**
- 支援帶參數和回傳值的任務（使用 `std::packaged_task`）
- 線程間通訊支援（透過 `std::future`）
- 可選的 NOTWAIT 模式，適合全域變數使用
- 任務佇列管理

#### DllLoader - 動態連結程式庫載入器

跨平台的 DLL/SO 載入器：

**特色：**
- 統一的跨平台介面（Windows/Linux/macOS）
- 取得的函數使用 `std::function` 包裝
- 智能生命週期管理：所有函數銷毀後才卸載 DLL
- 類型安全的函數獲取

**主要功能：**
```cpp
auto loader = IDllLoader::create("mylib.dll");
if (loader && loader->isValid()) {
    auto func = loader->getProc<int(int, int)>("add");
    int result = func(1, 2);
}
```

#### 其他工具

- **Semaphore**: 信號量實作，用於線程同步
- **LockTime**: 時間鎖定工具
- **rmconst**: const 移除工具

## 設計特色

### 1. 線程安全
- 所有容器和管理器都提供線程安全保證
- 使用 `shared_mutex` 實現讀寫鎖機制
- 避免死鎖的精心設計

### 2. 高效能
- 大量使用 O(1) 時間複雜度的操作
- 雙重索引機制提升查找速度
- 智能的記憶體管理減少分配開銷

### 3. 記憶體安全
- 智能指標管理，避免記憶體洩漏
- 循環引用檢測與處理
- 非同步刪除避免堆疊溢出

### 4. 跨平台支援
- 完整的 Windows/Linux/macOS 支援
- 統一的 API 介面
- 平台相關程式碼隔離

### 5. 現代 C++ 特性
- 使用 C++20 標準
- CRTP 設計模式
- 完善的類型推導
- `[[nodiscard]]` 屬性防止誤用

## 使用範例

### TreeNode 範例
```cpp
#include <cxxlcommon/treenode.hpp>

auto root = CXXL::TreeNode<int>::createRoot(u8"root");
auto child1 = root->addChild(u8"child1");
auto child2 = root->addChild(u8"child2");

child1->setData(100);
child2->setData(200);

root->forEachChild([](const auto& child) {
    std::cout << child->getData() << std::endl;
});
```

### PathNode 範例
```cpp
#include <cxxlcommon/pathnode.hpp>

auto root = CXXL::PathNode<std::string>::createRoot(u8"root");
auto node = root->createNodeByPath(u8"/config/database/host");
node->setData("localhost");

auto found = root->findNodeByPath(u8"/config/database/host");
std::cout << found->getData() << std::endl;
```

### UniBase 範例
```cpp
#include <cxxlcore/unibase.hpp>

class MyResource : public CXXL::UniBase<CXXL::UniBaseType::ALL> {
    // 資源實作
};

CXXL::UniOwner<MyResource> owner1(this,
    [this](auto* owner, void* pChk) {
        std::lock_guard lock(m_mutex);
        if (owner->chkUniBase(pChk))
            owner->destroy();
    });

owner1.setUniBase(CXXL::UniPtr<MyResource>(new MyResource()));
```

### ThreadPool 範例
```cpp
#include <cxxlcommon/threadmgr.hpp>

CXXL::ThreadPool<> pool(4); // 4 個線程

auto future = pool([](int a, int b) { return a + b; }, 10, 20);
if (future.has_value()) {
    int result = future.value().get(); // 30
}
```

## 編譯與使用

### 系統需求
- CMake 3.10.0 或更高版本
- 支援 C++20 的編譯器（GCC 10+, Clang 10+, MSVC 2019+）

### 編譯
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 連結
在您的 CMakeLists.txt 中：
```cmake
add_subdirectory(path/to/CxxlMan3_Src/cpp_Src)
target_link_libraries(your_target cxxlcore cxxlcommon)
```

## 測試與範例

測試程式位於 `C:\MySrc\CxxlMan3_test\test\cpp`，包含：
- Semaphore 和 ThreadMgr 測試
- 線程池通訊測試
- UniOwner 循環參照處理測試
- TreeNode 完整測試
- PathNode 用法介紹
- TreeNode I/O 序列化測試
- DLL Loader 測試

## 授權

本程式庫由 CxxlMan 開發，版權所有 © 2025。

## 技術支援

如需更多資訊或技術支援，請參考測試範例或原始碼註解。
