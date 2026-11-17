// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_TEXT_LAYOUT_H_
#define PUBLIC_TEXTRA_C_TEXTRA_TEXT_LAYOUT_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL textra_text_layout_create(
    textra_fontmgr_collection_t* collection, textra_shaper_type_t type,
    textra_text_layout_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_text_layout_destroy(textra_text_layout_t* layout) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_text_layout_preload(textra_shaper_type_t type) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_text_layout_clear_shape_cache(void)
    TEXTRA_NOEXCEPT;

TEXTRA_API textra_layout_result_t TEXTRA_CALL textra_text_layout_layout_ex(
    const textra_text_layout_t* layout, textra_paragraph_t* paragraph,
    textra_layout_region_t* region,
    textra_tttext_context_t* context) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_text_layout_layout_single_line(
    const textra_text_layout_t* layout, textra_paragraph_t* paragraph,
    textra_tttext_context_t* context) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_TEXT_LAYOUT_H_
