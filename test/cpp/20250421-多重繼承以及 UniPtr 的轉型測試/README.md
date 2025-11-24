# UniBase Virtual Inheritance and UniPtr Casting Test

[中文](#中文說明) | [English](#english-description)

---

## 中文說明

### 📋 專案簡介

本測試範例用來驗證 CxxlMan3 框架中 **UniBase 的強制虛擬繼承(virtual inheritance)特性**，以及 **UniPtr 智慧指標在多重繼承情境下的類型轉換功能**。

### 🎯 測試目標

1. **菱形繼承處理**: 驗證 UniBase 是否能正確處理菱形繼承(Diamond Problem)
2. **指標安全轉型**: 驗證 UniPtr 的 `cast()` 方法是否能安全地進行指標轉型
3. **解構順序驗證**: 確認解構函式按正確順序呼叫

### 🏗️ 繼承結構

#### 簡化繼承圖
```
           MyBase (UniBase)
           /    \
    MyDerived1  MyDerived2
           \    /
        MultiDerived
```

#### 完整繼承結構圖
```
                    IDestroyable (介面)
                         ↑
                         │ (一般繼承)
                         │
                     _UniBase (基礎實作) ← 全域唯一!
                         ↑
                         │ virtual 繼承 ← UniBase 內部強制
                         │ class _UniBaseOne : virtual public _UniBase
                         │
                   _UniBaseOne (ONE 策略實作) ← 全域唯一!
                         ↑
                         │ virtual 繼承 ← UniBase 內部強制
                         │ UniBase<ONE> : virtual public _UniBaseOne
                         │
              UniBase<UniBaseType::ONE> (公開模板) ← 全域唯一!
                         ↑
                         │ (一般繼承)
                         │
                      MyBase ← 全域唯一!
                      ↗    ↖
                     /      \  virtual 繼承 ← 使用者必須明確指定!
                    /        \  class MyDerived1 : virtual public MyBase
             MyDerived1    MyDerived2
                     \      /
                      ↘    ↙
                  MultiDerived
```

### 🔑 關鍵特性

#### 1. UniBase 的自動保證
- `_UniBase`、`_UniBaseOne`、`UniBase<ONE>` 在整個繼承樹中只有一個實例
- 這是由 UniBase 內部的 virtual 繼承自動保證的
- 使用者不需要手動處理 UniBase 內部層級

#### 2. 使用者的責任
- 如果要建立菱形繼承結構(如 `MyBase`)，使用者**必須明確使用 virtual 繼承**
- 否則會出現編譯錯誤: `'MyBase' is an ambiguous base`
- 這不是 UniBase 的限制，而是 C++ 語言的特性

#### 3. 完整的 Virtual 繼承鏈
```cpp
// ✅ 正確寫法
class MyDerived1 : virtual public MyBase  // 明確指定 virtual
class MyDerived2 : virtual public MyBase  // 明確指定 virtual

// ❌ 錯誤寫法
class MyDerived1 : public MyBase  // 會導致 MyBase 歧義
class MyDerived2 : public MyBase  // 會導致編譯錯誤
```

### 🧪 測試內容

#### 測試 1: 建立多重繼承物件
```cpp
UniPtr<MultiDerived> multiDerived_ptr = new MultiDerived;
multiDerived_ptr->doSomething();  // 呼叫 MultiDerived::doSomething()
```

#### 測試 2: 驗證 MyBase 實例唯一性
```cpp
MyBase* base_via_derived1 = static_cast<MyDerived1*>(multiDerived_ptr.get());
MyBase* base_via_derived2 = static_cast<MyDerived2*>(multiDerived_ptr.get());
MyBase* base_direct = static_cast<MyBase*>(multiDerived_ptr.get());

// 三個指標應該指向同一個 MyBase 實例
assert(base_via_derived1 == base_via_derived2);
assert(base_via_derived2 == base_direct);
```

#### 測試 3: UniPtr 指標轉型
```cpp
// 將 UniPtr<MultiDerived> 轉換為 UniPtr<MyDerived2>
UniPtr<MyDerived2> myDerived2_ptr =
    multiDerived_ptr.cast((MyDerived2*)multiDerived_ptr.get());
myDerived2_ptr->doSomething();  // 仍然呼叫 MultiDerived::doSomething()
```

### 📊 預期輸出

```
Doing something in MultiDerived

驗證 UniBase 實例唯一性:
  base_via_derived1 地址: 0x...
  base_via_derived2 地址: 0x...  (與上面相同)
  base_direct 地址:       0x...  (與上面相同)
  三者是否相同: 是 ✓

Doing something in MyDerived2

Hello, from mytest!
MultiDerived destructor
MyDerived2 destructor
MyDerived1 destructor
MyBase destructor
```

### 🔍 重要觀念

#### Virtual 繼承的兩層責任

| 層級 | 責任方 | 方式 | 說明 |
|------|--------|------|------|
| **UniBase 內部** | 框架自動處理 | `class _UniBaseOne : virtual public _UniBase` | 確保 `_UniBase` 唯一 |
| **使用者層級** | 開發者手動指定 | `class MyDerived1 : virtual public MyBase` | 確保 `MyBase` 唯一 |

#### 記憶體位址偏移

在多重繼承中，子類別物件的記憶體位址和父類別子物件的位址可能不同:

```
MultiDerived 物件記憶體佈局:
┌─────────────────────────────────┐ ← MultiDerived* (0x1000)
│ MultiDerived 資料                │
├─────────────────────────────────┤ ← MyDerived1* (0x1010)
│ MyDerived1 資料 (vptr)           │
├─────────────────────────────────┤ ← MyDerived2* (0x1020)
│ MyDerived2 資料 (vptr)           │
├─────────────────────────────────┤ ← MyBase* (0x1030) ← 三個路徑都指向這裡!
│ MyBase 資料 (只有一份!)          │
├─────────────────────────────────┤
│ UniBase<ONE> 資料                │
├─────────────────────────────────┤
│ _UniBase 資料 (只有一份!)        │
└─────────────────────────────────┘
```

### 🛠️ 編譯與執行

```bash
# 編譯
cmake --build build

# 執行
./build/mytest.exe  # Windows
./build/mytest      # Linux/macOS
```

### 📚 相關文件

- [UniBase 架構說明](https://github.com/your-repo/CxxlMan3/docs/UniBase_Architecture.md)
- [UniPtr 使用指南](https://github.com/your-repo/CxxlMan3/docs/UniPtr_Guide.md)

---

## English Description

### 📋 Project Overview

This test example demonstrates **UniBase's enforced virtual inheritance** and **UniPtr smart pointer type casting** in multiple inheritance scenarios within the CxxlMan3 framework.

### 🎯 Test Objectives

1. **Diamond Inheritance Handling**: Verify UniBase correctly handles the Diamond Problem
2. **Safe Pointer Casting**: Verify UniPtr's `cast()` method safely performs type conversions
3. **Destructor Order Verification**: Ensure destructors are called in the correct order

### 🏗️ Inheritance Structure

#### Simplified Inheritance Diagram
```
           MyBase (UniBase)
           /    \
    MyDerived1  MyDerived2
           \    /
        MultiDerived
```

#### Complete Inheritance Structure
```
                    IDestroyable (interface)
                         ↑
                         │ (regular inheritance)
                         │
                     _UniBase (base implementation) ← Globally unique!
                         ↑
                         │ virtual inheritance ← Enforced by UniBase
                         │ class _UniBaseOne : virtual public _UniBase
                         │
                   _UniBaseOne (ONE policy) ← Globally unique!
                         ↑
                         │ virtual inheritance ← Enforced by UniBase
                         │ UniBase<ONE> : virtual public _UniBaseOne
                         │
              UniBase<UniBaseType::ONE> (public template) ← Globally unique!
                         ↑
                         │ (regular inheritance)
                         │
                      MyBase ← Globally unique!
                      ↗    ↖
                     /      \  virtual inheritance ← User must specify!
                    /        \  class MyDerived1 : virtual public MyBase
             MyDerived1    MyDerived2
                     \      /
                      ↘    ↙
                  MultiDerived
```

### 🔑 Key Features

#### 1. UniBase's Automatic Guarantee
- `_UniBase`, `_UniBaseOne`, `UniBase<ONE>` have only one instance in the inheritance tree
- This is automatically guaranteed by UniBase's internal virtual inheritance
- Users don't need to handle UniBase internal levels manually

#### 2. User's Responsibility
- To create diamond inheritance (like `MyBase`), users **must explicitly use virtual inheritance**
- Otherwise, you'll get a compilation error: `'MyBase' is an ambiguous base`
- This is not a UniBase limitation, but a C++ language feature

#### 3. Complete Virtual Inheritance Chain
```cpp
// ✅ Correct approach
class MyDerived1 : virtual public MyBase  // Explicitly specify virtual
class MyDerived2 : virtual public MyBase  // Explicitly specify virtual

// ❌ Wrong approach
class MyDerived1 : public MyBase  // Will cause MyBase ambiguity
class MyDerived2 : public MyBase  // Will cause compilation error
```

### 🧪 Test Cases

#### Test 1: Create Multiple Inheritance Object
```cpp
UniPtr<MultiDerived> multiDerived_ptr = new MultiDerived;
multiDerived_ptr->doSomething();  // Calls MultiDerived::doSomething()
```

#### Test 2: Verify MyBase Instance Uniqueness
```cpp
MyBase* base_via_derived1 = static_cast<MyDerived1*>(multiDerived_ptr.get());
MyBase* base_via_derived2 = static_cast<MyDerived2*>(multiDerived_ptr.get());
MyBase* base_direct = static_cast<MyBase*>(multiDerived_ptr.get());

// All three pointers should point to the same MyBase instance
assert(base_via_derived1 == base_via_derived2);
assert(base_via_derived2 == base_direct);
```

#### Test 3: UniPtr Pointer Casting
```cpp
// Cast UniPtr<MultiDerived> to UniPtr<MyDerived2>
UniPtr<MyDerived2> myDerived2_ptr =
    multiDerived_ptr.cast((MyDerived2*)multiDerived_ptr.get());
myDerived2_ptr->doSomething();  // Still calls MultiDerived::doSomething()
```

### 📊 Expected Output

```
Doing something in MultiDerived

驗證 UniBase 實例唯一性:
  base_via_derived1 地址: 0x...
  base_via_derived2 地址: 0x...  (same as above)
  base_direct 地址:       0x...  (same as above)
  三者是否相同: 是 ✓

Doing something in MyDerived2

Hello, from mytest!
MultiDerived destructor
MyDerived2 destructor
MyDerived1 destructor
MyBase destructor
```

### 🔍 Important Concepts

#### Two Levels of Virtual Inheritance Responsibility

| Level | Responsible Party | Method | Description |
|-------|------------------|--------|-------------|
| **UniBase Internal** | Framework automatic | `class _UniBaseOne : virtual public _UniBase` | Ensures `_UniBase` uniqueness |
| **User Level** | Developer manual | `class MyDerived1 : virtual public MyBase` | Ensures `MyBase` uniqueness |

#### Memory Address Offset

In multiple inheritance, the memory address of a derived class object may differ from its parent class sub-objects:

```
MultiDerived Object Memory Layout:
┌─────────────────────────────────┐ ← MultiDerived* (0x1000)
│ MultiDerived data                │
├─────────────────────────────────┤ ← MyDerived1* (0x1010)
│ MyDerived1 data (vptr)           │
├─────────────────────────────────┤ ← MyDerived2* (0x1020)
│ MyDerived2 data (vptr)           │
├─────────────────────────────────┤ ← MyBase* (0x1030) ← All three paths point here!
│ MyBase data (single instance!)  │
├─────────────────────────────────┤
│ UniBase<ONE> data                │
├─────────────────────────────────┤
│ _UniBase data (single instance!)│
└─────────────────────────────────┘
```

### 🛠️ Build and Run

```bash
# Build
cmake --build build

# Run
./build/mytest.exe  # Windows
./build/mytest      # Linux/macOS
```

### 📚 Related Documentation

- [UniBase Architecture](https://github.com/your-repo/CxxlMan3/docs/UniBase_Architecture.md)
- [UniPtr Usage Guide](https://github.com/your-repo/CxxlMan3/docs/UniPtr_Guide.md)

---

## 📝 License

This test is part of the CxxlMan3 framework.

## 🤝 Contributing

Feel free to submit issues and pull requests to improve this test example.
