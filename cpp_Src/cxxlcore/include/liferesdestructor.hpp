/************************************************************************************************
 * liferesdestructor.hpp v0.1.0
 *
 * 提供銷毀 LifeRes 功能的介面
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLCORE_LIFERESDESTRUCTOR_HPP_CxxlMan3
#define __CXXLCORE_LIFERESDESTRUCTOR_HPP_CxxlMan3

#include "commondef.hpp"
#include "sysdef.hpp"

namespace CXXL
{

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
        virtual void cxxlFASTCALL checkDestroy(IDestroyable *pDestroyable) = 0;
    };

    // 定義在外部的 ILifeResDestructor 的實例指標
    extern ILifeResDestructor *g_pLifeResDestructor;


}

#endif