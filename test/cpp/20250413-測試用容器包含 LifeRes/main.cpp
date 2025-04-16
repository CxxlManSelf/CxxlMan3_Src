/******************************************************************
 * LifeRes 必需要被放入 LifeObserver 或 LifeOwner 中，容
 * 器也有同樣的要求
 * 這裡示範 std::unordered_set 這個容器如何使用
 ******************************************************************/

#include <iostream>
#include <unordered_set>

#include <liferes.hpp>

using namespace CxxlMan3;

// 繼承自 LifeRes 的 base 類別
class MyLifeRes : public LifeRes<LifeResType::ONE>
{
public:
    virtual ~MyLifeRes() {}
    // 延伸類別要做的事
    virtual void doSomething() = 0;
};

// derived class 1
class MyLifeRes1 : public MyLifeRes
{
    virtual void doSomething() override
    {
        std::cout << "MyLifeRes1 doSomething" << std::endl;
    }

public:
    virtual ~MyLifeRes1()
    {
        std::cout << "MyLifeRes1 destructor" << std::endl;
    }
};

// derived class 2
class MyLifeRes2 : public MyLifeRes
{
    virtual void doSomething() override
    {
        std::cout << "MyLifeRes2 doSomething" << std::endl;
    }

public:
    virtual ~MyLifeRes2()
    {
        std::cout << "MyLifeRes2 destructor" << std::endl;
    }
};

// derived class 3
class MyLifeRes3 : public MyLifeRes
{
    virtual void doSomething() override
    {
        std::cout << "MyLifeRes3 doSomething" << std::endl;
    }

public:
    virtual ~MyLifeRes3()
    {
        std::cout << "MyLifeRes3 destructor" << std::endl;
    }
};

// 以 MyLifeRes 的位址為 Hash
struct Hash
{
    size_t operator()(const LifeOwner<MyLifeRes> &lifeOwner) const
    {
        return (size_t)((void *)(lifeOwner.getLifeRes().get()));
    }
};

struct Equal
{
    bool operator()(const LifeOwner<MyLifeRes> &lhs, const LifeOwner<MyLifeRes> &rhs) const
    {
        return lhs.getLifeRes() != nullptr && rhs.getLifeRes() != nullptr && lhs.getLifeRes().get() == rhs.getLifeRes().get();
    }
};

// 含有一個容器，用來存放各種 LifeRes
class MyRoot : public LifeRes<LifeResType::ONE>
{
    std::mutex m_mutex; // LifeOwner 的存取必需要的鎖

    // 特別為 LifeOwner 打造的容器
    std::unordered_set<LifeOwner<MyLifeRes>,
                       Hash,
                       Equal>
        m_lifeOwnerSet;

public:
    // Constructor
    MyRoot()
    {
    }

    // Setter
    void addLifeOwner(const std::shared_ptr<MyLifeRes> &lifeRes_ptr)
    {
        // LifeOwner 和 LifeObserver 的存取必需要的鎖
        std::lock_guard<std::mutex> lock(m_mutex);

        // 設定 LifeOwner 所需要的 detachLifeResFunc
        auto detachLifeResFunc = [this](LifeOwner<MyLifeRes> *pSender, void *pChkLifeRes)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (pSender->chkLifeRes(pChkLifeRes))
            {
                m_lifeOwnerSet.erase(*pSender); // 這裡採用直接移除的方式
            }
        };

        // 產生一個暫時的 LifeOwner
        LifeOwner<MyLifeRes> tmpLifeOwner(this, detachLifeResFunc);

        if (tmpLifeOwner.setLifeRes(lifeRes_ptr))           // 將 LifeRes 設定給 LifeOwner
            m_lifeOwnerSet.insert(std::move(tmpLifeOwner)); // 成功才放入容器中
    }

    // 把放置於容器中的 MyLifeRes 叫出來辦事
    void doSomething()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto &lifeOwner : m_lifeOwnerSet)
            lifeOwner.getLifeRes()->doSomething();
    }

    // 移除指定的 MyLifeRes
    void removeLifeOwner(const std::shared_ptr<MyLifeRes> &lifeRes_ptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // 產生一個暫時的 LifeOwner
        LifeOwner<MyLifeRes> tmpLifeOwner(this, [this](LifeOwner<MyLifeRes> *pSender, void *pChkLifeRes) {});

        if (tmpLifeOwner.setLifeRes(lifeRes_ptr)) // 將 LifeRes 設定給 LifeOwner
        {                                         // 成功才執行移除程序
            auto it = m_lifeOwnerSet.find(tmpLifeOwner);
            if (it != m_lifeOwnerSet.end())    // 若有找到
                it = m_lifeOwnerSet.erase(it); // 這裡採用找到才移除的方式
        }
    }
};

// 這是一個實用的技巧，可以釋放 std::shared_ptr 也可以觸發銷毀處理器
template <typename LIFERES>
class KickLifeRes : public LifeRes<LifeResType::ONE>
{
public:
    KickLifeRes(std::shared_ptr<LIFERES> &lifeRes_ptr)
    {

        LifeOwner<LIFERES> lifeOwner(this, [](LifeOwner<MyLifeRes> *pSender, void *pChkLifeRes) {});
        lifeOwner.setLifeRes(lifeRes_ptr);
        lifeRes_ptr.reset();
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
        std::shared_ptr<MyLifeRes> lifeRes1_ptr(new MyLifeRes1());
        std::shared_ptr<MyLifeRes> lifeRes2_ptr(new MyLifeRes2());
        std::shared_ptr<MyLifeRes> lifeRes3_ptr(new MyLifeRes3());
        std::cout << "已經產生所有要測試的 LifeRes 物件\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.addLifeOwner(lifeRes1_ptr);
        root_ptr.addLifeOwner(lifeRes2_ptr);
        root_ptr.addLifeOwner(lifeRes3_ptr);
        std::cout << "已經將三個 MyLifeRes 放入 root 中\n";
        std::cout << "接下來看看裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.doSomething();
        std::cout << std::endl;

        lifeRes1_ptr.reset();
        std::cout << "已嘗試釋放 lifeRes1_ptr\n";
        std::cout << "再看看 root 裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.doSomething();
        std::cout << std::endl;

        root_ptr.removeLifeOwner(lifeRes2_ptr);
        std::cout << "已嘗試移除 lifeRes2\n";
        std::cout << "再看看 root 裡面的內容\n";
        std::cout << "按 <enter> 鍵繼續\n";
        std::cin.get();

        root_ptr.doSomething();
        std::cout << std::endl;

        KickLifeRes kickLifeRes(lifeRes3_ptr);
        std::cout << "已嘗試踼除 lifeRes3\n";
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
