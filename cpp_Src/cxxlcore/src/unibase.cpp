
#include "unibase.hpp"

namespace CXXL
{

    bool cxxlFASTCALL UniResourcePrivate::_UniBase::checkNoHost()
    {
        // 已找過
        if (fFlag)
            return true;

        fFlag = true; // 設為已找過的狀態
        g_pUniBaseDestructor->reset_fFlag(this);

        std::lock_guard<std::mutex> lock(m_UniBaseMutex);

        if (rFlag == false) // 本身是 rootUniBase，即未放入過 _Holder 為 false
            return false;

        if (cFlag || m_holderSet.size() == 0)
            return false; // 回報我也要處理待結束共用確認

        bool fNoRootUniBase = true; // 有找到 rootUniBase 或還有要放棄持有的處理回覆 false
        for (auto &it : m_holderSet)
        {
            auto pHost = it->m_pHost;

            fNoRootUniBase = ((RmConst<decltype(pHost)>::type)pHost)->checkNoHost();
            if (!fNoRootUniBase)
                break;
        }
        return fNoRootUniBase;
    }

    bool cxxlFASTCALL UniResourcePrivate::_UniBase::LD_shouldDestroy()
    {
        std::lock_guard<std::mutex> lock(m_UniBaseMutex);

        if (m_isDestroy)
            return ldFlag = true; // 已標識須放棄共用

        // 無持有者了
        if (m_holderSet.size() == 0)
            return ldFlag = true;

        fFlag = true; // 設為已找過的狀態

        bool fNoRootUniBase; // 檢查是不是已經沒有 root 的 Host 存在
        for (auto &it : m_holderSet)
        {
            auto pHost = it->m_pHost;
            fNoRootUniBase = ((RmConst<decltype(pHost)>::type)pHost)->checkNoHost();
            if (!fNoRootUniBase)
                break;
        }

        cFlag = false; // 可再被放入待放棄共用的佇列
        fFlag = false;

        return ldFlag = fNoRootUniBase;
    }

    void cxxlFASTCALL UniResourcePrivate::_UniBase::LD_destroy()
    {
        m_UniBaseMutex.lock();
        m_isDestroy = true;
        while (true)
        {
            auto it = m_holderSet.begin();
            if (it == m_holderSet.end())
                break;

            m_UniBaseMutex.unlock();
            // 解鎖之後，不用擔心 _Holder 會不存在
            // 這是處理機制的
            (*it)->detachUniBase(this);
            m_UniBaseMutex.lock();
        }
        m_UniBaseMutex.unlock();
    }

    void cxxlFASTCALL UniResourcePrivate::_UniBase::LD_clearFlag()
    {
        fFlag = false;
    }

    /****************************************************************************************** */

    bool cxxlFASTCALL UniResourcePrivate::_UniBase::attach(const _Holder *pHolder)
    {
        if (m_isDestroy)
            return false;

        rFlag = true;
        m_holderSet.insert(pHolder);
        return true;
    }

    void cxxlFASTCALL UniResourcePrivate::_UniBase::detach(const _Holder *pHolder)
    {
        m_holderSet.erase(pHolder);
    }

    bool cxxlFASTCALL UniResourcePrivate::_UniBase::attachObserver(const _Holder *pObserver)
    {
        std::lock_guard<std::mutex> lock(m_UniBaseMutex);
        return attach(pObserver);
    }

    void cxxlFASTCALL UniResourcePrivate::_UniBase::detachObserver(const _Holder *pObserver)
    {
        std::lock_guard<std::mutex> lock(m_UniBaseMutex);
        detach(pObserver);
    }

    bool cxxlFASTCALL UniResourcePrivate::_UniBase::attachOwner(const _Holder *pOwner)
    {
        std::lock_guard<std::mutex> lock(m_UniBaseMutex);
        if (!attach(pOwner))
            return false;

        addOwner();
        return true;
    }

    void cxxlFASTCALL UniResourcePrivate::_UniBase::detachOwner(const _Holder *pOwner)
    {
        std::lock_guard<std::mutex> lock(m_UniBaseMutex);
        detach(pOwner);
        m_isDestroy = removeOwner();
    }

    void cxxlFASTCALL UniResourcePrivate::_UniBase::detachMoveOwner(const _Holder *pOwner)
    {
        std::lock_guard<std::mutex> lock(m_UniBaseMutex);
        detach(pOwner);
        removeOwner();
    }


    // 叫用 detachOwner() 之後呼叫放棄共用處理器檢查是否需要放棄共用
    // UniBase_ptr 其實就是自己，只是為了有 std::shared_ptr 包裹，會交給放棄共用處理器
    void cxxlFASTCALL UniResourcePrivate::_UniBase::checkDestroy(const std::shared_ptr<_UniBase> &uniBase_ptr)
    {
        {
            std::lock_guard<std::mutex> lock(m_UniBaseMutex);

            // 已放入待放棄共用佇列的物件不用再次放入
            if (cFlag)
                return;

            // 前次檢查放棄共用處理器已判定須放棄共用
            if (ldFlag)
                return;

            cFlag = true; // 標記已放入放棄共用佇列
        }

        g_pUniBaseDestructor->checkDestroy(std::static_pointer_cast<IDestroyable>(uniBase_ptr));
    }

    // Constructor
    UniResourcePrivate::_UniBase::_UniBase()
    {
    }

    // Destructor
    UniResourcePrivate::_UniBase::~_UniBase()
    {
    }

    bool cxxlFASTCALL UniResourcePrivate::_UniBase::isDestroy() const
    {
        std::lock_guard<std::mutex> lock(m_UniBaseMutex);
        return m_isDestroy;
    }
}