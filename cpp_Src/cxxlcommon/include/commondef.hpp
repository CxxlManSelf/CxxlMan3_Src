/***********************************************************
 * commondef.hpp v0.1.0
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

/* 
  為 DLL 的函數和 class 提供前綴語法

  CXXL_DLLEXPORT   提供作為 dll 時使用
  CXXL_DLLIMPORT   使用端引用時使用
*/
#if defined(_WIN32) || defined(_WIN64)
  #define CXXL_DLLEXPORT __declspec( dllexport )
  #define CXXL_DLLIMPORT __declspec( dllimport )
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
  #define CXXL_DLLEXPORT
  #define CXXL_DLLIMPORT
#else
  #error "Unsupported platform"
#endif  


#endif // __CXXLCOMMON_COMMONDEF_HPP_CxxlMan3