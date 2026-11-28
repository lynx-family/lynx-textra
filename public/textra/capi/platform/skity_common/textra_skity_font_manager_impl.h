// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_FONT_MANAGER_IMPL_H_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_FONT_MANAGER_IMPL_H_

#include <textra/capi/platform/skity_common/textra_skity_typeface_impl.h>

TEXTRA_EXTERN_C_BEGIN
typedef struct textra_skity_impl_font_manager {
  skity_font_manager manager;
  int owned;
} textra_skity_impl_font_manager_t;
static inline textra_status_t TEXTRA_CALL textra_skity_impl_create_manager(
    void* userdata, void* native_manager, void** out_manager) {
  textra_skity_impl_font_manager_t* manager_state;
  (void)userdata;
  *out_manager = NULL;
  manager_state =
      (textra_skity_impl_font_manager_t*)calloc(1, sizeof(*manager_state));
  if (!manager_state) return TEXTRA_STATUS_OUT_OF_MEMORY;
  manager_state->owned = native_manager == NULL;
  manager_state->manager = native_manager ? (skity_font_manager)native_manager
                                          : skity_font_manager_ref_default();
  if (!manager_state->manager) {
    free(manager_state);
    return TEXTRA_STATUS_UNSUPPORTED;
  }
  *out_manager = manager_state;
  return TEXTRA_STATUS_OK;
}
static inline void TEXTRA_CALL textra_skity_impl_destroy_manager(void* userdata,
                                                                 void* object) {
  textra_skity_impl_font_manager_t* manager_state =
      (textra_skity_impl_font_manager_t*)object;
  (void)userdata;
  if (!manager_state) return;
  if (manager_state->owned) skity_font_manager_destroy(manager_state->manager);
  free(manager_state);
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_count_families(
    void* userdata, void* manager_object, int32_t* family_count) {
  (void)userdata;
  *family_count = skity_font_manager_count_families(
      ((textra_skity_impl_font_manager_t*)manager_object)->manager);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_get_platform_font_from_typeface(void* userdata,
                                                  void* manager_object,
                                                  void* typeface,
                                                  const void** native_font) {
  (void)manager_object;
  return textra_skity_impl_get_native_typeface(userdata, typeface, native_font);
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_match_family_style_literal(void* userdata,
                                             void* manager_object,
                                             const char* family,
                                             const textra_font_style_t* style,
                                             void** out_typeface) {
  (void)userdata;
  *out_typeface = NULL;
  skity_font_manager manager =
      ((textra_skity_impl_font_manager_t*)manager_object)->manager;
  skity_font_style font_style = {style->weight, (int32_t)(style->width),
                                 (skity_font_slant)(style->slant)};
  return textra_skity_impl_wrap_typeface(
      family
          ? skity_font_manager_match_family_style(manager, family, font_style)
          : skity_font_manager_get_default_typeface(manager, font_style),
      out_typeface);
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_match_family_style(
    void* userdata, void* manager_object, const char* family,
    const textra_font_style_t* style, void** out_typeface) {
  *out_typeface = NULL;
  skity_font_manager manager =
      ((textra_skity_impl_font_manager_t*)manager_object)->manager;
  skity_font_style font_style = {style->weight, (int32_t)(style->width),
                                 (skity_font_slant)(style->slant)};
  if (!family || !strcmp(family, "sans-serif"))
    return textra_skity_impl_wrap_typeface(
        skity_font_manager_get_default_typeface(manager, font_style),
        out_typeface);
  return textra_skity_impl_match_family_style_literal(
      userdata, manager_object, family, style, out_typeface);
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_match_family_style_character(
    void* userdata, void* manager_object, const char* family,
    const textra_font_style_t* style, const char* const* languages,
    uint32_t language_count, uint32_t character, void** out_typeface) {
  (void)userdata;
  *out_typeface = NULL;
  skity_font_manager manager =
      ((textra_skity_impl_font_manager_t*)manager_object)->manager;
  skity_font_style font_style = {style->weight, (int32_t)(style->width),
                                 (skity_font_slant)(style->slant)};
  return textra_skity_impl_wrap_typeface(
      skity_font_manager_match_family_style_character(
          manager, family ? family : "", font_style, (const char**)languages,
          (int)(language_count), character),
      out_typeface);
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_make_from_file(
    void* userdata, void* manager_object, const char* path,
    int32_t collection_index, void** out_typeface) {
  (void)userdata;
  *out_typeface = NULL;
  return textra_skity_impl_wrap_typeface(
      skity_font_manager_make_from_file(
          ((textra_skity_impl_font_manager_t*)manager_object)->manager, path,
          collection_index),
      out_typeface);
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_create_typeface_from_platform_font(void* userdata,
                                                     void* manager_object,
                                                     const void* native_font,
                                                     void** out_typeface) {
  (void)manager_object;
  *out_typeface = NULL;
  return textra_skity_impl_import_typeface(userdata, native_font, out_typeface);
}
static const textra_backend_font_manager_api_t
    textra_skity_impl_font_manager_api = {
        sizeof(textra_backend_font_manager_api_t),
        TEXTRA_FONT_MANAGER_API_VERSION,
        textra_skity_impl_destroy_manager,
        textra_skity_impl_count_families,
        textra_skity_impl_match_family_style,
        textra_skity_impl_match_family_style,
        textra_skity_impl_match_family_style_literal,
        textra_skity_impl_match_family_style_character,
        textra_skity_impl_make_from_file,
        textra_skity_impl_match_family_style,
        textra_skity_impl_match_family_style_literal,
        textra_skity_impl_create_typeface_from_platform_font,
        textra_skity_impl_get_platform_font_from_typeface};
TEXTRA_EXTERN_C_END
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_FONT_MANAGER_IMPL_H_
