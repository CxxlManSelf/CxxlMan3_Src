cmake_minimum_required(VERSION 3.10.0)

string(TIMESTAMP BUILD_TIMESTAMP "%y%j%H%M")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})
project(mytest VERSION 0.1.${PROJECT_VERSION_PATCH} LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

aux_source_directory(. SRC_LIST)

if(WIN32)
  if(MSVC)
    set(LIBSRC_LIST optimized cxxlcore.lib)
    set(LIBSRC_D_LIST debug cxxlcore_d.lib)
  else()
    if(MINGW)
      set(LIBSRC_LIST optimized -lcxxlcore)
      set(LIBSRC_D_LIST debug -lcxxlcore_d)
    endif()
  endif()
else()
  if(UNIX)
    set(LIBSRC_LIST optimized -lcxxlcore optimized -ldl)
    set(LIBSRC_D_LIST debug -lcxxlcore_d debug -ldl)
  else()
    message(FATAL_ERROR "Not available for this platformn in mytest.")
  endif()
endif()


add_executable(${TARGET_NAME} ${SRC_LIST} )
target_link_libraries(${TARGET_NAME}
  ${LIBSRC_D_LIST}
  ${LIBSRC_LIST}
)

