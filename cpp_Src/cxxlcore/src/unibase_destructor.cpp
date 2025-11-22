

#include <atomic>
#include <list>
#include <mutex>

#include <semaphore.hpp>
#include <threadmgr.hpp>
#include <unibasedestructor.hpp>

namespace CXXL
{

// 用於等待放棄共用器的待放棄共用清單清空
cxxlSemaphore g_waitDestructorEmptied;

// 儲存被巡行過的 UniBase，以便巡行後將 fFlag 清除。
// 要能快速循序取出和剔除
std::list<IDestroyable *> g_UniBaseSet_fFlag;

/**
 * 放棄共用處理器
 **/
class UniBaseDestructor
{
    std::mutex m_mutex;

    bool m_isOver = false; // 是否結束執行緒的標識

    // 待放棄共用檢測清單
    // std::list<std::shared_ptr<IDestroyable> > m_list;
    std::list<std::function<void()>> m_list;

    // threadProc 的等待通知管制，待放棄共用清單沒有物件的時候
    // 會等待，待放棄共用清單有放入物件的時候會得到通知才運行
    std::condition_variable m_gate;

    // 放棄共用處理器所用的執行緒
    void cxxlFASTCALL threadProc()
    {
        // 用於取得待放棄共用物件
        std::function<void()> checkFunction;
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_gate.wait(lock, [this]()
                            { return !m_list.empty() || m_isOver; });
                if (m_list.empty() && m_isOver)
                    break;

                checkFunction = std::move(m_list.front());
                m_list.pop_front();
            }

            if (checkFunction != nullptr)
                checkFunction();
        }

        g_waitDestructorEmptied.release();
    }

public:
    // Constructor
    UniBaseDestructor()
    {
        std::thread([this]
                    { this->threadProc(); })
            .detach();
    }

    // Destructor
    ~UniBaseDestructor()
    {
    }

    // 放入待放棄共用檢測
    void cxxlFASTCALL add(std::function<void()> checkFunction)
    {
        // g_waitDestructorEmptied.zero();
        std::lock_guard<std::mutex> lock(m_mutex);
        m_list.push_front(checkFunction);
        m_gate.notify_one();
    }

    void cxxlFASTCALL reset_fFlag(const IDestroyable *pDestroyable)
    {
        g_UniBaseSet_fFlag.push_back(const_cast<IDestroyable *>(pDestroyable));
    }

    // 由使用端呼叫結束執行緒
    void cxxlFASTCALL stop()
    {
        m_isOver = true;
        m_gate.notify_all();
    }

} g_Destructor;

class CUniBaseDestructor : public IUniBaseDestructor
{
    virtual void cxxlFASTCALL reset_fFlag(const IDestroyable *pDestroyable) override
    {
        g_Destructor.reset_fFlag(pDestroyable);
    }

    void cxxlFASTCALL justAdd(const std::shared_ptr<IDestroyable> &destroyable_ptr)
        override // class IUniBaseDestructor
    {
        // destroyable_ptr 保存到待放棄清單，避免多執行緒 destroy 干擾
        g_Destructor.add(
            [destroyable_ptr]()
            {
                destroyable_ptr->LD_clearJustAddFlag();
            });
    }

    void cxxlFASTCALL checkDestroy(const std::shared_ptr<IDestroyable> &destroyable_ptr)
        override // class IUniBaseDestructor
    {
        g_Destructor.add(
            [destroyable_ptr]()
            {
                if (destroyable_ptr->LD_shouldDestroy())
                    destroyable_ptr->LD_destroy();

                for (auto it : g_UniBaseSet_fFlag)
                    it->LD_clearFFlag();

                g_UniBaseSet_fFlag.clear();
            });
    }

public:
}g_UniBaseDestructor;

IUniBaseDestructor *g_pUniBaseDestructor = &g_UniBaseDestructor;

class DestrWaiter : public IDestrWaiter
{
public:
    virtual ~DestrWaiter() noexcept
    {
        g_Destructor.stop();
        g_waitDestructorEmptied.wait();
    }
};

std::shared_ptr<IDestrWaiter> cxxlFASTCALL getDestructor()
{
    static std::atomic<bool> f{false};

    bool expected = false;
    if (!f.compare_exchange_strong(expected, true))
    {
        // 只能取得一次
        return nullptr;
    }

    return std::make_shared<DestrWaiter>();
}
}