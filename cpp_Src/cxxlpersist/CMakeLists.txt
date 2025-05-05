#string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d%H%M%S")
string(TIMESTAMP BUILD_TIMESTAMP "%y%j%H%M")
set(PROJECT_VERSION_PATCH ${BUILD_TIMESTAMP})
project(cxxlpersist VERSION 0.1.${PROJECT_VERSION_PATCH} DESCRIPTION "CxxlMan3 永續儲存" LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in 
                       ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}/version.rc 
                      @ONLY)

include_directories("../cxxlcommon/include" "../cxxlcore/include" "include")
aux_source_directory(src SRC_LIST)

set(LIBSRC_LIST cxxlcommon cxxlcore)
if(UNIX)
    set(LIBSRC_LIST ${LIBSRC_LIST} pthread)
endif()

# 編譯程式庫時需要加入這個巨集，以作為和使用端的編譯做區別
add_definitions(-D_CXXLPERSIST_DLLEXPORT)

#add_library(${TARGET_NAME} SHARED ${SRC_LIST})
#target_sources(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}/version.rc)
#if (NOT LIBSRC_LIST STREQUAL "")
#    target_link_libraries(${TARGET_NAME} ${LIBSRC_LIST})
#endif()


#install(TARGETS ${TARGET_NAME}
#        RUNTIME DESTINATION bin
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib
#        )

#if (MSVC)        
#    install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> DESTINATION bin OPTIONAL)
#endif()

install(DIRECTORY include/ DESTINATION include)