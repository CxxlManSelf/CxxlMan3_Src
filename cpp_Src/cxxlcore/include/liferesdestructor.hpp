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

namespace CXXL
{

    class _LifeRes;

    // 要被銷毀的 LifeRes 的界面，作為要被銷毀處理器銷毀的作用對象
    class IDestroyable
    {
    public:
        virtual ~IDestroyable() = default;

        // 當物件需要銷毀時將調用此方法
        virtual void cxxlFASTCALL destroy() = 0;
    };

    // LifeRes<> 的銷毀處理器的使用界面
    class ILifeResDestructor
    {
    public:
        virtual ~ILifeResDestructor() = default;

        // 檢查 IDestroyable 物件是否需要被銷毀時將調用此方法
        virtual void cxxlFASTCALL checkDestroy(const IDestroyable *pDestroy) = 0;
    };

    // 定義在外部的 ILifeResDestructor 的實例指標
    extern ILifeResDestructor *g_pLifeResDestructor;


}

#endif