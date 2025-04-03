/*******************************************************
 * 
 * rm_const_point.hpp 1.0.0
 *   
 * Description: 去除 const 限定的指標，可以處理以下的指標形式
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

#ifndef __CXXLCOMMON_RM_CONST_POINT_HPP_CxxlMan3
#define __CXXLCOMMON_RM_CONST_POINT_HPP_CxxlMan3

namespace CXXL
{

    template<typename T>
    struct RemoveConst
    {
        typedef T type;
    };
    
    template<typename T>
    struct RemoveConst<const T>
    {
        typedef T type;
    };
    
    template<typename T>
    struct RemoveConst<T*>
    {
        typedef typename RemoveConst<T>::type *type;
    };
    
    template<typename T>
    struct RemoveConst<const T*>
    {
        typedef typename RemoveConst<T>::type *type;
    };
        
}

#endif