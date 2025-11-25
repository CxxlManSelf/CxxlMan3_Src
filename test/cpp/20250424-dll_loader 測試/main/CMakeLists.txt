# ========================================================================
# 主程序專案：MyTest - 動態連結庫載入測試程序
#
# 功能說明：
#   此可執行程序展示如何使用 IDllLoader 介面動態載入插件 DLL，
#   並在多執行緒環境中調用插件提供的函數。
#
# 依賴庫：
#   - cxxlcommon  : 公共功能庫 (包含 dll_loader.hpp、threadmgr.hpp、semaphore.hpp)
#   - cxxlcore    : 核心功能庫 (其他功能)
#   - dl (僅 Unix): 動態加載函數庫 (dlopen, dlsym, dlclose)
# ========================================================================

cmake_minimum_required(VERSION 3.10.0)

# ========================================================================
# 版本號設定 - 使用時間戳作為 PATCH 版本
# ========================================================================
# 生成編譯時間戳 (格式: 年份(2位)+天數(3位)+時(2位)+分(2位))
# 例如：25324 表示 2025年第324天 1234分
string(TIMESTAMP BUILD_TIMESTAMP "%y%j%H%M")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})

# 定義專案名稱和版本 (例如：0.1.2532410834)
project(MyTest VERSION 0.1.${PROJECT_VERSION_PATCH} DESCRIPTION "動態連結載入測試" LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

# ========================================================================
# 收集原始碼檔案
# ========================================================================
# 自動收集當前目錄下所有原始碼檔案到 SRC_LIST 變數
aux_source_directory(. SRC_LIST)

# ========================================================================
# 跨平台連結庫設定
# ========================================================================
if(WIN32)
  # Windows 平台
  if(MSVC)
    # Microsoft Visual C++ 編譯器
    # Release 版本連結 cxxlcore.lib 和 cxxlcommon.lib
    set(LIBSRC_LIST optimized cxxlcore.lib optimized cxxlcommon.lib)
    # Debug 版本連結 cxxlcore_d.lib 和 cxxlcommon_d.lib (帶 _d 後綴)
    set(LIBSRC_D_LIST debug cxxlcore_d.lib debug cxxlcommon_d.lib)
  else()
    if(MINGW)
      # MinGW 編譯器 (Windows 上的 GCC)
      # 使用 -l 參數指定連結庫名稱
      set(LIBSRC_LIST optimized -lcxxlcore optimized -lcxxlcommon)
      set(LIBSRC_D_LIST debug -lcxxlcore_d debug -lcxxlcommon_d)
    endif()
  endif()
else()
  if(UNIX)
    # Unix-like 系統 (Linux, macOS)
    # 需要額外連結 -ldl 以支援動態載入函數 (dlopen/dlsym/dlclose)
    set(LIBSRC_LIST optimized -lcxxlcommon optimized -lcxxlcore optimized -ldl)
    set(LIBSRC_D_LIST debug -lcxxlcommon_d debug -lcxxlcore_d debug -ldl)
  else()
    # 不支援的平台
    message(FATAL_ERROR "Not available for this platformn in MyTest.")
  endif()
endif()

# ========================================================================
# 建置目標設定
# ========================================================================
# 建立可執行檔案，使用自動收集的原始碼
add_executable(${TARGET_NAME} ${SRC_LIST} )

# 連結所需的函數庫
# Debug 和 Release 版本會自動選擇對應的庫
target_link_libraries(${TARGET_NAME}
  ${LIBSRC_D_LIST}    # Debug 版本的庫
  ${LIBSRC_LIST}       # Release 版本的庫
)