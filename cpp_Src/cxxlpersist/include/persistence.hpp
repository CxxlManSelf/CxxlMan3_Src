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

    template <UniBaseType T>
    class IPersistable;

    // 在此宣告一些 private 類別
    class PersistResourcePrivate
    {
        class _ChildLink;

        // Persistable 的基底類別
        // 負責和儲存體溝通
        class _Persistable : public IPersistChannel
        {
            std::list<_ChildLink *> m_childLinks; // 子物件集合
        public:
            virtual ~_Persistable() {}

            friend class _ChildLink;
        };

        class _ChildLink : public IChildLinkChannel
        {
            _Persistable *m_pPersistable; // 包裹子

        public:
            _ChildLink(_Persistable *persistable_ptr, _Persistable *pHost)
                : m_pPersistable(persistable_ptr)
            {
                pHost->m_childLinks.push_back(this);
            }
            virtual ~_ChildLink() {}
        };

        template <UniBaseType T>
        friend class IPersistable;
    };

    /**
     * 可永久儲存的物件基礎類別
     * 所有需要永續儲存的物件都應該繼承此類別
     **/
    template <UniBaseType T>
    class IPersistable : virtual public UniBase<T>, virtual public PersistResourcePrivate::_Persistable
    {
    protected:
        // 執行永續儲存
        virtual bool cxxlFASTCALL
        doPersist(ISerialize *pSerialize) = 0;

        // IPersistable 延伸類別須用此 mutex
        std::mutex persistable_mutex;

    public:
        // Constructor
        IPersistable() = default;

        // Destructor
        virtual ~IPersistable() {}
    };

    // IPersistable 和子 IPersistable 的連接關係
    template <typename T, typename HOLDER = UniOwner<T>>
    class ChildLink : public PersistResourcePrivate::_ChildLink
    {
        HOLDER<T> m_child;

        const H *m_pHost;

    public:
        // Constructor
        template <typename H>
        ChildLink(const UniPtr<T> &child, const H *pHost)
            : m_child(
                  pHost,
                  [pHost](HOLDER *pHolder, void *pChk)
                  {
                      std::lock_guard<std::mutex> lock(pHost->persistable_mutex);
                      if (pHolder->chkUniBase(pChk))
                          pHolder->destroy();
                  }),
              m_pHost(pHost)
        {
        }

        // Setter
        // 若成功被加入則回傳 true
        // 若 child_ptr 被標示為結束共用狀態則不會被加入，且回傳 false
        bool cxxlFASTCALL set(const UniPtr<T> &child_ptr)
        {
            return m_child.setUniBase(child_ptr);
        }

        // Getter
        UniPtr<T> cxxlFASTCALL get() const
        {
            return m_child.getUniBase();
        }

        // 結束持有
        void cxxlFASTCALL destroy()
        {
            m_child.destroy();
        }
    };

    // IPersistable 和子 IPersistable 的連接關係
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
            : m_pHost(pHost)
        {
        }

        // Setter
        // 若成功被加入則回傳 true
        // 若 child_ptr 被標示為結束共用狀態則不會被加入，且回傳 false
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
    };

}

#endif // __CXXLPERSIST_PERSISTENCE_HPP_CxxlMan3