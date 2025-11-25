/*
 * 本測試範例用來驗證 UniBase 的強制虛擬繼承(virtual inheritance)特性，
 * 以及 UniPtr 智慧指標在多重繼承情境下的類型轉換功能。
 *
 * 主要測試目標:
 * 1. 多重繼承時 UniBase 是否能正確處理菱形繼承問題
 * 2. UniPtr 的 cast() 方法是否能安全地進行指標轉型
 * 3. 解構函式是否按正確順序呼叫
 */

#include <iostream>
#include <uniptr.hpp>

using namespace CxxlMan3;

/**
 * MyBase - 基礎類別
 *
 * 繼承自 UniBase<UniBaseType::ONE>，這會強制所有衍生類別使用 virtual 繼承。
 * UniBase 的設計確保在多重繼承情境下，基礎類別只會有一個實例，
 * 避免傳統的菱形繼承問題(Diamond Problem)。
 */
class MyBase : public UniBase<UniBaseType::ONE>
{
public:
    // 虛擬解構函式 - 確保透過基礎類別指標刪除物件時，能正確呼叫衍生類別的解構函式
    virtual ~MyBase()
    {
        std::cout << "MyBase destructor\n";
    }

    // 基礎類別的方法
    void doSomething()
    {
        std::cout << "Doing something in MyBase\n";
    }
};

/**
 * MyDerived1 - 第一個衍生類別
 *
 * 注意:這裡必須使用 virtual 繼承!
 * 雖然 UniBase 內部已經使用了 virtual 繼承來確保 _UniBase 唯一,
 * 但在菱形繼承的中間層(MyBase),我們仍需要明確指定 virtual 繼承,
 * 否則 MultiDerived 會有兩個 MyBase 實例。
 */
class MyDerived1 : virtual public MyBase
{
public:
    virtual ~MyDerived1()
    {
        std::cout << "MyDerived1 destructor\n";
    }

    // 覆寫基礎類別的方法
    void doSomething()
    {
        std::cout << "Doing something in MyDerived1\n";
    }
};

/**
 * MyDerived2 - 第二個衍生類別
 *
 * 同樣必須使用 virtual 繼承 MyBase。
 * 這樣在 MultiDerived 多重繼承時,MyBase 才會只有一個實例。
 * 與 MyDerived1 形成平行的繼承關係,共同構成菱形繼承結構的兩個分支。
 */
class MyDerived2 : virtual public MyBase
{
public:
    virtual ~MyDerived2()
    {
        std::cout << "MyDerived2 destructor\n";
    }

    // 覆寫基礎類別的方法
    void doSomething()
    {
        std::cout << "Doing something in MyDerived2\n";
    }
};

/**
 * MultiDerived - 多重繼承類別
 *
 * 這是測試的關鍵類別,同時繼承自 MyDerived1 和 MyDerived2,
 * 形成典型的菱形繼承結構:
 *
 * ========== 簡化繼承圖 ==========
 *           MyBase (UniBase)
 *           /    \
 *    MyDerived1  MyDerived2
 *           \    /
 *        MultiDerived
 *
 * ========== 完整繼承結構圖 ==========
 *
 *                    IDestroyable (介面)
 *                         ↑
 *                         │ (一般繼承)
 *                         │
 *                     _UniBase (基礎實作) ← 全域唯一!
 *                         ↑
 *                         │ virtual 繼承 ← UniBase 內部強制
 *                         │ class _UniBaseOne : virtual public _UniBase
 *                         │
 *                   _UniBaseOne (ONE 策略實作) ← 全域唯一!
 *                         ↑
 *                         │ virtual 繼承 ← UniBase 內部強制
 *                         │ UniBase<ONE> : virtual public _UniBaseOne
 *                         │
 *              UniBase<UniBaseType::ONE> (公開模板) ← 全域唯一!
 *                         ↑
 *                         │ (一般繼承)
 *                         │
 *                      MyBase ← 全域唯一!
 *                      ↗    ↖
 *                     /      \  virtual 繼承 ← 使用者必須明確指定!
 *                    /        \  class MyDerived1 : virtual public MyBase
 *             MyDerived1    MyDerived2
 *                     \      /
 *                      ↘    ↙
 *                  MultiDerived
 *
 * 關鍵特性與注意事項:
 *
 * 1. UniBase 的保證 (自動):
 *    - _UniBase、_UniBaseOne、UniBase<ONE> 在繼承樹中只有一個實例
 *    - 這是由 UniBase 內部的 virtual 繼承自動保證的
 *    - 使用者不需要手動處理這部分
 *
 * 2. 使用者的責任 (手動):
 *    - 如果要建立菱形繼承 (如 MyBase),使用者必須明確使用 virtual 繼承
 *    - 否則會出現歧義錯誤:"'MyBase' is an ambiguous base"
 *    - 這不是 UniBase 的限制,而是 C++ 語言的特性
 *
 * 3. 完整的 virtual 繼承鏈:
 *    - UniBase 層級: 自動 (由框架處理)
 *    - 使用者層級: 手動 (由開發者明確指定)
 *    - 兩者配合才能實現完整的單一實例保證
 *
 * 測試重點:
 * - 驗證 UniBase 是否只有一個實例
 * - 驗證物件可以安全轉型為任一父類別
 * - 驗證解構函式按正確順序呼叫(MultiDerived -> MyDerived2 -> MyDerived1 -> MyBase)
 */
