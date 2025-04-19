/******************************************************************
 * UniBase 必需要被放入 UniObserver 或 UniOwner 中，容
 * 器也有同樣的要求
 * 這裡用 std::unordered_set 示範如何使用
 ******************************************************************/

#include <iostream>
#include <unordered_set>

#include <unibase.hpp>

using namespace CxxlMan3;

// 繼承自 UniBase 的 base 類別
class MyUniBase : public UniBase<UniBaseType::ONE>
{
public:
    virtual ~MyUniBase() {}
    // 延伸類別要做的事
    virtual void doSomething() = 0;
};

// derived class 1
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

// derived class 2
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

// derived class 3
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

// 以 MyUniBase 的位址為 Hash
struct Hash
{
    size_t operator()(const UniOwner<MyUniBase> &uniOwner) const
    {
        return (size_t)((void *)(uniOwner.getUniBase().get()));
    }
};

struct Equal
{
    bool operator()(const UniOwner<MyUniBase> &lhs, const UniOwner<MyUniBase> &rhs) const
    {
        return lhs.getUniBase() != nullptr && rhs.getUniBase() != nullptr && 
               lhs.getUniBase().get() == rhs.getUniBase().get();
    }
};

// 含有一個容器，用來存放各種 UniBase
class MyRoot : public UniBase<UniBaseType::ONE>
{
    std::mutex m_mutex; // UniOwner 的存取必需要的鎖

    // 特別為 UniOwner 打造的容器
    std::unordered_set<UniOwner<MyUniBase>,
                       Hash,
                       Equal>
        m_uniOwnerSet;

public:
    // Constructor
    MyRoot()
    {
    }

    // Setter
    void addUniBase(const std::shared_ptr<MyUniBase> &uniBase_ptr)
    {
        // 設定 UniOwner 所需要的 detachUniBaseFunc
        auto detachUniBaseFunc = [this](UniOwner<MyUniBase> *pSender, void *pChkUniBase)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (pSender->chkUniBase(pChkUniBase))
            {
                m_uniOwnerSet.erase(*pSender); // 這裡採用直接移除的方式
            }
        };

        // 產生一個暫時的 UniOwner
        UniOwner<MyUniBase> tmpUniOwner(this, detachUniBaseFunc);

        if (tmpUniOwner.setUniBase(uniBase_ptr))           // 將 UniBase 設定給 UniOwner
            m_uniOwnerSet.insert(std::move(tmpUniOwner)); // 成功才放入容器中
    }

    // 把放置於容器中的 MyUniBase 叫出來辦事
    void doSomething()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto &uniOwner : m_uniOwnerSet)
            uniOwner.getUniBase()->doSomething();
    }

    // 移除指定的 MyUniBase
    void removeUniBase(const std::shared_ptr<MyUniBase> &uniBase_ptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // 產生一個暫時的 UniOwner
        UniOwner<MyUniBase> tmpUniOwner(this, [this](UniOwner<MyUniBase> *pSender, void *pChkUniBase) {});

        if (tmpUniOwner.setUniBase(uniBase_ptr)) // 將 UniBase 設定給 UniOwner
        {                                         // 成功才執行移除程序
            auto it = m_uniOwnerSet.find(tmpUniOwner);
            if (it != m_uniOwnerSet.end())    // 若有找到
                it = m_uniOwnerSet.erase(it); // 這裡採用找到才移除的方式
        }
    }
};

// 這是一個實用的技巧，可以釋放 std::shared_ptr 也可以觸發銷毀處理器
template <typename UNIBASE>
class KickUniBase : public UniBase<UniBaseType::ONE>
{
public:
    KickUniBase(std::shared_ptr<UNIBASE> &uniBase_ptr)
    {

        UniOwner<UNIBASE> uniOwner(this, [](UniOwner<MyUniBase> *pSender, void *pChkUniBase) {});
        uniOwner.setUniBase(uniBase_ptr);
        uniBase_ptr.reset();
    }
};

int main(int, char **)
{
    {
        std::shared_ptr<IDestrWaiter> destrWaiter_ptr = getDestructor();
        std::cout << "已經取得核心銷毁控制器\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        MyRoot root_ptr;
        std::shared_ptr<MyUniBase> uniBase1_ptr(new MyUniBase1());
        std::shared_ptr<MyUniBase> uniBase2_ptr(new MyUniBase2());
        std::shared_ptr<MyUniBase> uniBase3_ptr(new MyUniBase3());
        std::cout << "已經產生所有要測試的 UniBase 物件\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.addUniBase(uniBase1_ptr);
        root_ptr.addUniBase(uniBase2_ptr);
        root_ptr.addUniBase(uniBase3_ptr);
        std::cout << "已經將三個 MyUniBase 放入 root 中\n";
        std::cout << "接下來看看裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.doSomething();
        std::cout << std::endl;

        uniBase1_ptr.reset();
        std::cout << "已嘗試釋放 uniBase1_ptr\n";
        std::cout << "再看看 root 裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.doSomething();
        std::cout << std::endl;

        root_ptr.removeUniBase(uniBase2_ptr);
        std::cout << "已嘗試移除 uniBase2\n";
        std::cout << "再看看 root 裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.doSomething();
        std::cout << std::endl;

        KickUniBase kickUniBase(uniBase3_ptr);
        std::cout << "已嘗試踼除 uniBase3\n";
        std::cout << "再看看 root 裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.doSomething();
        std::cout << std::endl;
    }
    std::cout << "已經完成所有的演示，程式將結束\n";
    std::cout << "按 <enter> 鍵結束程式\n";
    std::cin.get();
}
