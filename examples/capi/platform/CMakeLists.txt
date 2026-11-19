# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0.

# The parent provides skia/skity (possibly IMPORTED) targets with their SDK
# include paths and link dependencies. These adapters never link Textra core.
get_filename_component(textra_source_root "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

function(textra_add_host_adapter name source)
    add_library(${name} STATIC ${source})
    target_include_directories(${name} PUBLIC
        ${textra_source_root} ${textra_source_root}/public)
    target_compile_features(${name} PRIVATE cxx_std_17)
    set_target_properties(${name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    if(MSVC)
        target_compile_options(${name} PRIVATE /EHs-c-)
    else()
        target_compile_options(${name} PRIVATE -fno-exceptions)
    endif()
endfunction()

if(TARGET skia)
    textra_add_host_adapter(textra_capi_skia skia/textra_skia_backend.cc)
    target_link_libraries(textra_capi_skia PUBLIC skia)
endif()

if(TARGET skity)
    textra_add_host_adapter(textra_capi_skity skity/textra_skity_backend.cc)
    target_link_libraries(textra_capi_skity PUBLIC skity)
    if(APPLE)
        textra_add_host_adapter(textra_capi_skity_coretext skity_coretext/textra_skity_coretext_backend.cc)
        target_link_libraries(textra_capi_skity_coretext PUBLIC
            skity "-framework CoreText")
    endif()
endif()