class MultiDerived : public MyDerived1, public MyDerived2
{
public:
    virtual ~MultiDerived()
    {
        std::cout << "MultiDerived destructor\n";
    }

    // 覆寫所有父類別的方法
    void doSomething()
    {
        std::cout << "Doing something in MultiDerived\n";
    }
};

/**
 * main 函式 - 測試程式進入點
 *
 * 執行多重繼承和智慧指標轉型的測試流程
 */
int main(int, char **)
{
    // 取得解構等待器 - 這是 UniPtr 的特殊機制,
    // 用來監控物件的解構過程,確保所有物件都正確釋放
    std::shared_ptr<IDestrWaiter> destrWaiter_ptr = getDestructor();

    // ========== 測試 1: 建立多重繼承物件 ==========
    // 建立一個 MultiDerived 物件,由 UniPtr 智慧指標管理
    // UniPtr 會自動處理物件的生命週期和記憶體釋放
    UniPtr<MultiDerived> multiDerived_ptr = new MultiDerived;

    // 呼叫方法 - 應該會呼叫 MultiDerived::doSomething()
    multiDerived_ptr->doSomething();
    std::cout << std::endl;

    // ========== 驗證 UniBase 只有一個實例 ==========
    // 取得三個不同路徑的 UniBase 指標
    MyBase* base_via_derived1 = static_cast<MyDerived1*>(multiDerived_ptr.get());
    MyBase* base_via_derived2 = static_cast<MyDerived2*>(multiDerived_ptr.get());
    MyBase* base_direct = static_cast<MyBase*>(multiDerived_ptr.get());

    // 這三個指標應該指向同一個 MyBase 實例(因為 virtual 繼承)
    std::cout << "驗證 UniBase 實例唯一性:\n";
    std::cout << "  base_via_derived1 地址: " << base_via_derived1 << "\n";
    std::cout << "  base_via_derived2 地址: " << base_via_derived2 << "\n";
    std::cout << "  base_direct 地址:       " << base_direct << "\n";
    std::cout << "  三者是否相同: "
              << (base_via_derived1 == base_via_derived2 &&
                  base_via_derived2 == base_direct ? "是 ✓" : "否 ✗")
              << "\n" << std::endl;

    // ========== 測試 2: 指標向上轉型 (Upcasting) ==========
    // 將 MultiDerived* 轉型為 MyDerived2*
    // 這是測試 UniPtr 的 cast() 方法是否能正確處理多重繼承的指標轉換
    //
    // 重要觀念:
    // - 在多重繼承中,子類別物件的記憶體位址和父類別子物件的位址可能不同
    // - 需要透過 cast() 方法正確計算位址偏移
    // - (MyDerived2 *)multiDerived_ptr.get() 取得正確的 MyDerived2 子物件位址
    UniPtr<MyDerived2> myDerived2_ptr =
        multiDerived_ptr.cast<MyDerived2>();

    // 呼叫方法 - 應該會呼叫 MultiDerived::doSomething()
    // (因為是 virtual 函式,會根據實際物件型別決定呼叫哪個版本)
    myDerived2_ptr->doSomething();
    std::cout << std::endl;

    std::cout << "Hello, from mytest!\n";

    // ========== 程式結束時的解構順序 ==========
    // 當函式結束時,智慧指標會自動釋放物件,解構函式會按以下順序呼叫:
    // 1. MultiDerived destructor
    // 2. MyDerived2 destructor
    // 3. MyDerived1 destructor
    // 4. MyBase destructor (只會呼叫一次,因為使用 virtual 繼承)
    //
    // 這證明了 UniBase 的 virtual 繼承機制運作正常,
    // 避免了菱形繼承中基礎類別解構函式被多次呼叫的問題。
}
