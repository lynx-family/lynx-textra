// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_CONTEXT_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_CONTEXT_H_

#include <textra/capi/textra_types.h>

/* Context contains per-operation configuration and continuation state.
 * Configure features before first use; changing features while continuation
 * state/results exist returns BUSY. Create a fresh context/engine for a new
 * shaping policy. reset clears continuation/bottom, preserving configuration.
 * Default features: LAST_LINE_CAN_OVERFLOW and TRIM_LINE_TAIL_SPACE are true;
 * every other feature is false.
 * Unsupported platform features return UNSUPPORTED. Feature IDs are stable C
 * values, not internal FeatureOption ordinals.
 *
 * Continuation positions are opaque and bound to an unmodified paragraph and
 * engine. Do not expose run indices, which can change during internal layout.
 * The context retains dependencies during continuation. reset releases them.
 */
TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_text_context_create(textra_text_context_t** out_context);
TEXTRA_API void TEXTRA_CALL
textra_text_context_destroy(textra_text_context_t* context);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_text_context_reset(textra_text_context_t* context);

/* A new context has no cache. Pass TEXTRA_FALSE for single-threaded use or
 * TEXTRA_TRUE to share a cache across threads. Contexts still require
 * external serialization. Share only with matching fonts, shaper and shaping
 * settings. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_shaper_cache_create(
    textra_bool_t thread_safe, textra_shaper_cache_t** out_cache);
TEXTRA_API void TEXTRA_CALL
textra_shaper_cache_destroy(textra_shaper_cache_t* cache);
/* The context retains the cache across reset. Pass NULL to detach it.
 * Configure before layout, or after reset and release of outstanding results.
 * Changing the binding while the context is in use returns BUSY. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_context_set_shaper_cache(
    textra_text_context_t* context, const textra_shaper_cache_t* cache);
/* Returns an owned reference, or NOT_FOUND and NULL for an unbound context. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_context_get_shaper_cache(
    const textra_text_context_t* context, textra_shaper_cache_t** out_cache);
/* Clear shaping results or platform font entries independently for every
 * context sharing this cache. Both succeed when no cache is attached. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_text_context_clear_shape_cache(textra_text_context_t* context);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_text_context_clear_font_cache(textra_text_context_t* context);

TEXTRA_API textra_status_t TEXTRA_CALL textra_text_context_set_feature(
    textra_text_context_t* context, textra_feature_t feature,
    textra_bool_t enabled);
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_context_get_feature(
    const textra_text_context_t* context, textra_feature_t feature,
    textra_bool_t* out_enabled);
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_context_get_position(
    const textra_text_context_t* context, uint32_t* out_codepoint_offset);
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_context_get_layout_bottom(
    const textra_text_context_t* context, float* out_bottom);
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_context_set_layout_bottom(
    textra_text_context_t* context, float bottom);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_CONTEXT_H_
