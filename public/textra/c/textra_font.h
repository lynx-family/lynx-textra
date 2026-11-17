// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_FONT_H_
#define PUBLIC_TEXTRA_C_TEXTRA_FONT_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

/* Empty callback adapters. Callback/fallback setters only accept these adapters.
 * Each method uses its callback when configured, otherwise its fallback object.
 * Callback results (including NULL/0) are final. With neither, values/outputs
 * are zero/NULL. Fallback objects are shared; callers may release their
 * handles. NULL clears a callback/fallback. Fallback chains must be acyclic.
 * user_data is destroyed once at final C++ object destruction.
 * Callback parameters are borrowed for the call; do not retain, release or
 * store shared handles passed to callbacks. Typeface callback return
 * values transfer one owned C reference; retain a stored handle before
 * returning it. Returned raw font data must remain valid while shaping uses the
 * typeface. Configure before layout; do not reconfigure during an active call.
 */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_manager_create(void* user_data, textra_destroy_callback_t destroy,
                           textra_font_manager_t** out) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_manager_set_fallback(
    textra_font_manager_t* value,
    textra_font_manager_t* fallback) TEXTRA_NOEXCEPT;

typedef int32_t(TEXTRA_CALL* textra_font_manager_count_families_callback_t)(
    void* user_data);
typedef textra_typeface_helper_t*(
    TEXTRA_CALL* textra_font_manager_match_family_style_callback_t)(
    void* user_data, const char* family, textra_font_style_t style);
typedef textra_typeface_helper_t*(
    TEXTRA_CALL* textra_font_manager_match_family_style_character_callback_t)(
    void* user_data, const char* family, textra_font_style_t style,
    const char* const* locales, int32_t locale_count, uint32_t character);
typedef textra_typeface_helper_t*(
    TEXTRA_CALL* textra_font_manager_make_from_file_callback_t)(
    void* user_data, const char* path, int32_t ttc_index);
typedef textra_typeface_helper_t*(
    TEXTRA_CALL* textra_font_manager_legacy_make_typeface_callback_t)(
    void* user_data, const char* family, textra_font_style_t style);
typedef textra_typeface_helper_t*(
    TEXTRA_CALL*
        textra_font_manager_create_typeface_from_platform_font_callback_t)(
    void* user_data, const void* platform_font);
typedef void*(
    TEXTRA_CALL*
        textra_font_manager_get_platform_font_from_typeface_callback_t)(
    void* user_data, textra_typeface_helper_t* typeface);
typedef textra_typeface_helper_t*(
    TEXTRA_CALL* textra_font_manager_match_generic_family_callback_t)(
    void* user_data, const char* family, textra_font_style_t style);
typedef textra_typeface_helper_t*(
    TEXTRA_CALL* textra_font_manager_match_family_style_literal_callback_t)(
    void* user_data, const char* family, textra_font_style_t style);
typedef textra_typeface_helper_t*(
    TEXTRA_CALL* textra_font_manager_legacy_make_typeface_literal_callback_t)(
    void* user_data, const char* family, textra_font_style_t style);

