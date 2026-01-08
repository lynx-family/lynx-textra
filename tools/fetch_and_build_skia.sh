#!/bin/bash
# Copyright 2021 The Lynx Authors. All rights reserved.

git clone --branch chrome/m130 --depth 1 https://skia.googlesource.com/skia.git third_party/skia
pushd third_party/skia
GIT_SYNC_DEPS_SKIP_EMSDK=true python3 tools/git-sync-deps
gn gen out/Static --args='cc="clang" cxx="clang++" is_debug=false is_trivial_abi=false skia_enable_fontmgr_custom_directory=true skia_use_fontconfig=false skia_enable_gpu=false skia_use_freetype=true skia_use_system_freetype2=false skia_enable_skottie=false skia_enable_pdf=false'
ninja -C out/Static
popd