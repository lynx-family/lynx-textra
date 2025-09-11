cmake_minimum_required(VERSION 3.10)

project(perftest)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED PACKAGE_FIND_FILE)
message("PACKAGE_FIND_FILE: ${PACKAGE_FIND_FILE}")
include(${PACKAGE_FIND_FILE})
endif()

add_library(perftest SHARED
    src/main/cpp/napi_init.cc
    src/main/cpp/oh_drawing_test.cc
    src/main/cpp/oh_drawing_test.h
    src/main/cpp/textra_test.cc
    src/main/cpp/textra_test.h
    src/main/cpp/oh_drawing_newapi_test.cc
    src/main/cpp/oh_drawing_newapi_test.h
    )
target_link_libraries(perftest PUBLIC
    lynxtextra::lynxtextra
    libhilog_ndk.z.so
    libace_napi.z.so
    libace_ndk.z.so
    libnative_drawing.so
    libnative_display_manager.so
    )