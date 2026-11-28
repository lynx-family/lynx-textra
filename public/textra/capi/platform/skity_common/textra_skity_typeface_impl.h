// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_TYPEFACE_IMPL_H_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_TYPEFACE_IMPL_H_

#include <textra/capi/platform/skity_common/textra_skity_common.h>

TEXTRA_EXTERN_C_BEGIN
typedef struct textra_skity_impl_typeface {
  skity_typeface font;
  skity_data data;
  int32_t ascent, descent;
} textra_skity_impl_typeface_t;

static inline skity_font textra_skity_impl_make_font(skity_typeface face,
                                                     float font_size) {
  skity_font font = skity_font_create_with_typeface(face, font_size);
  if (font) skity_font_set_hinting(font, SKITY_FONT_HINTING_SLIGHT);
  return font;
}
static inline textra_status_t textra_skity_impl_wrap_typeface(
    skity_typeface native_typeface, void** out_typeface) {
  textra_skity_impl_typeface_t* face;
  uint8_t metrics[4];
  *out_typeface = NULL;
  if (!native_typeface) return TEXTRA_STATUS_OK;
  face = (textra_skity_impl_typeface_t*)calloc(1, sizeof(*face));
  if (!face) {
    skity_typeface_destroy(native_typeface);
    return TEXTRA_STATUS_OUT_OF_MEMORY;
  }
  face->font = native_typeface;
  /* Immutable after publication: data and metrics remain valid until destroy.
   */
  face->data = skity_typeface_get_data(native_typeface);
  if (skity_typeface_get_table_data(native_typeface, 0x4f532f32u, 68,
                                    sizeof(metrics),
                                    metrics) == sizeof(metrics)) {
    uint16_t ascent_value = (uint16_t)((metrics[0] << 8) | metrics[1]);
    uint16_t descent_value = (uint16_t)((metrics[2] << 8) | metrics[3]);
    face->ascent =
        ascent_value < 0x8000u ? ascent_value : (int32_t)ascent_value - 65536;
    face->descent = -(descent_value < 0x8000u ? descent_value
                                              : (int32_t)descent_value - 65536);
  }
  *out_typeface = face;
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_import_typeface(
    void* userdata, const void* native_typeface, void** out_typeface) {
  skity_font font;
  skity_typeface face;
  (void)userdata;
  *out_typeface = NULL;
  if (!native_typeface) return TEXTRA_STATUS_INVALID_ARGUMENT;
  /* The font getter returns an owning reference to the borrowed native face. */
  font = skity_font_create_with_typeface((skity_typeface)native_typeface, 1.f);
  if (!font) return TEXTRA_STATUS_OUT_OF_MEMORY;
  face = skity_font_get_typeface(font);
  skity_font_destroy(font);
  return textra_skity_impl_wrap_typeface(face, out_typeface);
}
static inline void TEXTRA_CALL textra_skity_impl_destroy_face(void* userdata,
                                                              void* typeface) {
  textra_skity_impl_typeface_t* face = (textra_skity_impl_typeface_t*)typeface;
  (void)userdata;
  if (!face) return;
  if (face->data) skity_data_destroy(face->data);
  skity_typeface_destroy(face->font);
  free(face);
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_get_native_typeface(
    void* userdata, void* typeface, const void** native_typeface) {
  (void)userdata;
  *native_typeface = ((textra_skity_impl_typeface_t*)typeface)->font;
  return TEXTRA_STATUS_OK;
}
static inline skity_font textra_skity_impl_make_typeface_font(void* typeface,
                                                              float font_size) {
  return textra_skity_impl_make_font(
      ((textra_skity_impl_typeface_t*)typeface)->font, font_size);
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_get_horizontal_advance(void* userdata, void* typeface,
                                         uint16_t glyph, float font_size,
                                         float* advance) {
  (void)userdata;
  skity_font font = textra_skity_impl_make_typeface_font(typeface, font_size);
  if (!font) return TEXTRA_STATUS_OUT_OF_MEMORY;
  skity_font_get_widths(font, &glyph, 1, advance);
  skity_font_destroy(font);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_get_horizontal_advances(void* userdata, void* typeface,
                                          const uint16_t* glyphs,
                                          uint32_t glyph_count, float font_size,
                                          float* advances) {
  (void)userdata;
  skity_font font = textra_skity_impl_make_typeface_font(typeface, font_size);
  if (!font) return TEXTRA_STATUS_OUT_OF_MEMORY;
  skity_font_get_widths(font, glyphs, glyph_count, advances);
  skity_font_destroy(font);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_get_glyph_bounds(
    void* userdata, void* typeface, uint16_t glyph, float font_size,
    textra_rect_t* bounds) {
  (void)userdata;
  skity_rect glyph_bounds;
  skity_font font = textra_skity_impl_make_typeface_font(typeface, font_size);
  if (!font) return TEXTRA_STATUS_OUT_OF_MEMORY;
  skity_font_get_bounds(font, &glyph, 1, &glyph_bounds);
  bounds->left = glyph_bounds.left;
  bounds->top = glyph_bounds.top;
  bounds->right = glyph_bounds.right - glyph_bounds.left;
  bounds->bottom = glyph_bounds.bottom - glyph_bounds.top;
  skity_font_destroy(font);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_get_collection_index(void* userdata, void* typeface,
                                       int32_t* collection_index) {
  (void)userdata;
  skity_font_descriptor descriptor;
  memset(&descriptor, 0, sizeof(descriptor));
  skity_typeface_get_font_descriptor(
      ((textra_skity_impl_typeface_t*)typeface)->font, &descriptor, NULL, 0);
  *collection_index = descriptor.collection_index;
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_unichar_to_glyph(
    void* userdata, void* typeface, uint32_t codepoint,
    uint32_t variation_selector, uint16_t* glyph) {
  (void)userdata;
  if (variation_selector) return TEXTRA_STATUS_UNSUPPORTED;
  *glyph = skity_typeface_unichar_to_glyph(
      ((textra_skity_impl_typeface_t*)typeface)->font, codepoint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_unichars_to_glyphs(
    void* userdata, void* typeface, const uint32_t* codepoints,
    uint32_t codepoint_count, uint16_t* glyphs) {
  (void)userdata;
  skity_typeface_unichars_to_glyphs(
      ((textra_skity_impl_typeface_t*)typeface)->font, codepoints,
      (int)(codepoint_count), glyphs);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_get_units_per_em(
    void* userdata, void* typeface, uint32_t* units_per_em) {
  (void)userdata;
  *units_per_em = skity_typeface_get_units_per_em(
      ((textra_skity_impl_typeface_t*)typeface)->font);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_get_font_metrics(
    void* userdata, void* typeface, float font_size,
    textra_backend_font_metrics_t* metrics) {
  (void)userdata;
  skity_font_metrics font_metrics;
  memset(&font_metrics, 0, sizeof(font_metrics));
  skity_font font = textra_skity_impl_make_typeface_font(typeface, font_size);
  if (!font) return TEXTRA_STATUS_OUT_OF_MEMORY;
  skity_font_get_metrics(font, &font_metrics);
  metrics->top = 0;
  metrics->ascent = font_metrics.ascent;
  metrics->descent = font_metrics.descent;
  metrics->bottom = 0;
  metrics->leading = 0;
  metrics->font_size = font_size;
  skity_font_destroy(font);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_get_font_style(
    void* userdata, void* typeface, textra_font_style_t* style) {
  (void)userdata;
  skity_font_style font_style = {0, 0, SKITY_FONT_SLANT_UPRIGHT};
  skity_typeface_get_font_style(((textra_skity_impl_typeface_t*)typeface)->font,
                                &font_style);
  style->weight = (uint16_t)(font_style.weight);
  style->width = (uint8_t)(font_style.width);
  style->slant = (uint8_t)(font_style.slant);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_get_glyphs_bounds(
    void* userdata, void* typeface, const uint16_t* glyphs,
    uint32_t glyph_count, float font_size, textra_rect_t* bounds) {
  skity_font font;
  skity_rect* glyph_bounds;
  float width = 0, top, bottom;
  uint32_t i;
  (void)userdata;
  memset(bounds, 0, sizeof(*bounds));
  if (!glyph_count) return TEXTRA_STATUS_OK;
  glyph_bounds = (skity_rect*)textra_skity_impl_allocate(glyph_count,
                                                         sizeof(*glyph_bounds));
  if (!glyph_bounds) return TEXTRA_STATUS_OUT_OF_MEMORY;
  font = textra_skity_impl_make_typeface_font(typeface, font_size);
  if (!font) {
    free(glyph_bounds);
    return TEXTRA_STATUS_OUT_OF_MEMORY;
  }
  skity_font_get_bounds(font, glyphs, glyph_count, glyph_bounds);
  top = glyph_bounds[0].top;
  bottom = glyph_bounds[0].bottom;
  for (i = 0; i < glyph_count; ++i) {
    width += glyph_bounds[i].right - glyph_bounds[i].left;
    top = fminf(top, glyph_bounds[i].top);
    bottom = fmaxf(bottom, glyph_bounds[i].bottom);
  }
  bounds->left = glyph_bounds[0].left;
  bounds->top = top;
  bounds->right = glyph_bounds[0].left + width;
  bounds->bottom = bottom;
  skity_font_destroy(font);
  free(glyph_bounds);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_get_font_data(
    void* userdata, void* typeface, const void** font_data) {
  skity_data data = ((textra_skity_impl_typeface_t*)typeface)->data;
  (void)userdata;
  *font_data = data ? skity_data_get_data(data) : NULL;
  return data ? TEXTRA_STATUS_OK : TEXTRA_STATUS_UNSUPPORTED;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_get_font_data_size(
    void* userdata, void* typeface, uint64_t* font_data_size) {
  skity_data data = ((textra_skity_impl_typeface_t*)typeface)->data;
  (void)userdata;
  *font_data_size = data ? skity_data_get_size(data) : 0;
  return data ? TEXTRA_STATUS_OK : TEXTRA_STATUS_UNSUPPORTED;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_get_em_height(void* userdata, void* typeface, float font_size,
                                textra_backend_em_height_t* em_height) {
  textra_skity_impl_typeface_t* face = (textra_skity_impl_typeface_t*)typeface;
  float ascent = (float)face->ascent, descent = (float)face->descent;
  (void)userdata;
  if (ascent < 0 || descent < 0 || ascent + descent <= 0 || font_size < 0) {
    skity_font_metrics metrics;
    skity_font font = textra_skity_impl_make_typeface_font(typeface, font_size);
    if (!font) return TEXTRA_STATUS_OUT_OF_MEMORY;
    skity_font_get_metrics(font, &metrics);
    skity_font_destroy(font);
    ascent = -metrics.ascent;
    descent = metrics.descent;
  }
  if (ascent >= 0 && descent >= 0 && ascent + descent > 0 && font_size >= 0) {
    em_height->ascent = ascent * font_size / (ascent + descent);
    em_height->descent = font_size - em_height->ascent;
  } else {
    em_height->ascent = ascent;
    em_height->descent = descent;
  }
  return TEXTRA_STATUS_OK;
}
static const textra_backend_typeface_api_t textra_skity_impl_typeface_api = {
    sizeof(textra_backend_typeface_api_t),
    TEXTRA_TYPEFACE_API_VERSION,
    textra_skity_impl_destroy_face,
    textra_skity_impl_get_horizontal_advance,
    textra_skity_impl_get_horizontal_advances,
    textra_skity_impl_get_glyph_bounds,
    textra_skity_impl_get_glyphs_bounds,
    textra_skity_impl_get_font_data,
    textra_skity_impl_get_font_data_size,
    textra_skity_impl_get_collection_index,
    textra_skity_impl_unichar_to_glyph,
    textra_skity_impl_unichars_to_glyphs,
    textra_skity_impl_get_units_per_em,
    textra_skity_impl_get_font_metrics,
    textra_skity_impl_get_em_height,
    textra_skity_impl_get_font_style,
    textra_skity_impl_get_native_typeface};
TEXTRA_EXTERN_C_END
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_TYPEFACE_IMPL_H_
