#string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d%H%M%S")
string(TIMESTAMP BUILD_TIMESTAMP "%y%j%H%M")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})
project(cxxlcore VERSION 0.1.${PROJECT_VERSION_PATCH} DESCRIPTION "CxxlMan3 最底層核心" LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in 
                       ${CMAKE_CURRENT_BINARY_DIR}/version.rc 
                      @ONLY)

set(SRC_LIST "src/life_res_destructor.cpp" "src/liferes.cpp")

#set(LIBSRC_LIST cxxlcommon)
if(UNIX)
    set(LIBSRC_LIST ${LIBSRC_LIST} pthread)
endif()

# 編譯程式庫時需要加入這個巨集，以作為和使用端的編譯做區別
add_definitions(-D_CXXLCORE_DLLEXPORT)

include_directories("../cxxlcommon/include" "include")
add_library(${TARGET_NAME} SHARED ${SRC_LIST})
target_sources(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
if (NOT LIBSRC_LIST STREQUAL "")
    target_link_libraries(${TARGET_NAME} ${LIBSRC_LIST})
endif()


INSTALL(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

install(DIRECTORY include/ DESTINATION include)