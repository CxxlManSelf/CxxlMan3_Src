cmake_minimum_required(VERSION 3.10.0)

string(TIMESTAMP BUILD_TIMESTAMP "%y%j%H%M")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})
project(mytest VERSION 0.1.${PROJECT_VERSION_PATCH} LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  set(LIBSRC_LIST optimized cxxlcore.lib)
  set(LIBSRC_D_LIST debug cxxlcore_d.lib)
else()
  if(UNIX)
    set(LIBSRC_LIST optimized cxxlcore)
    set(LIBSRC_D_LIST debug cxxlcore_d)
  else()
    message(FATAL_ERROR "Not available for this platform in mytest.")
  endif()
endif()


add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME}
  ${LIBSRC_D_LIST}
  ${LIBSRC_LIST}
)

