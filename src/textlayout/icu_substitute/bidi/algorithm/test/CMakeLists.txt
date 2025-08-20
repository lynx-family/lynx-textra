# Copyright 2021 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

project(biditest)

include_directories(../../bidi/)
include_directories(../../textlayout/bidi_wrapper/)
add_executable(test
        main.cpp
        ../bidi.cc
        ../bidi_brackets.cc
        ../bidi_line.cc
        ../bidi_mirror.cc
        ../bidi_writer.cc
        ../bidi_character.cc
        ../u_character.h
        ../ucharacter_enums.h
        ../../textlayout/bidi_wrapper/bidi_utils.cc
        ../../textlayout/utils/u_8_string.cc
        )

target_compile_options(test PRIVATE ${ASAN_COMPILE_OPTION})
target_link_options(test PRIVATE ${ASAN_COMPILE_OPTION})
