/**************************************************************
* sysdef.hpp v1.0.1
*
* 依照不同的平台做適當的設定
*
* Author: CxxlMan
* Date: 2025-
**************************************************************/


#ifndef __CXXLCOMMON_SYSDEF_HPP_CxxlMan3
#define __CXXLCOMMON_SYSDEF_HPP_CxxlMan3

// 平台的代表編號
#define _UNKNOWN_CxxlMan3 0
#define _WINDOWS_CxxlMan3 1   //  WIN 平台
#define _LINUX_CxxlMan3 2   //  linux 平台
#define _MAC_CxxlMan3 3     //  MAC 平台



#if defined(_WIN32) || defined(_WIN64)
    // Windows platform
    #define PLATFORM_NAME _WINDOWS_CxxlMan3
    #define cxxlSLASH u8'\\'
    #define cxxlCDECL __cdecl
    #define cxxlSTDCALL __stdcall
    #define cxxlFASTCALL __fastcall
  #elif defined(__APPLE__) || defined(__MACH__)
    // MacOS platform
    #define PLATFORM_NAME _MAC_CxxlMan3
    #define cxxlSLASH u8'/'
    #define cxxlCDECL
    #define cxxlSTDCALL
    #define cxxlFASTCALL

#elif defined(__linux__)
    // Linux platform
    #define PLATFORM_NAME _LINUX_CxxlMan3
    #define cxxlSLASH u8'/'
    #define cxxlCDECL
    #define cxxlSTDCALL
    #define cxxlFASTCALL
#else
    // Unsupported platform
    #define PLATFORM_NAME _UNKNOWN_CxxlMan3
    #error "Unsupported platform"
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


#endif // __CXXLCOMMON_SYSDEF_HPP_CxxlMan3
