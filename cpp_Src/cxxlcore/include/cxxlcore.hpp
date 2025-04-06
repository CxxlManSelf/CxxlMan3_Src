/*******************************************************************************
 * cxxlcore.hpp v0.1.0
 * 
 * C++ Library Core
 * 
 * Author: CxxlMan
 * Date: 2025 -
*******************************************************************************/

#include "commondef.hpp"

#if defined(_CXXLCORE_DLLEXPORT)
#define CXXLCORE_DLLEXPORT CXXL_DLLEXPORT
#else
#define CXXLCORE_DLLEXPORT CXXL_DLLIMPORT
#endif
