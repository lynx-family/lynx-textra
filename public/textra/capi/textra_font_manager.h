// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_FONT_MANAGER_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_FONT_MANAGER_H_

#include <textra/capi/textra_types.h>

/* Each callback corresponds to one IFontManager method. Inputs are borrowed
 * for the synchronous call. Successful font callbacks transfer one owned
 * provider typeface to the engine; NULL means no match. Errors are not
 * propagated through I* methods. */
#define TEXTRA_FONT_MANAGER_API_VERSION ((uint32_t)2)
typedef struct textra_backend_font_manager_api {
  uint32_t struct_size, version;
  void(TEXTRA_CALL* destroy)(void*, void*);
  textra_status_t(TEXTRA_CALL* count_families)(void*, void*, int32_t*);
  textra_status_t(TEXTRA_CALL* match_family_style)(
      void* userdata, void* manager, const char* family,
      const textra_font_style_t* style, void** out_typeface);
  textra_status_t(TEXTRA_CALL* match_generic_family)(
      void* userdata, void* manager, const char* family,
      const textra_font_style_t* style, void** out_typeface);
  textra_status_t(TEXTRA_CALL* match_family_style_literal)(
      void* userdata, void* manager, const char* family,
      const textra_font_style_t* style, void** out_typeface);
  textra_status_t(TEXTRA_CALL* match_family_style_character)(
      void* userdata, void* manager, const char* family,
      const textra_font_style_t* style, const char* const* languages,
      uint32_t language_count, uint32_t character, void** out_typeface);
  textra_status_t(TEXTRA_CALL* make_from_file)(void* userdata, void* manager,
                                               const char* path,
                                               int32_t collection_index,
                                               void** out_typeface);
  textra_status_t(TEXTRA_CALL* legacy_make_typeface)(
      void* userdata, void* manager, const char* family,
      const textra_font_style_t* style, void** out_typeface);
  textra_status_t(TEXTRA_CALL* legacy_make_typeface_literal)(
      void* userdata, void* manager, const char* family,
      const textra_font_style_t* style, void** out_typeface);
  textra_status_t(TEXTRA_CALL* create_typeface_from_platform_font)(
      void* userdata, void* manager, const void* native_font,
      void** out_typeface);
  textra_status_t(TEXTRA_CALL* platform_font)(void*, void*, void* typeface,
                                              const void**);
} textra_backend_font_manager_api_t;

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_FONT_MANAGER_H_
