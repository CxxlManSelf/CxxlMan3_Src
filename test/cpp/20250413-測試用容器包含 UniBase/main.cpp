/******************************************************************
 * 測試目的：展示如何將 UniBase 物件放入標準 STL 容器中
 *
 * 核心概念：
 * 1. UniBase 物件必須透過 UniObserver 或 UniOwner 來管理
 * 2. 當 UniBase 需要放入容器時，容器中存放的應該是 UniOwner
 * 3. 本範例使用 std::unordered_set<UniOwner<T>> 來示範
 *
 * 關鍵技術點：
 * - UniOwner 作為容器元素的管理者
 * - 自訂 Hash 和 Equal 函數來支援 unordered_set
 * - detachUniBaseFunc 回呼函數處理物件生命週期
 * - 互斥鎖(mutex)保護容器的執行緒安全
 *
 * 測試情境：
 * 1. 將三個 UniBase 衍生物件加入容器
 * 2. 透過 reset() 展示 UniPtr 不參與生命週期管理
 * 3. 手動從容器中移除物件（觸發 ONE 模式銷毀）
 * 4. 使用 KickUniBase 技巧強制觸發放棄共享
 ******************************************************************/

#include <iostream>
#include <unordered_set>
#include <mutex>

#include <uniptr.hpp>

using namespace CxxlMan3;

/*******************************************************************
 * MyUniBase - 測試用的基礎類別
 *
 * 說明：
 * - 繼承自 UniBase<UniBaseType::ONE>，採用 ONE 模式
 * - ONE 模式的特性：可以被多個 UniOwner 擁有，但只要有任何
 *   一個 UniOwner 放棄（detach），UniBase 就會立即被銷毀
 * - 這是一種「脆弱共享」機制，適合用於必須嚴格管理生命週期的場景
 * - 定義純虛擬函數 doSomething()，強制衍生類別實作
 * - 作為多型基底類別，供 MyUniBase1/2/3 繼承
 *******************************************************************/
class MyUniBase : public UniBase<UniBaseType::ONE>
{
public:
    virtual ~MyUniBase() {}

    // 抽象介面：要求衍生類別實作特定行為
    virtual void doSomething() = 0;
};

/*******************************************************************
 * MyUniBase1 - 第一個測試衍生類別
 *
 * 說明：
 * - 實作 doSomething() 以顯示自己的身份
 * - 解構函數會印出訊息，方便追蹤物件的生命週期
 *******************************************************************/
class MyUniBase1 : public MyUniBase
{
    virtual void doSomething() override
    {
        std::cout << "MyUniBase1 doSomething" << std::endl;
    }

public:
    virtual ~MyUniBase1()
    {
        std::cout << "MyUniBase1 destructor" << std::endl;
    }
};

/*******************************************************************
 * MyUniBase2 - 第二個測試衍生類別
 *******************************************************************/
class MyUniBase2 : public MyUniBase
{
    virtual void doSomething() override
    {
        std::cout << "MyUniBase2 doSomething" << std::endl;
    }

public:
    virtual ~MyUniBase2()
    {
        std::cout << "MyUniBase2 destructor" << std::endl;
    }
};

/*******************************************************************
 * MyUniBase3 - 第三個測試衍生類別
 *******************************************************************/
class MyUniBase3 : public MyUniBase
{
    virtual void doSomething() override
    {
        std::cout << "MyUniBase3 doSomething" << std::endl;
    }

public:
    virtual ~MyUniBase3()
    {
        std::cout << "MyUniBase3 destructor" << std::endl;
    }
};

/*******************************************************************
 * Hash - 自訂雜湊函數物件
 *
 * 用途：
 * - 讓 UniOwner<MyUniBase> 可以作為 unordered_set 的鍵值
 * - std::unordered_set 需要 Hash 函數來計算元素的雜湊值
 *
 * 實作策略：
 * - 使用 MyUniBase 物件的記憶體位址作為雜湊值
 * - 透過 uniOwner.getUniBase().get() 取得原始指標
 * - 使用 reinterpret_cast 將指標轉換為 size_t 整數
 *
 * 注意：
 * - 此方法確保相同的 UniBase 物件始終產生相同的雜湊值
 * - 不同物件的位址必定不同，因此不會有雜湊衝突
 *******************************************************************/
