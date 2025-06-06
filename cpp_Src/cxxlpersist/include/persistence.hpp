/*****************************************************************************
 * persistence.hpp v0.1.0
 *
 * CxxlMan3 採用的永續儲存標準介面約定
 *
 * IPersistable   提供物件的永續儲存功能
 * ChildLink      IPersistable 可含有子 IPersistable，但要用 ChildLink 來連接
 * ISerializable  IPersistable 永續儲存的存取介面
 *
 *
 * Author: CxxlMan
 * Date: 2025 -
 ******************************************************************************/
#ifndef __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3
#define __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3

#include <memory>
#include <list>
#include <mutex>

#include "cxxlpersist.hpp"
#include "uniptr.hpp"
#include "persist_storage.hpp"
#include "maplist.hpp"

namespace CXXL
{

    class IPersistable;

    template <typename T, typename HOLDER>
    class ChildLinkSet;


    // 在此宣告一些 private 類別
    class PersistResourcePrivate
    {
        class _ChildLink;

        // Persistable 的基底類別
        // 負責和儲存體溝通
        class _Persistable : public IPersistChannel
        {
            // Save 序列化狀態:
            //  0: 未序列化
            //  1: mutex 鎖定中
            //  2: 完成序列化 Save            
            //  3: 失敗
            // 不管成功失敗都會 mutex 解除鎖定，並將 m_SerializeState 設為 0
            //
            // Load 序列化狀態:
            //  0: 未序列化
            //  1: mutex 鎖定中
            //  2: Load 檢查成功
            //  3: Load 檢查失敗
            //  4: 完成序列化 Load
            // 不管成功失敗都會 mutex 解除鎖定，並將 m_SerializeState 設為 0            
            int8_t m_SerializeState = 0;

            // try_lock() 失敗回覆 0
            // try_lock() 鎖定成功回覆 1
            // 成功又再 lock 一次回覆 2
            int cxxlFASTCALL lockMutex() override final // class IPersistChannel
            {                
                if(persistable_mutex.try_lock() == false)
                    return 0;

                if(m_SerializeState != 0)
                    return 2;
                else
                    return 1;
            }

            // 呼叫端須管控好，lockMutex() 成功(回覆非 0)才能呼叫
            void cxxlFASTCALL unlockMutex() override final // class IPersistChannel
            {                
                m_SerializeState = 0;
                persistable_mutex.unlock();
            }


            // _ChildLink 集合
            // 含 0 至 多個
            std::list<_ChildLink *> m_childLinks; 

            std::recursive_mutex persistable_mutex;

        public:
            virtual ~_Persistable() {}

            friend class _ChildLink;
            friend class IPersistable;
        };

        // ChildLinkSet 的基底類別
        // 上接一個 _Persistable 父物件        
        // 內含 0 至 多個 _Persistable 子物件        
        class _ChildLink : public IChildLinkChannel
        {
        public:
            _ChildLink(_Persistable *pHost)
            {
                pHost->m_childLinks.push_back(this);
            }
            virtual ~_ChildLink() {}

            
        };

        friend class IPersistable;

        template <typename T, typename HOLDER>
        friend class ChildLinkSet;

    };

    /**
     * 可永久儲存的物件基礎類別
     * 所有需要永續儲存的物件都應該繼承此類別
    **/
    class IPersistable : virtual public UniBase<UniBaseType::ALL>, virtual public PersistResourcePrivate::_Persistable
    {
    protected:
        // 執行 Save 永續儲存
        // 回傳值為 false 表示遇到 null pointer 的情況
        // 只要有一個失敗就應回傳 false
        virtual bool cxxlFASTCALL
        Save(ISerializeSave *pSerialize) = 0;

        // 執行 Load 永續儲存
        // 回傳值為 false 表示失敗，pPersistable 的資料不會改變
        // 只要有一個失敗就應回傳 false
        virtual bool cxxlFASTCALL
        Load(ISerializeLoad *pSerialize) = 0;

        // IPersistable 延伸類別須用此 mutex
        // 是一個 std::recursive_mutex
        using PersistResourcePrivate::_Persistable::persistable_mutex;

    public:
        // Constructor
        IPersistable() = default;

        // Destructor
        virtual ~IPersistable() {}
    };


    // 一個父 IPersistable 和多個子 IPersistable 的連接關係
    // 用於包含 0 至多個子 IPersistable 延伸類別
    // 注意：若包含有 null 將不能被永續儲存
    template <typename T, typename HOLDER = UniOwner<T>>
    class ChildLinkSet : public PersistResourcePrivate::_ChildLink
    {
        // 子物件集合
        MapList<T, HOLDER> m_childSet;

        const H *m_pHost;

    public:
        // Constructor
        template <typename H>
        ChildLinkSet(const UniPtr<T> &child, const H *pHost)
            :_ChildLink(pHost),
             m_pHost(pHost)
        {
        }

        // Setter
        // 若 child_ptr 被標示為結束共用狀態則不會被加入，且回傳 false
        // 同一個物件只能被放入一次
        // 若要當作單一子物件的連接，應先執行 destroy() 後再使用
        bool cxxlFASTCALL set(const UniPtr<T> &child_ptr)
        {
            auto detachUniBaseFunc = [m_pHost](HOLDER *pHolder, void *pChk)
            {
                std::lock_guard<std::mutex> lock(m_pHost->persistable_mutex);
                if (pHolder->chkUniBase(pChk))
                    m_childSet.remove(pHolder->getUniBase());
            }

            // 產生一個暫時的 HOLDER
            HOLDER tmpUniOwner(m_pHost, detachUniBaseFunc);

            if (tmpUniOwner.setUniBase(child_ptr))      // 將 child_ptr 設定給 HOLDER
            {
                m_uniOwnerSet.add(std::move(tmpUniOwner)); // 成功才放入容器中
                return true;
            }
            else
                return false;
        }

        // Getter
        std::list<UniPtr<T> > cxxlFASTCALL get() const
        {
            std::list<UniPtr<T> > resultList;

            for(auto it = m_childSet.begin(); it != m_childSet.end(); ++it)
                resultList.push_back(it->getUniBase());

            return std::move(resultList);
        }

        // 清除所有子物件
        void cxxlFASTCALL destroy()
        {
            m_childSet.clear();
        }
    };

}

#endif // __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3