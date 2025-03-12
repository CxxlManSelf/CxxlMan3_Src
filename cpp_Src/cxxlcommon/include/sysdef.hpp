/***************************
* sysdef.hpp v0.1.0
*
* 依照不同的平台做適當的設定
*
* Author: CxxlMan
* Date: 2022-08-01 
***************************/

#ifndef __CXXLCOMMON_SYSDEF_HPP_CxxlMan3
#define __CXXLCOMMON_SYSDEF_HPP_CxxlMan3

#if defined(_WIN32) || defined(_WIN64)
    // Windows platform
    #define PLATFORM_NAME "Windows"
    #define cxxlSLASH u8'\\'
    #define cxxlCDECL __cdecl
    #define cxxlSTDCALL __stdcall
    #define cxxlFASTCALL __fastcall
  #elif defined(__APPLE__) || defined(__MACH__)
    // MacOS platform
    #define PLATFORM_NAME "MacOS"
    #define cxxlSLASH u8'/'
    #define cxxlCDECL
    #define cxxlSTDCALL
    #define cxxlFASTCALL

#elif defined(__linux__)
    // Linux platform
    #define PLATFORM_NAME "Linux"
    #define cxxlSLASH u8'/'
    #define cxxlCDECL
    #define cxxlSTDCALL
    #define cxxlFASTCALL
#else
    // Unsupported platform
    #define PLATFORM_NAME "Unknown"
    #error "Unsupported platform"
#endif

#endif // __CXXLCOMMON_SYSDEF_HPP_CxxlMan3
