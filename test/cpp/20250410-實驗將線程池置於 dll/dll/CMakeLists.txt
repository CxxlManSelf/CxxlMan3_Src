cmake_minimum_required(VERSION 3.10.0)
project(ThreadDll VERSION 0.1.0 LANGUAGES C CXX)


add_library(${PROJECT_NAME} SHARED dll.cpp)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE 
        MYDLL_EXPORTS)