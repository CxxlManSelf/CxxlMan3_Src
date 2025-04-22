/************************************************************************************************
 * uniresdestructor.hpp v1.0.5
 *
 * 提供結束共用 UniBase 功能的介面
 * 
 * 結束共用處理器是獨立的單一執行緒機制，確保同一個時間只會有一個 UniBase 被進行結束共用的處理
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLCORE_UNIBASEDESTRUCTOR_HPP_CxxlMan3
#define __CXXLCORE_UNIBASEDESTRUCTOR_HPP_CxxlMan3

#include <memory>

#include "commondef.hpp"
#include "sysdef.hpp"
#include "cxxlcore.hpp"


namespace CXXL
{

    class _UniBase;

    // 作為要被結束共用處理器的作用介面
    class CXXLCORE_DLLEXPORT IDestroyable
    {
    public:
        virtual ~IDestroyable() = default;

        // 虛擬函數，詢問此物件是否應當被結束共用
        virtual bool cxxlFASTCALL LD_shouldDestroy() = 0;        

        // 當物件須要結束共用時將調用此方法
        virtual void cxxlFASTCALL LD_destroy() = 0;

        // 將 fFlag 清為 false
        virtual void cxxlFASTCALL LD_clearFFlag() = 0;

        // 將 justAddFlag 清為 false
        virtual void cxxlFASTCALL LD_clearJustAddFlag() = 0;
    };

    // 放棄持有的處理界面
    class IUniBaseDestructor
    {
    public:
        virtual ~IUniBaseDestructor() = default;

        // 進行 destroyable_ptr 物件檢測
        // 若標記為結束共用則進行結束共用
        // 否則進行 root 持有者搜尋，若找不到也會進行結束共用
        virtual void cxxlFASTCALL checkDestroy(const std::shared_ptr<IDestroyable> &destroyable_ptr) = 0;

        // 不進行檢測，只放入待放棄清單，以免多執行緒干擾
        virtual void cxxlFASTCALL justAdd(const std::shared_ptr<IDestroyable> &destroyable_ptr) = 0;

        // 被結束共用處理器巡行過的 UniBase 被設定 fFlag，須叫用此函數記錄以便巡行後將 fFlag 清除
        virtual void cxxlFASTCALL reset_fFlag(const IDestroyable *pDestroyable) = 0;
    };

    // 定義在外部的 IUniBaseDestructor 的實例指標
    extern CXXLCORE_DLLEXPORT IUniBaseDestructor *g_pUniBaseDestructor;


    // 等待結束共用處理器的 待結束共用清單 清空，以及結束子執行緒
    class IDestrWaiter
    {
    public:
        // Destructor
        virtual ~IDestrWaiter() {}
    };

    // 主程式須先取得核心銷毁控制器，並於結束前鎖毁。
    // 此控制器只能取得一次。
    extern CXXLCORE_DLLEXPORT std::shared_ptr<IDestrWaiter> cxxlFASTCALL getDestructor();

}

#endif