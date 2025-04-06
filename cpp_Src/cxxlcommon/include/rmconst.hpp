/*******************************************************
 *
 * rmconst.hpp 1.0.0
 *
 * 去除被 const 限定的類型，可以處理以下的指標形式
 *
 * 1. T*
 * 2. const T*
 * 3. T**
 * 4. const T**
 * 5. ....
 *
 *
 * Author: CxxlMan
 * Date: 2025-
 ********************************************************/
#ifndef __CXXLCOMMON_RMCONST_HPP_CxxlMan3
#define __CXXLCOMMON_RMCONST_HPP_CxxlMan3

#include "commondef.hpp"

namespace CXXL
{

    template <typename T>
    struct RmConst
    {
        typedef T type;
    };

    template <typename T>
    struct RmConst<const T>
    {
        typedef T type;
    };

    template <typename T>
    struct RmConst<T *>
    {
        typedef typename RmConst<T>::type *type;
    };

    template <typename T>
    struct RmConst<const T *>
    {
        typedef typename RmConst<T>::type *type;
    };

}

#endif