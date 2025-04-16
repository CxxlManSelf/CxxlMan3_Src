

#include <list>
#include <mutex>

#include <threadmgr.hpp>
#include <unibasedestructor.hpp>

namespace CXXL
{

    // 用於等待放棄共用器的待放棄共用清單清空
	cxxlSemaphore g_waitDestructorEmptied;


    // 放棄共用處理器
    class UniBaseDestructor
    {
        // 建立一個獨立的執行緒，專門處理放棄共用
		// ThreadPool m_threadPool{1}; // main() 結束時會先砍掉所有子執行緒，所以用這個不行

        std::mutex m_mutex;

		bool m_isOver = false; // 是否結束執行緒的標識
		// std::optional<std::future<void>> m_future; // 執行緒的回傳值，等待執行緒結束

        // 待放棄共用清單
        std::list<std::shared_ptr<IDestroyable> > m_list;

        // threadProc 的等待通知管制，待放棄共用清單沒有物件的時候
        // 會等待，待放棄共用清單有放入物件的時候會得到通知才運行
        cxxlSemaphore m_gate;

        // 建立一個容器來儲存被放棄共用處理器巡行過的 UniBase，以便巡行後將 fFlag 清除。
        // 要能快速循序取出和剔除
        std::list<IDestroyable *> m_UniBaseSet_fFlag;


        // 放棄共用處理器所用的執行緒
        void cxxlFASTCALL threadProc()
        {
            while (!m_isOver)
            {
                m_gate.wait();
                // g_waitDestructorEmptied.zero();

                while (true)
                {
                    // 用於取得待放棄共用物件
                    std::shared_ptr<IDestroyable> destroyable_ptr;
                    
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);

                        if (!m_list.empty())
                        {
                            destroyable_ptr = m_list.front();
                            m_list.pop_front();
                        }
                        else
                            break;
                    }

                    if (destroyable_ptr->LD_shouldDestroy())
                        destroyable_ptr->LD_destroy();

                    for(auto it:m_UniBaseSet_fFlag)
                        it->LD_clearFlag();

                    m_UniBaseSet_fFlag.clear();
                }
            }
            g_waitDestructorEmptied.release();
        }

    public:

        // Constructor
        UniBaseDestructor()
        {
            // m_future = m_threadPool(std::bind(&UniBaseDestructor::threadProc, this));
            std::thread([this]
                { this->threadProc(); }).detach();
        }

        // Destructor
        ~UniBaseDestructor()
        {
            //stop();
            //g_waitDestructorEmptied.wait();
        }

        // 放入待放棄共用物件
        void cxxlFASTCALL add(const std::shared_ptr<IDestroyable> &destroyable_ptr)
        {
            // g_waitDestructorEmptied.zero();
            std::lock_guard<std::mutex> lock(m_mutex);
            m_list.push_front(destroyable_ptr);
            m_gate.release();
        }

        void cxxlFASTCALL reset_fFlag(const IDestroyable *pDestroyable)
        {
            m_UniBaseSet_fFlag.push_back(const_cast<IDestroyable *>(pDestroyable));
        }

        // 由使用端呼叫結束執行緒
        void cxxlFASTCALL stop()
        {
            m_isOver = true;
            m_gate.release();
        }

    } g_Destructor;

    class CUniBaseDestructor : public IUniBaseDestructor
    {
        virtual void cxxlFASTCALL reset_fFlag(const IDestroyable *pDestroyable) override
        {
            g_Destructor.reset_fFlag(pDestroyable);
        }

        void cxxlFASTCALL checkDestroy(const std::shared_ptr<IDestroyable> &destroyable_ptr)
            override // class IUniBaseDestructor
        {
            g_Destructor.add(destroyable_ptr);
        }

    public:
    } g_UniBaseDestructor;

    IUniBaseDestructor *g_pUniBaseDestructor = &g_UniBaseDestructor;

    class DestrWaiter:public IDestrWaiter
    {
    public:
        virtual ~DestrWaiter()
        {
            g_Destructor.stop();
            g_waitDestructorEmptied.wait();
        }
    };

    std::shared_ptr<IDestrWaiter> cxxlFASTCALL getDestructor()
    {
        static bool f = false;

        if (f)
        {
            // 只能取得一次
            return nullptr;
        }

        f = true;

        return std::shared_ptr<IDestrWaiter>(new DestrWaiter());
    }

}