cmake_minimum_required(VERSION 3.10.0)
project(MyTest VERSION 0.1.0 LANGUAGES C CXX)



include_directories("../dll")

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ThreadDll)

