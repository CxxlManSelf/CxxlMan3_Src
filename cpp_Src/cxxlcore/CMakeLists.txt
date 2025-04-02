string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d%H%M%S")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})
project(cxxlcore VERSION 0.1.${PROJECT_VERSION_PATCH} LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

set(SRC_LIST "src/life_res_destructor.cpp")

set(LIBSRC_LIST cxxlcommon)
if(NOT WIN32)
    set(LIBSRC_LIST ${LIBSRC_LIST} pthread)
endif()

# 編譯程式庫時需要加入這個巨集，以作為和使用端的編譯做區別
add_definitions(-D_CXXLCORE_DLLEXPORT)

include_directories("../cxxlcommon/include" "include")
add_library(${TARGET_NAME} SHARED ${SRC_LIST})
#target_link_libraries(${TARGET_NAME} ${LIBSRC_LIST})

INSTALL(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY include/ DESTINATION include)