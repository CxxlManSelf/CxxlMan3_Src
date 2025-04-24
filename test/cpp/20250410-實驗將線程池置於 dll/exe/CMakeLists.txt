cmake_minimum_required(VERSION 3.10.0)

string(TIMESTAMP BUILD_TIMESTAMP "%y%j%H%M")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})
project(MyTest VERSION 0.1.${PROJECT_VERSION_PATCH} DESCRIPTION "實驗將線程池置於 dll" LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

aux_source_directory(. SRC_LIST)

if(WIN32)
  if(MSVC)
    set(LIBSRC_LIST optimized cxxlcore.lib optimized cxxlcommon.lib)
    set(LIBSRC_D_LIST debug cxxlcore_d.lib debug cxxlcommon_d.lib)
  else()
    if(MINGW)
      set(LIBSRC_LIST optimized -lcxxlcore optimized -lcxxlcommon)
      set(LIBSRC_D_LIST debug -lcxxlcore_d debug -lcxxlcommon_d)
    endif()
  endif()
else()
  if(UNIX)
    set(LIBSRC_LIST optimized -lcxxlcommon optimized -lcxxlcore optimized -ldl)
    set(LIBSRC_D_LIST debug -lcxxlcommon_d debug -lcxxlcore_d debug -ldl)
  else()
    message(FATAL_ERROR "Not available for this platformn in mytest.")
  endif()
endif()

include_directories("../dll")

add_executable(${TARGET_NAME} ${SRC_LIST} )
target_link_libraries(${TARGET_NAME} PRIVATE ThreadDll
  ${LIBSRC_D_LIST}
  ${LIBSRC_LIST}
)