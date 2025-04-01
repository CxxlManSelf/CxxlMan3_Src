cmake_minimum_required(VERSION 3.10.0)
string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d%H%M%S")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})
project(cxxlcommon VERSION 0.1.${PROJECT_VERSION_PATCH} LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

include_directories(include)

#add_definitions(-D_CXXLCOMMON_EXPORT)
#add_library(${PROJECT_NAME} cxxlcommon.cpp)

install(DIRECTORY include/ DESTINATION include)