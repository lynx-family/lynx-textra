// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_FONT_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_FONT_H_

#include <textra/capi/textra_platform.h>

/* Frozen record: four printable ASCII tag bytes, without a NUL terminator,
 * and the signed feature value used by FontDescriptor. */
typedef struct textra_font_feature {
  char tag[4];
  int32_t value;
} textra_font_feature_t;

TEXTRA_EXTERN_C_BEGIN

/* Descriptor families retain insertion order. literal=true requests a literal
 * family, false allows the backend's generic-family policy. Explicit typeface
 * overrides family matching and is retained. Default style is 400/5/upright. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_descriptor_create(textra_font_descriptor_t** out_descriptor);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_descriptor_clone(const textra_font_descriptor_t* source,
                             textra_font_descriptor_t** out_font_descriptor);
TEXTRA_API void TEXTRA_CALL
textra_font_descriptor_destroy(textra_font_descriptor_t* font_descriptor);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_descriptor_clear_families(textra_font_descriptor_t* descriptor);
TEXTRA_API textra_status_t TEXTRA_CALL textra_font_descriptor_add_family(
    textra_font_descriptor_t* descriptor, const char* utf8, uint32_t byte_count,
    textra_bool_t literal);
TEXTRA_API textra_status_t TEXTRA_CALL textra_font_descriptor_get_family_count(
    const textra_font_descriptor_t* descriptor, uint32_t* out_count);
TEXTRA_API textra_status_t TEXTRA_CALL textra_font_descriptor_copy_family(
    const textra_font_descriptor_t* descriptor, uint32_t index, char* buffer,
    uint32_t capacity, uint32_t* out_size, textra_bool_t* out_literal);
TEXTRA_API textra_status_t TEXTRA_CALL textra_font_descriptor_set_font_style(
    textra_font_descriptor_t* descriptor, const textra_font_style_t* style);
TEXTRA_API textra_status_t TEXTRA_CALL textra_font_descriptor_get_font_style(
    const textra_font_descriptor_t* descriptor, textra_font_style_t* out_style);
/* Replace all OpenType feature settings with a copied list. NULL/zero clears
 * the list and restores backend defaults. Order, duplicate tags and signed
 * values are preserved, matching C++ FontDescriptor. Unknown tags follow the
 * backend/font's behavior. Invalid input leaves the descriptor unchanged.
 * Changes affect styles only after set_font_descriptor is called again. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_font_descriptor_set_font_features(
    textra_font_descriptor_t* descriptor, const textra_font_feature_t* features,
    uint32_t count);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_descriptor_get_font_feature_count(
    const textra_font_descriptor_t* descriptor, uint32_t* out_count);
/* Copies one frozen record; an invalid index leaves out_feature unchanged. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_font_descriptor_get_font_feature(
    const textra_font_descriptor_t* descriptor, uint32_t index,
    textra_font_feature_t* out_feature);
/* NULL clears the explicit face. Native and callback ownership follows
 * textra_platform.h. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_font_descriptor_set_typeface(textra_font_descriptor_t* descriptor,
                                    const textra_typeface_options_t* options);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_FONT_H_
