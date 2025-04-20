cmake_minimum_required(VERSION 3.10.0)
project(mytest VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



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



add_executable(mytest main.cpp)
target_link_libraries(${PROJECT_NAME} ${LIBSRC_LIST} ${LIBSRC_D_LIST})

