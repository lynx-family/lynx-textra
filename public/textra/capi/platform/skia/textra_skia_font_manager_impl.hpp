// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_FONT_MANAGER_IMPL_HPP_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_FONT_MANAGER_IMPL_HPP_

#include <include/core/SkFontMgr.h>

#include <textra/capi/platform/skia/textra_skia_typeface_impl.hpp>

namespace textra_native {
namespace skia_backend {
inline textra_status_t TEXTRA_CALL CreateManager(void*, void* p, void** out) {
  *out = nullptr;

  if (!p) return TEXTRA_STATUS_UNSUPPORTED;
  auto manager = static_cast<SkFontMgr*>(p);
  manager->ref();
  *out = manager;
  return TEXTRA_STATUS_OK;
}
inline void TEXTRA_CALL DestroyManager(void*, void* p) {
  static_cast<SkFontMgr*>(p)->unref();
}
inline textra_status_t TEXTRA_CALL Count(void*, void* p, int32_t* out) {
  *out = static_cast<SkFontMgr*>(p)->countFamilies();
  return 0;
}
inline textra_status_t TEXTRA_CALL Platform(void* u, void*, void* p,
                                            const void** out) {
  return Native(u, p, out);
}
inline textra_status_t TEXTRA_CALL
MatchFamilyStyle(void* u, void* p, const char* family,
                 const textra_font_style_t* style, void** out) {
  *out = nullptr;
  auto* manager = static_cast<SkFontMgr*>(p);
  SkFontStyle font_style(style->weight, style->width,
                         static_cast<SkFontStyle::Slant>(style->slant));
  return Wrap(manager->matchFamilyStyle(family, font_style), out);
}
inline textra_status_t TEXTRA_CALL
MatchGenericFamily(void* u, void* p, const char* family,
                   const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return MatchFamilyStyle(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL
MatchFamilyStyleLiteral(void* u, void* p, const char* family,
                        const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return MatchFamilyStyle(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL MatchFamilyStyleCharacter(
    void* u, void* p, const char* family, const textra_font_style_t* style,
    const char* const* languages, uint32_t language_count, uint32_t character,
    void** out) {
  *out = nullptr;
  auto* manager = static_cast<SkFontMgr*>(p);
  SkFontStyle font_style(style->weight, style->width,
                         static_cast<SkFontStyle::Slant>(style->slant));
  return Wrap(manager->matchFamilyStyleCharacter(
                  family, font_style, const_cast<const char**>(languages),
                  static_cast<int>(language_count), character),
              out);
}
inline textra_status_t TEXTRA_CALL MakeFromFile(void* u, void* p,
                                                const char* path,
                                                int32_t collection_index,
                                                void** out) {
  *out = nullptr;
  return Wrap(static_cast<SkFontMgr*>(p)->makeFromFile(path, collection_index),
              out);
}
inline textra_status_t TEXTRA_CALL
LegacyMakeTypeface(void* u, void* p, const char* family,
                   const textra_font_style_t* style, void** out) {
  *out = nullptr;
  auto* manager = static_cast<SkFontMgr*>(p);
  SkFontStyle font_style(style->weight, style->width,
                         static_cast<SkFontStyle::Slant>(style->slant));
  return Wrap(manager->legacyMakeTypeface(family, font_style), out);
}
inline textra_status_t TEXTRA_CALL
LegacyMakeTypefaceLiteral(void* u, void* p, const char* family,
                          const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return LegacyMakeTypeface(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL CreateTypefaceFromPlatformFont(
    void* u, void* p, const void* native_font, void** out) {
  *out = nullptr;
  return Import(u, native_font, out);
}
inline const textra_backend_font_manager_api_t& ManagerApi() {
  static const textra_backend_font_manager_api_t kFontManagerApi = {
      sizeof(textra_backend_font_manager_api_t),
      TEXTRA_FONT_MANAGER_API_VERSION,
      DestroyManager,
      Count,
      MatchFamilyStyle,
      MatchGenericFamily,
      MatchFamilyStyleLiteral,
      MatchFamilyStyleCharacter,
      MakeFromFile,
      LegacyMakeTypeface,
      LegacyMakeTypefaceLiteral,
      CreateTypefaceFromPlatformFont,
      Platform};
  return kFontManagerApi;
}
}  // namespace skia_backend
}  // namespace textra_native
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_FONT_MANAGER_IMPL_HPP_
