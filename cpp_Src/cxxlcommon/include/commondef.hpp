/***********************************************************
 * commondef.hpp v1.0.0
 * 
 * 通用的定義
 * 
 * Author: CxxlMan
 * Date: 2025-  
 ***********************************************************/

#ifndef __CXXLCOMMON_COMMONDEF_HPP_CxxlMan3
#define __CXXLCOMMON_COMMONDEF_HPP_CxxlMan3

// namespace
#define CXXL CxxlMan3

// c++20 提供
#if defined(__cpp_lib_char8_t)
  #define cxxlU8CHAR char8_t
  #define cxxlSTDSTRING std::u8string
#else
  #define cxxlU8CHAR char
  #define cxxlSTDSTRING std::string
#endif



#endif // __CXXLCOMMON_COMMONDEF_HPP_CxxlMan3