struct Hash
{
    size_t operator()(const UniOwner<MyUniBase> &uniOwner) const
    {
        // 取得 MyUniBase 物件的原始指標位址，轉換為雜湊值
        return reinterpret_cast<size_t>(uniOwner.getUniBase().get());
    }
};

/*******************************************************************
 * Equal - 自訂相等比較函數物件
 *
 * 用途：
 * - 讓 UniOwner<MyUniBase> 可以進行相等性比較
 * - std::unordered_set 需要此函數來判斷兩個元素是否相同
 *
 * 實作策略：
 * - 比較兩個 UniOwner 所持有的 MyUniBase 物件指標是否相同
 * - 必須確保兩個指標都不是 nullptr 才進行比較
 *
 * 邏輯：
 * - 取得左側和右側 UniOwner 的 UniBase 原始指標
 * - 只有當兩者都非空且指向同一個物件時才返回 true
 *******************************************************************/
struct Equal
{
    bool operator()(const UniOwner<MyUniBase> &lhs, const UniOwner<MyUniBase> &rhs) const
    {
        // 取得兩個 UniOwner 內部持有的原始指標
        MyUniBase *p1 = lhs.getUniBase().get();
        MyUniBase *p2 = rhs.getUniBase().get();

        // 兩者都非空且指向同一物件時視為相等
        return p1 != nullptr && p2 != nullptr && p1 == p2;
    }
};

/*******************************************************************
 * MyRoot - 容器管理類別
 *
 * 設計目的：
 * - 展示如何使用容器來管理多個 UniBase 物件
 * - 本身也繼承自 UniBase，因此也能被其他 UniOwner 管理
 *
 * 核心機制：
 * - 使用 std::unordered_set 存放 UniOwner<MyUniBase>
 * - 透過自訂的 Hash 和 Equal 函數支援 UniOwner 作為鍵值
 * - 使用 mutex 確保多執行緒環境下的安全存取
 *
 * 重要觀念：
 * - 容器中存放的是 UniOwner，而非 UniBase 本身
 * - UniOwner 負責管理 UniBase 的生命週期
 * - 當 UniBase 被銷毀時，會自動觸發 detachUniBaseFunc 回呼
 *******************************************************************/
class MyRoot : public UniBase<UniBaseType::ONE>
{
    // 互斥鎖：保護容器的執行緒安全存取
    // UniOwner 的任何存取操作都應該在鎖的保護下進行
    std::mutex m_mutex;

    // UniOwner 專用容器：
    // - 第一個模板參數：元素類型 UniOwner<MyUniBase>
    // - 第二個模板參數：Hash 函數物件，用於計算雜湊值
    // - 第三個模板參數：Equal 函數物件，用於相等性比較
    std::unordered_set<UniOwner<MyUniBase>,
                       Hash,
                       Equal>
        m_uniOwnerSet;

public:
    // 建構函數
    MyRoot()
    {
        // 空實作即可，成員變數會自動初始化
    }

