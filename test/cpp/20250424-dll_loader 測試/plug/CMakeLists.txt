# ========================================================================
# 插件專案：PlugDll - 提供四則運算功能的動態連結庫
#
# 功能說明：
#   此動態連結庫提供四個可供外部調用的函數：
#   - add     : 加法運算
#   - sub     : 減法運算
#   - mul     : 乘法運算
#   - divide  : 除法運算
#
#   這些函數使用 extern "C" 和 CXXL_DLLEXPORT 導出，
#   使其可以被 IDllLoader 動態載入和調用。
#
# 編譯產出：
#   - Windows (MSVC)   : PlugDll.dll
#   - Windows (MinGW)  : libPlugDll.dll
#   - Linux            : libPlugDll.so
#   - macOS            : libPlugDll.dylib
# ========================================================================

cmake_minimum_required(VERSION 3.10.0)

# ========================================================================
# 版本號設定 - 使用時間戳作為 PATCH 版本
# ========================================================================
# 生成編譯時間戳 (格式: 年份(2位)+天數(3位)+時(2位)+分(2位))
string(TIMESTAMP BUILD_TIMESTAMP "%y%j%H%M")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})

# 定義專案名稱和版本
project(PlugDll VERSION 0.1.${PROJECT_VERSION_PATCH} DESCRIPTION "動態連結程式庫" LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

# ========================================================================
# 跨平台連結庫設定
# ========================================================================
if(WIN32)
  # Windows 平台
  if(MSVC)
    # Microsoft Visual C++ 編譯器
    set(LIBSRC_LIST optimized cxxlcore.lib optimized cxxlcommon.lib)
    set(LIBSRC_D_LIST debug cxxlcore_d.lib debug cxxlcommon_d.lib)
  else()
    if(MINGW)
      # MinGW 編譯器 (Windows 上的 GCC)
      set(LIBSRC_LIST optimized -lcxxlcore optimized -lcxxlcommon)
      set(LIBSRC_D_LIST debug -lcxxlcore_d debug -lcxxlcommon_d)
    endif()
  endif()
else()
  if(UNIX)
    # Unix-like 系統 (Linux, macOS)
    # 需要額外連結 -ldl 以支援動態載入功能
    set(LIBSRC_LIST optimized -lcxxlcommon optimized -lcxxlcore optimized -ldl)
    set(LIBSRC_D_LIST debug -lcxxlcommon_d debug -lcxxlcore_d debug -ldl)
  else()
    # 不支援的平台
    message(FATAL_ERROR "Not available for this platformn in PlugDll.")
  endif()
endif()

# ========================================================================
# 建置目標設定
# ========================================================================
# 建立動態連結庫 (MODULE 類型)
# MODULE 類型表示此庫不會在編譯時被其他目標連結，
# 而是在執行時通過 dlopen/LoadLibrary 動態載入
add_library(${PROJECT_NAME} MODULE plugdll.cpp)

# 連結所需的函數庫
target_link_libraries(${TARGET_NAME}
  ${LIBSRC_D_LIST}    # Debug 版本的庫
  ${LIBSRC_LIST}       # Release 版本的庫
)

