/*****************************************************************************
 * cxxlpersist.hpp v1.0.0
 *
 * CxxlMan3 採用的永續儲存標準介面約定
 *
 * Author: CxxlMan
 * Date: 2025 -
 ******************************************************************************/
#ifndef __CXXLPERSIST_CXXLPERSIST_HPP_CxxlMan3
#define __CXXLPERSIST_CXXLPERSIST_HPP_CxxlMan3

#include "commondef.hpp"

namespace CXXL
{

    class Persistable;

    // 在此宣告一些 private 類別
    class PersistResourcePrivate
    {
        // Persistable 的真正實作基底類別
        class _Persistable
        {
        };

        friend class Persistable;
    };

    /**
     * 可永久儲存的物件基礎類別
     * 所有需要永久儲存的物件都應該繼承此類別
     */
    class Persistable : virtual public PersistResourcePrivate::_Persistable
    {
    };

}

#endif // __CXXLPERSIST_CXXLPERSIST_HPP_CxxlMan3