    /***************************************************************
     * addUniBase - 將 UniBase 物件加入容器
     *
     * 參數：
     * - uniBase_ptr: 要加入的 UniBase 智慧指標
     *
     * 執行流程：
     * 1. 定義 detachUniBaseFunc 回呼函數
     * 2. 建立臨時的 UniOwner 物件
     * 3. 將 UniBase 設定給 UniOwner
     * 4. 若成功，將 UniOwner 移動到容器中
     *
     * detachUniBaseFunc 的作用：
     * - 當 UniBase 物件被銷毀或放棄共享時會被呼叫
     * - 此函數負責從容器中移除對應的 UniOwner
     * - 使用 lock_guard 確保執行緒安全
     * - 使用 chkUniBase() 驗證物件是否匹配後再移除
     *
     * 重要細節：
     * - tmpUniOwner 使用 std::move 轉移所有權到容器中
     * - 只有 setUniBase 成功時才會插入容器
     * - lambda 捕獲 [this] 以存取成員變數
     ***************************************************************/
    void addUniBase(const UniPtr<MyUniBase> &uniBase_ptr)
    {
        // 定義分離回呼函數：當 UniBase 被銷毀時自動從容器移除
        auto detachUniBaseFunc = [this](UniOwner<MyUniBase> *pSender, void *pChkUniBase)
        {
            // 鎖定互斥鎖，保護容器的存取
            std::lock_guard<std::mutex> lock(m_mutex);

            // 確認這個 UniOwner 確實持有要移除的 UniBase
            if (pSender->chkUniBase(pChkUniBase))
            {
                // 從容器中移除此 UniOwner
                // 這裡採用直接移除的策略，依賴 pSender 的解引用
                m_uniOwnerSet.erase(*pSender);
            }
        };

        // 建立臨時的 UniOwner，指定擁有者(this)和分離回呼函數
        UniOwner<MyUniBase> tmpUniOwner(this, detachUniBaseFunc);

        // 嘗試將 UniBase 設定給 UniOwner
        if (tmpUniOwner.setUniBase(uniBase_ptr))
        {
            // 設定成功，將 UniOwner 移動到容器中
            // 使用 std::move 避免複製，提升效能
            m_uniOwnerSet.insert(std::move(tmpUniOwner));
        }
    }

    /***************************************************************
     * doSomething - 遍歷容器中所有 UniBase 並呼叫其方法
     *
     * 說明：
     * - 展示如何安全地存取容器中的所有 UniBase 物件
     * - 透過 UniOwner.getUniBase() 取得 UniBase 的智慧指標
     * - 再透過該指標呼叫 doSomething() 方法
     *
     * 執行緒安全：
     * - 使用 lock_guard 確保整個遍歷過程的原子性
     * - 防止其他執行緒在遍歷時修改容器
     ***************************************************************/
    void doSomething()
    {
        // 鎖定互斥鎖，保護容器的遍歷過程
        std::lock_guard<std::mutex> lock(m_mutex);

        // 遍歷容器中的每個 UniOwner
        for (auto &uniOwner : m_uniOwnerSet)
        {
            // 取得 UniBase 智慧指標並呼叫其方法
            uniOwner.getUniBase()->doSomething();
        }
    }

    /***************************************************************
     * removeUniBase - 手動從容器中移除指定的 UniBase
     *
     * 參數：
     * - uniBase_ptr: 要移除的 UniBase 智慧指標
     *
     * 執行流程：
     * 1. 建立臨時的 UniOwner（使用空的 detach 回呼）
     * 2. 將要尋找的 UniBase 設定給臨時 UniOwner
     * 3. 使用 find() 在容器中查找匹配的 UniOwner
     * 4. 若找到則移除
     *
     * 與 addUniBase 的差異：
     * - addUniBase: 當 UniBase 被銷毀時「自動」移除
     * - removeUniBase: 「手動」從容器中移除，但不銷毀 UniBase
     *
     * 技巧說明：
     * - 此處 detachUniBaseFunc 設為空 lambda，因為這是臨時用途
     * - tmpUniOwner 僅用於查找，不會長期持有 UniBase
     * - 利用自訂的 Equal 函數進行 UniOwner 的比對
     ***************************************************************/
    void removeUniBase(const UniPtr<MyUniBase> &uniBase_ptr)
    {
        // 鎖定互斥鎖，保護容器的修改
        std::lock_guard<std::mutex> lock(m_mutex);

        // 建立臨時的 UniOwner，使用空的 detach 回呼（因為只是暫時使用）
        UniOwner<MyUniBase> tmpUniOwner(this, [](UniOwner<MyUniBase> *pSender, void *pChkUniBase) {});

        // 嘗試將 UniBase 設定給臨時 UniOwner
        if (tmpUniOwner.setUniBase(uniBase_ptr))
        {
            // 設定成功，開始在容器中查找匹配的 UniOwner
            auto it = m_uniOwnerSet.find(tmpUniOwner);

            // 若找到匹配的元素
            if (it != m_uniOwnerSet.end())
            {
                // 從容器中移除該 UniOwner
                // 這裡採用「先找後刪」的策略，確保元素存在才移除
                it = m_uniOwnerSet.erase(it);
            }
        }
    }
};

