// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_TTTEXT_CONTEXT_H_
#define PUBLIC_TEXTRA_C_TEXTRA_TTTEXT_CONTEXT_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_tttext_context_create(textra_tttext_context_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_tttext_context_destroy(textra_tttext_context_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_tttext_context_reset(textra_tttext_context_t* value) TEXTRA_NOEXCEPT;

/* Enable or disable one FeatureOption (not a bit mask). Reset preserves all
 * feature settings. Configure platform-specific features before layout. */
TEXTRA_API void TEXTRA_CALL textra_tttext_context_enable_feature(
    textra_tttext_context_t* context, textra_feature_option_t feature,
    uint32_t enabled) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_tttext_context_set_layout_position(
    const textra_tttext_context_t* context, uint32_t run_index,
    uint32_t char_index_in_run) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_tttext_context_get_layout_position(
    const textra_tttext_context_t* context,
    textra_layout_position_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_tttext_context_get_layout_bottom(
    const textra_tttext_context_t* context) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_tttext_context_set_layout_bottom(
    textra_tttext_context_t* context, float bottom) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_TTTEXT_CONTEXT_H_
