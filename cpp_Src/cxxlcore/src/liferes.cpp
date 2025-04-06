#include "liferes.hpp"

namespace CXXL
{

    bool cxxlFASTCALL LifeResourcePrivate::_LifeRes::checkNoHost()
    {
        // 已找過
        if (fFlag) return true;

        fFlag = true;   // 設為已找過的狀態
        g_pLifeResDestructor->reset_fFlag(this);

        std::lock_guard<std::mutex> lock(m_LifeResMutex);

        if (rFlag == false)  // 本身是 rootLifeRes,或未放入過 _OwnerObserverBase 為 false
            return false;

        if (cFlag || m_OwnerObserverSet.size() == 0)
            return false; // 回報我也要處理待刪確認
        
        bool fNoRootLifeRes = true; // 有找到 rootLifeRes 或還有要放棄持有的處理回覆 false
        for(auto &it : m_OwnerObserverSet)
        {            
            auto pHost = it->m_pHost;

            fNoRootLifeRes = ((RmConst<decltype(pHost)>::type)pHost)->checkNoHost();
            if(!fNoRootLifeRes) break;
        }
        return fNoRootLifeRes;
    }

    bool cxxlFASTCALL LifeResourcePrivate::_LifeRes::LD_shouldDestroy()
    {
        std::lock_guard<std::mutex> lock(m_LifeResMutex);


        if(m_isDestroy) return ldFlag = true; // 已標識須銷毀

        // 無持有者了
        if(m_OwnerObserverSet.size() == 0) return ldFlag = true;

        fFlag = true;   // 設為已找過的狀態

        bool fNoRootLifeRes; // 檢查是不是已經沒有未銷毀的 Host 存在
        for(auto &it : m_OwnerObserverSet)
        {
            auto pHost = it->m_pHost;
            fNoRootLifeRes = ((RmConst<decltype(pHost)>::type)pHost)->checkNoHost();
            if(!fNoRootLifeRes) break;
        }

        cFlag = 0;  // 可再被放入待刪佇列
        fFlag = false;

        return ldFlag = fNoRootLifeRes; 
    }

    void cxxlFASTCALL LifeResourcePrivate::_LifeRes::LD_destroy()
    {
        m_LifeResMutex.lock();
        m_isDestroy = true;
        while (m_OwnerObserverSet.size() > 0)
        {
            auto it = m_OwnerObserverSet.begin();
            // m_OwnerObserverSet.erase(it);
            m_LifeResMutex.unlock();
            // 解鎖之後，不用擔心 _OwnerObserverBase 會不存在
            // 因為這是銷毀處理器的約定機制
            (*it)->detachLifeRes(this);
            m_LifeResMutex.lock();
        }
        m_LifeResMutex.unlock();
    }

    void cxxlFASTCALL LifeResourcePrivate::_LifeRes::LD_clearFlag()
    {
        fFlag = false;
    }


/****************************************************************************************** */

    bool cxxlFASTCALL LifeResourcePrivate::_LifeRes::attach(const _OwnerObserverBase *pOwnerObserver)
    {
        if (m_isDestroy)
            return false;

        rFlag = true;
        m_OwnerObserverSet.insert(pOwnerObserver);
        return true;
    }

    void cxxlFASTCALL LifeResourcePrivate::_LifeRes::detach(const _OwnerObserverBase *pOwnerObserver)
    {
        m_OwnerObserverSet.erase(pOwnerObserver);
    }

    bool cxxlFASTCALL LifeResourcePrivate::_LifeRes::attachObserver(const _OwnerObserverBase *pObserver)
    {
        std::lock_guard<std::mutex> lock(m_LifeResMutex);
        return attach(pObserver);
    }

    void cxxlFASTCALL LifeResourcePrivate::_LifeRes::detachObserver(const _OwnerObserverBase *pObserver)
    {
        std::lock_guard<std::mutex> lock(m_LifeResMutex);
        detach(pObserver);
    }

    bool cxxlFASTCALL LifeResourcePrivate::_LifeRes::attachOwner(const _OwnerObserverBase *pOwner)
    {
        std::lock_guard<std::mutex> lock(m_LifeResMutex);
        if (!attach(pOwner))
            return false;

        addOwner();
        return true;
    }

    void cxxlFASTCALL LifeResourcePrivate::_LifeRes::detachOwner(const _OwnerObserverBase *pOwner)
    {
        std::lock_guard<std::mutex> lock(m_LifeResMutex);
        m_isDestroy = removeOwner();
    }

    // 叫用 detachOwner() 之後呼叫銷毁器檢查是否需要銷毀
    // lifeRes_ptr 其實就是自己，只是為了有 std::shared_ptr 包裹，會交給銷毀器
    void cxxlFASTCALL LifeResourcePrivate::_LifeRes::checkDestroy(const std::shared_ptr<_LifeRes> &lifeRes_ptr)
    {
        {
            std::lock_guard<std::mutex> lock(m_LifeResMutex);

            // 已放入待刪佇列的物件不用再次放入
            if (cFlag)
                return;

            // 前次檢查銷毀處理器已判定須銷毀
            if (ldFlag)
                return;

            cFlag = true; // 標記已放入待刪佇列
        }

        g_pLifeResDestructor->checkDestroy(std::static_pointer_cast<IDestroyable>(lifeRes_ptr));
    }

}