TEXTRA_API void TEXTRA_CALL textra_font_manager_set_count_families_callback(
    textra_font_manager_t* value,
    textra_font_manager_count_families_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_manager_set_match_family_style_callback(
    textra_font_manager_t* value,
    textra_font_manager_match_family_style_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_font_manager_set_match_family_style_character_callback(
    textra_font_manager_t* value,
    textra_font_manager_match_family_style_character_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_manager_set_make_from_file_callback(
    textra_font_manager_t* value,
    textra_font_manager_make_from_file_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_font_manager_set_legacy_make_typeface_callback(
    textra_font_manager_t* value,
    textra_font_manager_legacy_make_typeface_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_font_manager_set_create_typeface_from_platform_font_callback(
    textra_font_manager_t* value,
    textra_font_manager_create_typeface_from_platform_font_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_font_manager_set_get_platform_font_from_typeface_callback(
    textra_font_manager_t* value,
    textra_font_manager_get_platform_font_from_typeface_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_font_manager_set_match_generic_family_callback(
    textra_font_manager_t* value,
    textra_font_manager_match_generic_family_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_font_manager_set_match_family_style_literal_callback(
    textra_font_manager_t* value,
    textra_font_manager_match_family_style_literal_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_font_manager_set_legacy_make_typeface_literal_callback(
    textra_font_manager_t* value,
    textra_font_manager_legacy_make_typeface_literal_callback_t callback)
    TEXTRA_NOEXCEPT;

/* unique_id and style identify this typeface, independently of fallback.
 * IDs must be unique among the typefaces used by the same shaper/backend.
 * Updating fallback or metric callbacks invalidates borrowed cached metrics.
 * Metric callbacks may only write their temporary out object. Without an
 * em-height callback/fallback, em height is derived from this adapter's
 * metrics. Internal font data/index/units callbacks serve C++ shapers; they do
 * not add separate C query entry points. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_typeface_helper_create(
    uint32_t unique_id, textra_font_style_t style, void* user_data,
    textra_destroy_callback_t destroy,
    textra_typeface_helper_t** out) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_typeface_helper_set_fallback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_t* fallback) TEXTRA_NOEXCEPT;

typedef float(
    TEXTRA_CALL* textra_typeface_helper_get_horizontal_advance_callback_t)(
    void* user_data, uint16_t glyph, float size);
typedef void(
    TEXTRA_CALL* textra_typeface_helper_get_horizontal_advances_callback_t)(
    void* user_data, uint16_t* glyphs, uint32_t count, float* widths,
    float size);
typedef void(TEXTRA_CALL* textra_typeface_helper_get_width_bound_callback_t)(
    void* user_data, uint16_t glyph, float size, textra_rect_t* out);
typedef void(TEXTRA_CALL* textra_typeface_helper_get_width_bounds_callback_t)(
    void* user_data, uint16_t* glyphs, uint32_t count, float size,
    textra_bounds_t* out);
typedef uint16_t(
    TEXTRA_CALL* textra_typeface_helper_unichar_to_glyph_callback_t)(
    void* user_data, uint32_t codepoint, uint32_t variation_selector);
typedef void(TEXTRA_CALL* textra_typeface_helper_unichars_to_glyphs_callback_t)(
    void* user_data, const uint32_t* codepoints, uint32_t count,
    uint16_t* glyphs);
typedef const void*(
    TEXTRA_CALL* textra_typeface_helper_get_font_data_callback_t)(
    void* user_data);
typedef size_t(
    TEXTRA_CALL* textra_typeface_helper_get_font_data_size_callback_t)(
    void* user_data);
typedef int32_t(TEXTRA_CALL* textra_typeface_helper_get_font_index_callback_t)(
    void* user_data);
typedef uint32_t(
    TEXTRA_CALL* textra_typeface_helper_get_units_per_em_callback_t)(
    void* user_data);
typedef void(TEXTRA_CALL* textra_typeface_helper_create_font_info_callback_t)(
    void* user_data, textra_font_info_t* out, float size);
typedef void(
    TEXTRA_CALL* textra_typeface_helper_create_font_em_height_callback_t)(
    void* user_data, textra_font_em_height_t* out, float size);

TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_get_horizontal_advance_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_horizontal_advance_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_get_horizontal_advances_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_horizontal_advances_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_typeface_helper_set_get_width_bound_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_width_bound_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_get_width_bounds_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_width_bounds_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_unichar_to_glyph_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_unichar_to_glyph_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_unichars_to_glyphs_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_unichars_to_glyphs_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_typeface_helper_set_get_font_data_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_font_data_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_get_font_data_size_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_font_data_size_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_typeface_helper_set_get_font_index_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_font_index_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_get_units_per_em_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_units_per_em_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_create_font_info_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_create_font_info_callback_t callback)
    TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_set_create_font_em_height_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_create_font_em_height_callback_t callback)
    TEXTRA_NOEXCEPT;

/* Only write metric objects supplied as callback output parameters.
 * Objects returned by get_font_info/get_font_em_height remain read-only. */
TEXTRA_API void TEXTRA_CALL textra_font_info_set_top(
    textra_font_info_t* value, float input) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_info_set_ascent(
    textra_font_info_t* value, float input) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_info_set_descent(
    textra_font_info_t* value, float input) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_info_set_bottom(
    textra_font_info_t* value, float input) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_info_set_leading(
    textra_font_info_t* value, float input) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_info_set_font_size(
    textra_font_info_t* value, float input) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_em_height_set_ascent(
    textra_font_em_height_t* value, float input) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_font_em_height_set_descent(
    textra_font_em_height_t* value, float input) TEXTRA_NOEXCEPT;

/* Typeface operations retain the native backend's support and preconditions.
 * Some platform typefaces do not implement per-glyph metric operations.
 * Cached metric objects are borrowed and read-only. */

TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_descriptor_create(textra_font_descriptor_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_font_descriptor_destroy(textra_font_descriptor_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_font_descriptor_set_font_style(
    textra_font_descriptor_t* descriptor,
    textra_font_style_t style) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_font_descriptor_get_font_style(
    const textra_font_descriptor_t* descriptor,
    textra_font_style_t* out) TEXTRA_NOEXCEPT;

/* Native font is borrowed; use the matching platform font manager contract. */
TEXTRA_API void TEXTRA_CALL textra_font_descriptor_set_platform_font(
    textra_font_descriptor_t* descriptor,
    uint64_t platform_font) TEXTRA_NOEXCEPT;

TEXTRA_API uint64_t TEXTRA_CALL textra_font_descriptor_get_platform_font(
    const textra_font_descriptor_t* descriptor) TEXTRA_NOEXCEPT;

/* Strings are copied. literals may be NULL for legacy matching. */
TEXTRA_API void TEXTRA_CALL textra_font_descriptor_set_font_families(
    textra_font_descriptor_t* descriptor, const char* const* families,
    const uint8_t* literals, size_t count) TEXTRA_NOEXCEPT;

TEXTRA_API size_t TEXTRA_CALL textra_font_descriptor_get_font_family_count(
    const textra_font_descriptor_t* descriptor) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_font_descriptor_get_font_family(
    const textra_font_descriptor_t* descriptor, size_t index, char* buffer,
    size_t capacity, size_t* required_bytes) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_descriptor_get_font_family_literal(
    const textra_font_descriptor_t* descriptor, size_t index,
    uint32_t* has_literal, uint32_t* literal) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_fontmgr_collection_create(
    textra_font_manager_t* default_manager,
    textra_fontmgr_collection_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_fontmgr_collection_destroy(
    textra_fontmgr_collection_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_fontmgr_collection_set_default_font_manager(
    textra_fontmgr_collection_t* collection,
    textra_font_manager_t* manager) TEXTRA_NOEXCEPT;

/* Returns an owned shared handle, or NULL when unset. */
TEXTRA_API textra_font_manager_t* TEXTRA_CALL
textra_fontmgr_collection_get_default_font_manager(
    const textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_fontmgr_collection_set_asset_font_manager(
    textra_fontmgr_collection_t* collection,
    textra_font_manager_t* manager) TEXTRA_NOEXCEPT;

/* Returns an owned shared handle, or NULL when unset. */
TEXTRA_API textra_font_manager_t* TEXTRA_CALL
textra_fontmgr_collection_get_asset_font_manager(
    const textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_fontmgr_collection_set_dynamic_font_manager(
    textra_fontmgr_collection_t* collection,
    textra_font_manager_t* manager) TEXTRA_NOEXCEPT;

/* Returns an owned shared handle, or NULL when unset. */
TEXTRA_API textra_font_manager_t* TEXTRA_CALL
textra_fontmgr_collection_get_dynamic_font_manager(
    const textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_fontmgr_collection_clear_caches(
    textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_fontmgr_collection_enable_font_fallback(
    textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_fontmgr_collection_disable_font_fallback(
    textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_fontmgr_collection_font_fallback_enabled(
    const textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_fontmgr_collection_set_default_system_font_family(
    textra_fontmgr_collection_t* collection,
    const char* family) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_font_manager_retain(textra_font_manager_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_font_manager_release(textra_font_manager_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API int32_t TEXTRA_CALL textra_font_manager_count_families(
    const textra_font_manager_t* manager) TEXTRA_NOEXCEPT;

/* Owned shared result, or NULL on no match; family may be NULL. */
TEXTRA_API textra_typeface_helper_t* TEXTRA_CALL
textra_font_manager_match_family_style(
    textra_font_manager_t* manager, const char* family,
    textra_font_style_t style) TEXTRA_NOEXCEPT;

TEXTRA_API textra_typeface_helper_t* TEXTRA_CALL
textra_font_manager_make_from_file(textra_font_manager_t* manager,
                                   const char* path,
                                   int32_t ttc_index) TEXTRA_NOEXCEPT;

TEXTRA_API textra_typeface_helper_t* TEXTRA_CALL
textra_font_manager_create_typeface_from_platform_font(
    textra_font_manager_t* manager, const void* platform_font) TEXTRA_NOEXCEPT;

/* Native result follows this manager's C++ ownership contract. */
TEXTRA_API void* TEXTRA_CALL
textra_font_manager_get_platform_font_from_typeface(
    textra_font_manager_t* manager,
    textra_typeface_helper_t* typeface) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_retain(textra_typeface_helper_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_release(textra_typeface_helper_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_font_descriptor_assign(
    textra_font_descriptor_t* value,
    const textra_font_descriptor_t* source) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_descriptor_copy(const textra_font_descriptor_t* source,
                            textra_font_descriptor_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_font_descriptor_equals(
    const textra_font_descriptor_t* value,
    const textra_font_descriptor_t* other) TEXTRA_NOEXCEPT;

/* Owned shared match, or NULL. */
TEXTRA_API textra_typeface_helper_t* TEXTRA_CALL
textra_font_manager_legacy_make_typeface(
    textra_font_manager_t* manager, const char* family,
    textra_font_style_t style) TEXTRA_NOEXCEPT;

TEXTRA_API textra_typeface_helper_t* TEXTRA_CALL
textra_font_manager_match_family_style_character(
    textra_font_manager_t* manager, const char* family,
    textra_font_style_t style, const char** bcp47, int32_t bcp47_count,
    uint32_t character) TEXTRA_NOEXCEPT;

TEXTRA_API size_t TEXTRA_CALL textra_fontmgr_collection_get_font_managers_count(
    const textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

/* Each written result owns one shared reference. No writes on insufficient
 * capacity. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_fontmgr_collection_find_typefaces(
    const textra_fontmgr_collection_t* collection,
    const textra_font_descriptor_t* descriptor,
    textra_typeface_helper_t** typefaces, size_t capacity,
    size_t* required_count) TEXTRA_NOEXCEPT;

TEXTRA_API textra_typeface_helper_t* TEXTRA_CALL
textra_fontmgr_collection_default_fallback(
    const textra_fontmgr_collection_t* collection) TEXTRA_NOEXCEPT;

TEXTRA_API textra_typeface_helper_t* TEXTRA_CALL
textra_fontmgr_collection_default_fallback_character(
    const textra_fontmgr_collection_t* collection, uint32_t character,
    textra_font_style_t style, const char* locale) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_typeface_helper_get_horizontal_advance(
    const textra_typeface_helper_t* typeface, uint16_t glyph,
    float size) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_typeface_helper_get_horizontal_advances(
    const textra_typeface_helper_t* typeface, uint16_t* glyphs, uint32_t count,
    float* widths, float size) TEXTRA_NOEXCEPT;

TEXTRA_API uint16_t TEXTRA_CALL textra_typeface_helper_unichar_to_glyph(
    const textra_typeface_helper_t* typeface, uint32_t codepoint,
    uint32_t variation_selector) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_typeface_helper_unichars_to_glyphs(
    const textra_typeface_helper_t* typeface, const uint32_t* codepoints,
    uint32_t count, uint16_t* glyphs) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_typeface_helper_get_unique_id(
    const textra_typeface_helper_t* typeface) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_typeface_helper_get_width_bound(
    const textra_typeface_helper_t* typeface, uint16_t glyph, float size,
    textra_rect_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_typeface_helper_get_width_bounds(
    textra_typeface_helper_t* typeface, uint16_t* glyphs, uint32_t count,
    float size, textra_bounds_t* out) TEXTRA_NOEXCEPT;

/* Borrowed cached metrics; do not destroy or modify. */
TEXTRA_API const textra_font_info_t* TEXTRA_CALL
textra_typeface_helper_get_font_info(textra_typeface_helper_t* typeface,
                                     float size) TEXTRA_NOEXCEPT;

/* Borrowed cached metrics; do not destroy or modify. */
TEXTRA_API const textra_font_em_height_t* TEXTRA_CALL
textra_typeface_helper_get_font_em_height(textra_typeface_helper_t* typeface,
                                          float size) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_typeface_helper_get_font_style(const textra_typeface_helper_t* typeface,
                                      textra_font_style_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_font_info_get_top(const textra_font_info_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_font_info_get_ascent(const textra_font_info_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_font_info_get_descent(const textra_font_info_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_font_info_get_bottom(const textra_font_info_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_font_info_get_leading(const textra_font_info_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_font_info_get_font_size(const textra_font_info_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_font_info_get_height(const textra_font_info_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_font_em_height_get_ascent(
    const textra_font_em_height_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_font_em_height_get_descent(
    const textra_font_em_height_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API textra_font_style_t TEXTRA_CALL textra_font_style_normal(void)
    TEXTRA_NOEXCEPT;

TEXTRA_API textra_font_style_t TEXTRA_CALL textra_font_style_bold(void)
    TEXTRA_NOEXCEPT;

TEXTRA_API textra_font_style_t TEXTRA_CALL textra_font_style_italic(void)
    TEXTRA_NOEXCEPT;

TEXTRA_API textra_font_style_t TEXTRA_CALL textra_font_style_bold_italic(void)
    TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_FONT_H_
