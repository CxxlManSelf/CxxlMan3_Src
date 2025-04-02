/************************************************************************************************
 * liferesdestructor.hpp v0.1.0
 *
 * 提供銷毀 LifeRes 功能的介面
 * 
 * 銷毀處理器是獨立的單一執行緒機制，確保同一個時間只會有一個 LifeRes 進行銷毀的處理
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLCORE_LIFERESDESTRUCTOR_HPP_CxxlMan3
#define __CXXLCORE_LIFERESDESTRUCTOR_HPP_CxxlMan3

#include <memory>

#include "commondef.hpp"
#include "sysdef.hpp"

#if defined(_CXXLCORE_DLLEXPORT)
#define CXXLLIFERES_DLLEXPORT CXXL_DLLEXPORT
#else
#define CXXLLIFERES_DLLEXPORT CXXL_DLLIMPORT
#endif



namespace CXXL
{

    class _LifeRes;

    // 要被銷毀的 LifeRes 的界面，作為要被銷毀處理器銷毀的作用對象
    class IDestroyable
    {
    public:
        virtual ~IDestroyable() = default;

        // 虛擬函數，詢問此物件是否應當被銷毀
        virtual bool cxxlFASTCALL LD_shouldDestroy() const = 0;        

        // 當物件須要銷毀時將調用此方法
        virtual void cxxlFASTCALL LD_destroy() = 0;

        // 將 fFlag 清為 false
        virtual void cxxlFASTCALL LD_clearFlag() = 0;
    };

    // LifeRes<> 的銷毀處理器的使用界面
    class ILifeResDestructor
    {
    public:
        virtual ~ILifeResDestructor() = default;

        // 進行 pDestroyable 物件銷毀檢測
        // 若標記為銷毀則進行銷毀
        // 否則進行存活持有者搜尋，若找不到也會進行銷毀
        virtual void cxxlFASTCALL checkDestroy(const IDestroyable *pDestroyable) = 0;

        // 被銷毀處理器巡行過的 LifeRes 被設定 fFlag，須叫用此函數以便巡行後將 fFlag 清除
        virtual void cxxlFASTCALL clearFlag(const IDestroyable *pDestroyable) = 0;
    };

    // 定義在外部的 ILifeResDestructor 的實例指標
    extern ILifeResDestructor *g_pLifeResDestructor;


}

#endif