# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0.

# The parent provides skia/skity (possibly IMPORTED) targets with their SDK
# include paths and link dependencies. These adapters never link Textra core.
get_filename_component(textra_source_root "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)

if(TARGET skia)
    add_library(textra_capi_skia INTERFACE)
    target_include_directories(textra_capi_skia INTERFACE ${textra_source_root}/public)
    target_compile_features(textra_capi_skia INTERFACE cxx_std_17)
    if(MSVC)
        target_compile_options(textra_capi_skia INTERFACE /EHs-c-)
    else()
        target_compile_options(textra_capi_skia INTERFACE -fno-exceptions)
    endif()
    target_link_libraries(textra_capi_skia INTERFACE skia)
endif()

# Header-only C adapters: carry SDK requirements without building an archive.
if(TARGET skity)
    foreach(name textra_capi_skity textra_capi_skity_coretext)
        if(name STREQUAL "textra_capi_skity_coretext" AND NOT APPLE)
            continue()
        endif()
        add_library(${name} INTERFACE)
        target_include_directories(${name} INTERFACE
            ${textra_source_root} ${textra_source_root}/public
            ${CMAKE_CURRENT_SOURCE_DIR})
        target_compile_features(${name} INTERFACE c_std_11)
        target_link_libraries(${name} INTERFACE skity)
    endforeach()
endif()
