// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_PAGE_LISTENER_H_
#define PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_PAGE_LISTENER_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

typedef void(TEXTRA_CALL* textra_line_layouted_callback_t)(
    void* user_data, textra_layout_region_t* region, uint32_t line_index,
    uint32_t last_line_in_paragraph, float available_height,
    textra_layout_result_t* result);
/* Region is borrowed during the callback. The region borrows this listener,
 * which must outlive the region. */

TEXTRA_API textra_status_t TEXTRA_CALL textra_layout_page_listener_create(
    void* user_data, textra_destroy_callback_t destroy,
    textra_layout_page_listener_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_layout_page_listener_destroy(
    textra_layout_page_listener_t* listener) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_layout_page_listener_set_on_line_layouted_callback(
    textra_layout_page_listener_t* listener,
    textra_line_layouted_callback_t callback) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_LAYOUT_PAGE_LISTENER_H_
