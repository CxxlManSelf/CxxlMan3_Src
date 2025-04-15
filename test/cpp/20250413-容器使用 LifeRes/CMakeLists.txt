cmake_minimum_required(VERSION 3.10.0)
project(mytest VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_LIST "main.cpp")

if (MSVC)
  set(LIBSRC_LIST optimized cxxlcore.lib)
  set(LIBSRC_D_LIST debug cxxlcore_d.lib)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND WIN32)
  set(LIBSRC_LIST optimized -lcxxlcore)
  set(LIBSRC_D_LIST debug -lcxxlcore_d)
elseif (UNIX)
  set(LIBSRC_LIST optimized -lcxxlcore)
  set(LIBSRC_D_LIST debug -lcxxlcore_d)
endif()

add_executable(${PROJECT_NAME} ${SRC_LIST} )
target_link_libraries(${PROJECT_NAME} ${LIBSRC_LIST} ${LIBSRC_D_LIST})
