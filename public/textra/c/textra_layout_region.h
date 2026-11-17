// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_REGION_H_
#define PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_REGION_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL textra_layout_region_create(
    float width, float height, textra_layout_mode_t width_mode,
    textra_layout_mode_t height_mode, textra_layout_page_listener_t* listener,
    textra_layout_region_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_layout_region_destroy(textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_layout_region_get_page_width(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_layout_region_get_page_height(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_layout_region_get_layouted_width(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_layout_region_get_layouted_height(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_layout_region_get_line_count(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API textra_layout_mode_t TEXTRA_CALL textra_layout_region_get_width_mode(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API textra_layout_mode_t TEXTRA_CALL
textra_layout_region_get_height_mode(const textra_layout_region_t* region)
    TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_layout_region_did_exceed_max_lines(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_layout_region_is_full_filled(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_layout_region_is_empty(
    const textra_layout_region_t* region) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_layout_region_get_line_pitch(
    const textra_layout_region_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_layout_region_set_line_pitch(
    textra_layout_region_t* value, float input) TEXTRA_NOEXCEPT;

/* Borrowed; out-of-range returns NULL. */
TEXTRA_API textra_text_line_t* TEXTRA_CALL textra_layout_region_get_line(
    const textra_layout_region_t* region, uint32_t line_index) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_layout_region_update_layouted_size(
    textra_layout_region_t* region, textra_text_line_t* line,
    const textra_tttext_context_t* context) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_REGION_H_
