

#include <list>
#include <mutex>

#include <threadmgr.hpp>
#include <liferesdestructor.hpp>

namespace CXXL
{

    // 銷毀處理器
    class LifeResDestructor
    {
        // 建立一個獨立的執行緒，專門處理銷毀
        ThreadPool m_threadPool{1};

        std::mutex m_mutex;

		bool m_isOver = false; // 是否結束執行緒的標識

        // 待銷毀清單
        std::list<std::shared_ptr<IDestroyable> > m_list;

        // threadProc 的等待通知管制，待銷毀清單沒有放入的時候
        // 會等待，待銷毀清單有放入的時候會得到通知才運行
        cxxlSemaphore m_gate;

        // 建立一個容器來儲存被銷毀處理器巡行過的 LifeRes，以便巡行後將 fFlag 清除。
        // 要能快速循序取出和剔除
        std::list<IDestroyable *> m_LifeResSet_fFlag;


        // 銷毀處理器所用的執行緒
        void cxxlFASTCALL threadProc()
        {
            while (!m_isOver)
            {
                // 用於取得待銷毀物件
                std::shared_ptr<IDestroyable> destroyable_ptr;

                m_gate.wait();

                while (true)
                {
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

                    for(auto it:m_LifeResSet_fFlag)
                        it->LD_clearFlag();

                    m_LifeResSet_fFlag.clear();
                }
            }
        }

    public:

        // Constructor
        LifeResDestructor()
        {
            m_threadPool(std::bind(&LifeResDestructor::threadProc, this));
        }

		// Destructor
		~LifeResDestructor()
		{
			m_isOver = true;
			m_gate.release();
        }

        // 放入待銷毀物件
        void cxxlFASTCALL add(const std::shared_ptr<IDestroyable> &destroyable_ptr)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_list.push_front(destroyable_ptr);
            m_gate.release();
        }

        void cxxlFASTCALL reset_fFlag(const IDestroyable *pDestroyable)
        {
            m_LifeResSet_fFlag.push_back(const_cast<IDestroyable *>(pDestroyable));
        }

        

    } g_Destructor;

    class CLifeResDestructor : public ILifeResDestructor
    {
        virtual void cxxlFASTCALL reset_fFlag(const IDestroyable *pDestroyable) override
        {
            g_Destructor.reset_fFlag(pDestroyable);
        }

        void cxxlFASTCALL checkDestroy(const std::shared_ptr<IDestroyable> &destroyable_ptr)
            override // class ILifeResDestructor
        {
            g_Destructor.add(destroyable_ptr);
        }

    public:
    } g_LifeResDestructor;

    ILifeResDestructor *g_pLifeResDestructor = &g_LifeResDestructor;

}