/*******************************************************************
 * KickUniBase - 強制觸發 UniBase 放棄共享的實用工具類別
 *
 * 設計目的：
 * - 提供一個簡單的方法來「踢除」UniBase，觸發所有相關的
 *   detachUniBaseFunc 回呼
 * - 比單純呼叫 reset() 更明確地表達「主動放棄共享」的意圖
 *
 * 工作原理：
 * 1. 建立臨時的 UniOwner 來接管 UniBase
 * 2. 將 UniBase 設定給這個臨時 UniOwner
 * 3. 重置原始的 UniPtr（釋放外部持有的參考）
 * 4. 當建構函數結束時，臨時 UniOwner 被銷毀
 * 5. UniOwner 的銷毀會觸發 UniBase 的 detach 流程
 * 6. 所有持有該 UniBase 的 UniOwner 都會收到通知
 *
 * 使用情境：
 * - 當你想確保 UniBase 從所有容器中移除時
 * - 測試 detachUniBaseFunc 的觸發機制
 * - 明確表達「放棄共享」的語意
 *
 * 與 reset() 的差異：
 * - reset(): 單純釋放智慧指標，可能不會立即觸發 detach
 * - KickUniBase: 保證會觸發完整的 detach 流程
 *
 * 模板參數：
 * - UNIBASE: 必須是繼承自 UniBase 的類型
 *******************************************************************/
template <typename UNIBASE>
class KickUniBase : public UniBase<UniBaseType::ONE>
{
public:
    // 建構函數：接受 UniPtr 的參考，執行踢除動作
    KickUniBase(UniPtr<UNIBASE> &uniBase_ptr)
    {
        // 建立臨時的 UniOwner，使用空的 detach 回呼
        // 這個 UniOwner 只存在於建構函數的生命週期內
        UniOwner<UNIBASE> uniOwner(this, [](UniOwner<MyUniBase> *pSender, void *pChkUniBase) {});

        // 將 UniBase 設定給臨時 UniOwner
        // 此時 UniBase 會註冊這個 UniOwner 為新的擁有者
        bool bRet = uniOwner.setUniBase(uniBase_ptr);

        // 重置原始的 UniPtr，釋放外部對 UniBase 的持有
        uniBase_ptr.reset();

        // 當建構函數結束時：
        // - uniOwner 區域變數被銷毀
        // - UniOwner 的解構函數會觸發 UniBase 的 detach
        // - 所有相關容器的 detachUniBaseFunc 會被呼叫
        // - UniBase 從所有容器中被移除
    }
};

/*******************************************************************
 * main - 測試程式主函數
 *
 * 測試流程說明：
 * 本程式展示了四種不同的 UniBase 生命週期管理方式：
 *
 * 階段 1：初始化
 * - 取得核心銷毀控制器 (destrWaiter_ptr)
 * - 建立容器管理者 (root_ptr)
 * - 建立三個測試物件 (uniBase1/2/3)
 * - 將三個物件加入容器
 *
 * 階段 2：測試 UniPtr 的特性 (reset)
 * - 呼叫 uniBase1_ptr.reset() 釋放 UniPtr
 * - 展示 UniPtr 不參與生命週期管理
 * - uniBase1 仍然存在於容器中（由 UniOwner 管理）
 *
 * 階段 3：測試手動移除機制 (removeUniBase)
 * - 呼叫 root_ptr.removeUniBase(uniBase2_ptr)
 * - 從容器中移除 UniOwner
 * - 觸發 ONE 模式：UniOwner 放棄，uniBase2 被銷毀
 *
 * 階段 4：測試強制踢除機制 (KickUniBase)
 * - 使用 KickUniBase 工具類別踢除 uniBase3
 * - 保證觸發完整的 detach 流程
 * - 從所有相關容器中移除
 *
 * 階段 5：清理
 * - 離開作用域，root_ptr 被銷毀
 * - 容器中剩餘的 UniOwner 也會被清理
 *
 * 預期觀察結果：
 * - 第一次 doSomething(): 顯示 3 個物件 (Base1, Base2, Base3)
 * - 第二次 doSomething(): 顯示 3 個物件 (Base1, Base2, Base3) - reset() 無效果
 * - 第三次 doSomething(): 顯示 2 個物件 (Base1, Base3) - Base2 已移除並銷毀
 * - 第四次 doSomething(): 顯示 1 個物件 (Base1) - Base3 已被踢除並銷毀
 *******************************************************************/
