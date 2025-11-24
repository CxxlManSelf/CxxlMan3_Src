/*
 * UniBase 循環參照測試程式
 *
 * 目的：測試 UniOwner 智慧指標在處理循環參照（Circular Reference）時的能力
 *
 * 測試情境：
 * 1. MyClassA 和 MyClassB 互相持有對方的 UniOwner 參照（形成循環參照）
 * 2. MyRoot 持有 MyClassA 的參照
 * 3. 驗證當外部 UniPtr 釋放後，內部的循環參照能否被正確清理
 *
 * 預期結果：
 * - 即使 MyClassA 和 MyClassB 形成循環參照，當 MyRoot 被銷毀時
 * - 所有物件都應該能夠正確地被銷毀，不會發生記憶體洩漏
 */

#include <iostream>

#include <uniptr.hpp>

using namespace CxxlMan3;

// 前置宣告 MyClassB，因為 MyClassA 需要引用它
class MyClassB;

/*
 * MyClassA 類別
 * - 繼承自 UniBase<UniBaseType::ALL>，支援多個 UniPtr 參照
 * - 持有一個 MyClassB 的 UniOwner 參照
 */
class MyClassA : public UniBase<UniBaseType::ALL> 
{
    // 互斥鎖，用於保護成員變數的執行緒安全
    std::mutex m_mutex;

    // UniOwner 持有 MyClassB 的參照
    // UniOwner 是擁有者指標，當自己被銷毀時會主動銷毀所持有的物件
    UniOwner<MyClassB> m_myB;

public:
    // 建構函式
    // 初始化 m_myB，並設定銷毀回呼函式（destructor callback）
    MyClassA()
        : m_myB(this,  // 指定擁有者為 this
            [this](CxxlMan3::UniOwner<MyClassB> *myB, void *pChk)
            {
                // 銷毀回呼：當 MyClassA 被銷毀時，會呼叫此 lambda
                std::lock_guard<std::mutex> lock(m_mutex);

                // chkUniBase 檢查物件是否仍然有效
                // 如果有效，則主動銷毀它以打破循環參照
                if (myB->chkUniBase(pChk))
                    myB->destroy();
            })
    {}

    // 解構函式
    ~MyClassA()
    {
        std::cout << "MyClassA destructor\n";
    }

    // 設定 MyClassB 的參照
    // 參數 myB_ptr: 指向 MyClassB 物件的 UniPtr
    void cxxlFASTCALL addMyB(const UniPtr<MyClassB> &myB_ptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool ret = m_myB.setUniBase(myB_ptr);
    }
};

/*
 * MyClassB 類別
 * - 繼承自 UniBase<UniBaseType::ALL>，支援多個 UniPtr 參照
 * - 持有一個 MyClassA 的 UniOwner 參照
 * - 與 MyClassA 互相持有對方，形成循環參照
 */
class MyClassB : public UniBase<UniBaseType::ALL>
{
    // 互斥鎖，用於保護成員變數的執行緒安全
    std::mutex m_mutex;

    // UniOwner 持有 MyClassA 的參照
    // 這裡形成了循環參照：MyClassA -> MyClassB -> MyClassA
    UniOwner<MyClassA> m_myA;

public:
    // 建構函式
    // 初始化 m_myA，並設定銷毀回呼函式
    MyClassB()
        : m_myA(this,  // 指定擁有者為 this
            [this](CxxlMan3::UniOwner<MyClassA> *myA, void *pChk)
            {
                // 銷毀回呼：當 MyClassB 被銷毀時，會呼叫此 lambda
                std::lock_guard<std::mutex> lock(m_mutex);

                // 檢查並銷毀持有的 MyClassA 物件
                if (myA->chkUniBase(pChk))
                    myA->destroy();
            })
    {}

    // 解構函式
    ~MyClassB()
    {
        std::cout << "MyClassB destructor\n";
    }

    // 設定 MyClassA 的參照
    // 參數 myA_ptr: 指向 MyClassA 物件的 UniPtr
    void cxxlFASTCALL addMyA(const UniPtr<MyClassA> &myA_ptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool ret = m_myA.setUniBase(myA_ptr);
    }
};

/*
 * MyRoot 類別
 * - 繼承自 UniBase<UniBaseType::ONE>，只支援單一 UniPtr 參照
 * - 作為根節點持有 MyClassA 的參照
 * - 當 MyRoot 被銷毀時，會觸發整個物件鏈的銷毀流程
 */
class MyRoot : public UniBase<UniBaseType::ONE>
{
    // 互斥鎖，用於保護成員變數的執行緒安全
    std::mutex m_mutex;

    // UniOwner 持有 MyClassA 的參照
    // MyRoot 是整個物件鏈的根節點
    UniOwner<MyClassA> m_myA;

public:
    // 建構函式
    // 初始化 m_myA，並設定銷毀回呼函式
    MyRoot()
        : m_myA(this,  // 指定擁有者為 this
            [this](CxxlMan3::UniOwner<MyClassA> *myA, void *pChk)
            {
                // 銷毀回呼：當 MyRoot 被銷毀時，會呼叫此 lambda
                std::lock_guard<std::mutex> lock(m_mutex);

                // 檢查並銷毀持有的 MyClassA 物件
                // 這將觸發連鎖反應，解開 MyClassA 和 MyClassB 的循環參照
                if (myA->chkUniBase(pChk))
                    myA->destroy();
            })
    {}

