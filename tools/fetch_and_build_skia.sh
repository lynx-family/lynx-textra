#!/bin/bash
# Copyright 2024 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

set -e

# checking arguments
if [[ $# -lt 1 ]]; then
    echo "usage: $0 <target_directory>"
    echo "example: $0 /path/to/output"
    exit 1
fi

TARGET_DIR="$1"

echo "target directory: $TARGET_DIR"

git clone --branch chrome/m130 --depth 1 https://skia.googlesource.com/skia.git third_party/skia
pushd third_party/skia
GIT_SYNC_DEPS_SKIP_EMSDK=true python3 tools/git-sync-deps
gn gen out/Static --args='cc="gcc" cxx="g++" is_debug=false is_trivial_abi=false skia_enable_fontmgr_custom_directory=true skia_use_fontconfig=false skia_enable_gpu=false skia_use_freetype=true skia_use_system_freetype2=false skia_enable_skottie=false skia_enable_pdf=false'
ninja -C out/Static
popd

mkdir -p "$TARGET_DIR"

find third_party/skia/out/Static -name "*.a" -exec cp {} "$TARGET_DIR/" \;