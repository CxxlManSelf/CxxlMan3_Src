project(cxxlcore VERSION 0.1.0 LANGUAGES CXX)

# 變譯程式庫時需要加入這個巨集，以作為和使用端的編譯做區別
add_definitions(-D_CXXLCORE_EXPORT)

set(SRC_LIST "src/life_res_destructor.cpp" "src/cxxlstrpbrk.cpp" "src/u8_string.cpp")
set(LIBSRC_LIST cxxlcommon)

if(NOT WIN32)
    set(LIBSRC_LIST ${LIBSRC_LIST} pthread)
endif()

include_directories("../cxxlcommon/include" "include")
# add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
# target_link_libraries(${PROJECT_NAME} ${LIBSRC_LIST})

# set_target_properties(${PROJECT_NAME} 
#   PROPERTIES 
#     VERSION ${PROJECT_VERSION} 
#     SOVERSION ${PROJECT_VERSION_MAJOR})
