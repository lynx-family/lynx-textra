// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_DRAWER_H_
#define PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_DRAWER_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_layout_drawer_create(textra_canvas_helper_t* canvas,
                            textra_layout_drawer_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_layout_drawer_destroy(textra_layout_drawer_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_layout_drawer_draw_layout_page(
    textra_layout_drawer_t* drawer,
    textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_layout_drawer_draw_text_line(
    textra_layout_drawer_t* drawer, textra_text_line_t* line,
    uint32_t start_char_in_line, uint32_t end_char_in_line) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_layout_drawer_draw_single_line(
    textra_layout_drawer_t* drawer, const textra_paragraph_t* paragraph,
    float x, float y) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_DRAWER_H_
