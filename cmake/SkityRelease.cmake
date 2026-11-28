# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0.

get_filename_component(textra_skity_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(skity_release_dir "${textra_skity_root}/third_party/skity_release/apple")
if(NOT EXISTS "${skity_release_dir}/skity.xcframework/macos-arm64_x86_64/skity.framework/Headers/c/skity.h")
    message(FATAL_ERROR "Run tools/hab sync . --target-only --target skity-macos before configuring Skity tests/demos")
endif()

set(skity_framework_dir "${skity_release_dir}/skity.xcframework/macos-arm64_x86_64")
add_library(skity STATIC IMPORTED GLOBAL)
set_target_properties(skity PROPERTIES
    IMPORTED_LOCATION "${skity_framework_dir}/skity.framework/Versions/A/skity"
    INTERFACE_COMPILE_OPTIONS "-F${skity_framework_dir};-mmacosx-version-min=15.0"
    INTERFACE_LINK_OPTIONS "-mmacosx-version-min=15.0"
    INTERFACE_LINK_LIBRARIES "-framework CoreText;-framework CoreGraphics;-framework CoreServices;-framework Foundation;-framework ImageIO;-framework AppKit;-framework OpenGL;-framework Metal"
)