    // 解構函式
    ~MyRoot()
    {
        std::cout << "MyRoot destructor\n";
    }

    // 設定 MyClassA 的參照
    // 參數 myA_ptr: 指向 MyClassA 物件的 UniPtr
    void cxxlFASTCALL addMyA(const UniPtr<MyClassA> &myA_ptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool ret = m_myA.setUniBase(myA_ptr);
    }
};

/*
 * 主程式：循環參照測試流程
 *
 * 測試步驟：
 * 1. 取得核心銷毀控制器（⚠️ 只能成功取得一次，必須在程式一開始執行）
 * 2. 建立 MyClassA 和 MyClassB 物件
 * 3. 讓兩者互相引用（形成循環參照）
 * 4. 建立 MyRoot 物件並持有 MyClassA
 * 5. 釋放 myA_ptr 和 myB_ptr（但物件仍被 UniOwner 持有）
 * 6. 釋放 myRoot_ptr，觸發整個物件鏈的銷毀
 *
 * 關鍵觀察點：
 * - 當 myRoot_ptr.reset() 時，應該看到所有解構函式被正確呼叫
 * - 銷毀順序應該是：MyRoot -> MyClassA -> MyClassB（或相反）
 * - 不應該有任何記憶體洩漏
 *
 * ⚠️ 重要：getDestructor() 的使用限制
 * - getDestructor() 在整個程式生命週期中只能成功呼叫一次
 * - 第二次呼叫會失敗，因此必須在 main() 函式一開始就取得
 * - destrWaiter_ptr 必須在所有 UniBase 物件的生命週期內都保持有效
 */
int main(int, char**)
{
    {
        // 步驟 1: 取得核心銷毀控制器
        // getDestructor() 回傳一個控制器，用於管理 UniBase 物件的生命週期
        //
        // ⚠️ 重要限制：getDestructor() 在整個程式執行期間只能成功呼叫一次
        // - 如果重複呼叫，第二次會失敗或回傳 nullptr
        // - 因此必須在程式一開始就呼叫，並在整個生命週期內持有這個控制器
        // - 只有當 destrWaiter_ptr 離開作用域後，所有 UniBase 物件才會開始銷毀程序
        std::shared_ptr<IDestrWaiter> destrWaiter_ptr = getDestructor();
        std::cout << "已經取得核心銷毁控制器\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();

        // 步驟 2: 建立兩個互相參照的物件
        // myA_ptr 和 myB_ptr 是 UniPtr 智慧指標，指向各自的物件
        UniPtr<MyClassA> myA_ptr(new MyClassA);
        UniPtr<MyClassB> myB_ptr = new MyClassB;
        std::cout << "已經建立了 myA_ptr 和 myB_ptr\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();

        // 步驟 3: 建立循環參照
        // myA_ptr 內部的 m_myB 持有 myB_ptr
        // myB_ptr 內部的 m_myA 持有 myA_ptr
        // 形成循環：myA_ptr <-> myB_ptr
        myA_ptr->addMyB(myB_ptr);
        myB_ptr->addMyA(myA_ptr);
        std::cout << "已經讓 myA_ptr 和 myB_ptr 互相引用\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();

        // 步驟 4: 建立根節點
        // MyRoot 作為整個物件鏈的擁有者
        UniPtr<MyRoot> myRoot_ptr = new MyRoot;
        std::cout << "已經建立了 myRoot_ptr\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();

        // 步驟 5: 讓根節點持有 MyClassA
        // 現在的參照結構：myRoot_ptr -> myA_ptr <-> myB_ptr
        myRoot_ptr->addMyA(myA_ptr);
        std::cout << "已經讓 myRoot_ptr 持有 myA_ptr\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();

        // 步驟 6: 釋放外部的 UniPtr
        // 重要：這裡只是釋放 UniPtr 本身，但物件不會被銷毀
        // 因為 MyClassA 仍被 MyRoot 的 m_myA (UniOwner) 持有
        // MyClassB 仍被 MyClassA 的 m_myB (UniOwner) 持有
        myA_ptr.reset();
        myB_ptr.reset();
        std::cout << "已經釋放了 myA_ptr 和 myB_ptr\n";
        std::cout << "但彼此仍互相引用\n";

        // 步驟 7: 釋放根節點，觸發連鎖銷毀
        // 當 myRoot_ptr.reset() 時：
        // 1. MyRoot 的解構函式被呼叫
        // 2. MyRoot 的銷毀回呼被觸發，呼叫 m_myA.destroy()
        // 3. MyClassA 的解構函式被呼叫
        // 4. MyClassA 的銷毀回呼被觸發，呼叫 m_myB.destroy()
        // 5. MyClassB 的解構函式被呼叫
        // 6. 循環參照被成功打破，沒有記憶體洩漏
        std::cout << "按 <enter> 鍵後將會釋放 myRoot_ptr\n"; std::cin.get();
        myRoot_ptr.reset();
    }

    // 測試完成
    std::cout << "已經演釋完整個過程\n";
    std::cout << "按 <enter> 鍵結束程式\n"; std::cin.get();
}
