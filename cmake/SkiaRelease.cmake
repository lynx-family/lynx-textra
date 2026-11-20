# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0.

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
    message(FATAL_ERROR "The pinned Skia SDK supports Linux x64 only")
endif()
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "The Skia m130 binary uses libc++18; configure with buildtools/llvm/bin/clang++")
endif()
get_filename_component(textra_source_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(skia_sdk "${textra_source_root}/third_party/skia_release/linux-x64")
if(NOT EXISTS "${skia_sdk}/libskia.a" OR NOT EXISTS "${skia_sdk}/headers.tar.gz")
    message(FATAL_ERROR "Run tools/hab sync . --target-only --target skia-linux first")
endif()

if(CMAKE_VERSION VERSION_LESS 3.18)
    message(FATAL_ERROR "The Skia prebuilt requires CMake 3.18 or newer")
endif()
# Gitiles directory archives have unstable tar metadata. Use a checksum-pinned
# GitHub archive but extract only the SDK headers and license, never .cc files.
set(skia_header_prefix "skia-0f81dc8b2e43c060972a8d8d229b346694a674f8")
set(skia_headers "${skia_sdk}/headers/${skia_header_prefix}")
if(NOT EXISTS "${skia_headers}/include/core/SkCanvas.h")
    file(ARCHIVE_EXTRACT INPUT "${skia_sdk}/headers.tar.gz"
        DESTINATION "${skia_sdk}/headers"
        PATTERNS "${skia_header_prefix}/include/*"
            "${skia_header_prefix}/modules/skcms/skcms.h"
            "${skia_header_prefix}/modules/skcms/src/skcms_public.h"
            "${skia_header_prefix}/LICENSE")
endif()

# Extract pinned LLVM development packages locally; no system installation or
# source build. Use the same standard library for Textra and the demo C++ ABI.
find_program(SKIA_DPKG_DEB dpkg-deb REQUIRED)
file(GLOB skia_runtime_packages "${skia_sdk}/packages/*.deb")
list(LENGTH skia_runtime_packages skia_runtime_package_count)
if(NOT skia_runtime_package_count EQUAL 3)
    message(FATAL_ERROR "Skia requires the three pinned libc++18 development packages; rerun Habitat sync")
endif()
set(skia_runtime "${skia_sdk}/libcxx")
set(skia_runtime_lib "${skia_runtime}/usr/lib/llvm-18/lib")
set(skia_runtime_include "${skia_runtime}/usr/lib/llvm-18/include/c++/v1")
set(skia_runtime_digest "")
foreach(package IN LISTS skia_runtime_packages)
    file(SHA256 "${package}" package_digest)
    string(APPEND skia_runtime_digest "${package_digest}")
endforeach()
set(skia_runtime_stamp "${skia_runtime}/.packages-sha256")
set(skia_runtime_previous "")
if(EXISTS "${skia_runtime_stamp}")
    file(READ "${skia_runtime_stamp}" skia_runtime_previous)
endif()
if(NOT skia_runtime_previous STREQUAL skia_runtime_digest
        OR NOT EXISTS "${skia_runtime_include}/vector"
        OR NOT EXISTS "${skia_runtime_lib}/libc++.a"
        OR NOT EXISTS "${skia_runtime_lib}/libc++abi.a"
        OR NOT EXISTS "${skia_runtime_lib}/libunwind.a")
    foreach(package IN LISTS skia_runtime_packages)
        execute_process(COMMAND "${SKIA_DPKG_DEB}" --extract "${package}" "${skia_runtime}"
            RESULT_VARIABLE extract_result)
        if(NOT extract_result EQUAL 0)
            message(FATAL_ERROR "Failed to extract ${package}")
        endif()
    endforeach()
    file(WRITE "${skia_runtime_stamp}" "${skia_runtime_digest}")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc++ -isystem \"${skia_runtime_include}\" -fno-rtti")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib++")
find_package(Threads REQUIRED)
add_library(textra_skia_runtime INTERFACE)
target_link_libraries(textra_skia_runtime INTERFACE
    "${skia_runtime_lib}/libc++.a" "${skia_runtime_lib}/libc++abi.a"
    "${skia_runtime_lib}/libunwind.a" Threads::Threads ${CMAKE_DL_LIBS} m)
add_library(skia STATIC IMPORTED GLOBAL)
set_target_properties(skia PROPERTIES
    IMPORTED_LOCATION "${skia_sdk}/libskia.a"
    INTERFACE_INCLUDE_DIRECTORIES "${skia_headers}"
    INTERFACE_LINK_LIBRARIES textra_skia_runtime)
