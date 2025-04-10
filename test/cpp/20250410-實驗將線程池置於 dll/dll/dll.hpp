
#include <commondef.hpp>
#include <threadmgr.hpp>


#if defined(MYDLL_EXPORTS)
#define MYDLL_DLLEXPORT CXXL_DLLEXPORT
#else
#define MYDLL_DLLEXPORT CXXL_DLLIMPORT
#endif

extern MYDLL_DLLEXPORT CxxlMan3::ThreadPool<true> g_threadPool;