int main(int, char **)
{
    // 使用額外的作用域來控制物件的生命週期
    {
        // ===== 階段 1：初始化 =====

        // 取得核心銷毀控制器
        // 這是 UniBase 系統的核心組件，負責協調所有 UniBase 的銷毀
        std::shared_ptr<IDestrWaiter> destrWaiter_ptr = getDestructor();
        std::cout << "已經取得核心銷毁控制器\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        // 建立容器管理者
        MyRoot root_ptr;

        // 建立三個測試用的 UniBase 物件
        UniPtr<MyUniBase> uniBase1_ptr(new MyUniBase1());
        UniPtr<MyUniBase> uniBase2_ptr(new MyUniBase2());
        UniPtr<MyUniBase> uniBase3_ptr(new MyUniBase3());
        std::cout << "已經產生所有要測試的 UniBase 物件\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        // 將三個物件加入容器
        // 此時每個 UniBase 都會被 UniOwner 管理
        root_ptr.addUniBase(uniBase1_ptr);
        root_ptr.addUniBase(uniBase2_ptr);
        root_ptr.addUniBase(uniBase3_ptr);
        std::cout << "已經將三個 MyUniBase 放入 root 中\n";
        std::cout << "接下來看看裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        // 第一次測試：應該顯示所有三個物件
        root_ptr.doSomething();
        std::cout << std::endl;

        // ===== 階段 2：測試 UniPtr 特性 (reset) =====

        // 釋放 uniBase1 的 UniPtr
        // 重要：UniPtr 不參與生命週期管理，所以這個操作不會影響容器
        // uniBase1 仍然由容器中的 UniOwner 管理，會繼續存在
        uniBase1_ptr.reset();
        std::cout << "已釋放 uniBase1_ptr (UniPtr)\n";
        std::cout << "但 uniBase1 仍在容器中，因為 UniPtr 不管理生命週期\n";
        std::cout << "再看看 root 裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        // 第二次測試：應該還是顯示所有三個物件
        root_ptr.doSomething();
        std::cout << std::endl;

        // ===== 階段 3：測試手動移除 (removeUniBase) =====

        // 手動從容器中移除 uniBase2
        // 這會移除容器中的 UniOwner，觸發 ONE 模式
        // 因為 UniOwner 放棄，uniBase2 會被銷毀（應該會看到解構函數訊息）
        root_ptr.removeUniBase(uniBase2_ptr);
        std::cout << "已從容器移除 uniBase2（應該已被銷毀）\n";
        std::cout << "再看看 root 裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        // 第三次測試：應該只顯示 Base1 和 Base3
        root_ptr.doSomething();
        std::cout << std::endl;

        // ===== 階段 4：測試強制踢除 (KickUniBase) =====

        // 使用 KickUniBase 強制觸發 detach 流程
        // 建立臨時 UniOwner → 容器中的 UniOwner 收到 detach → 從容器移除
        // 臨時 UniOwner 銷毀 → 觸發 ONE 模式 → uniBase3 被銷毀
        KickUniBase kickUniBase(uniBase3_ptr);
        std::cout << "已踢除 uniBase3（應該已被銷毀）\n";
        std::cout << "再看看 root 裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        // 第四次測試：應該只顯示 Base1
        root_ptr.doSomething();
        std::cout << std::endl;

        // ===== 階段 5：清理 =====

        std::cout << "接下來 root 將被銷毀\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        // 離開作用域時：
        // - root_ptr 被銷毀
        // - 容器中的所有 UniOwner 被銷毀
        // - 剩餘的 UniBase 物件（只剩 uniBase1）也會被清理
        // - 應該會看到 MyUniBase1 destructor 訊息
    }

    // 所有物件都已被銷毀
    std::cout << "\n已經完成所有的演示，程式將結束\n";
    std::cout << "按 <enter> 鍵結束程式\n";
    std::cin.get();
}
