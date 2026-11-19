// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/capi/textra_callback_utils.h"
#include "src/capi/textra_internal.h"

using namespace textra_capi;

namespace ttoffice {
namespace tttext {
namespace backend {
CallbackFontManager::CallbackFontManager(
    const textra_font_manager_options_t& options, RenderingBackend backend)
    : native_tokens_(NativeFontTokens(static_cast<uint32_t>(backend))),
      userdata_(options.userdata),
      native_manager_(options.font_manager) {
  CopyCallbacks(callbacks_, options.manager_callbacks);
  CopyCallbacks(typeface_callbacks_, options.typeface_callbacks);
}

CallbackFontManager::~CallbackFontManager() {
  if (owns_manager_) callbacks_.destroy(userdata_, native_manager_);
}

int CallbackFontManager::countFamilies() const {
  int32_t n = 0;
  callbacks_.count_families(userdata_, native_manager_, &n);
  return n;
}

TypefaceRef CallbackFontManager::matchFamilyStyle(
    const char* family, const tttext::FontStyle& font_style) {
  const textra_font_style_t style{font_style.GetWeight(), font_style.GetWidth(),
                                  font_style.GetSlant()};
  void* face = nullptr;
  auto status = callbacks_.match_family_style(userdata_, native_manager_,
                                              family, &style, &face);
  return AdoptTypeface(face, status);
}
TypefaceRef CallbackFontManager::matchGenericFamily(
    const char* family, const tttext::FontStyle& font_style) {
  const textra_font_style_t style{font_style.GetWeight(), font_style.GetWidth(),
                                  font_style.GetSlant()};
  void* face = nullptr;
  auto status = callbacks_.match_generic_family(userdata_, native_manager_,
                                                family, &style, &face);
  return AdoptTypeface(face, status);
}
TypefaceRef CallbackFontManager::matchFamilyStyleLiteral(
    const char* family, const tttext::FontStyle& font_style) {
  const textra_font_style_t style{font_style.GetWeight(), font_style.GetWidth(),
                                  font_style.GetSlant()};
  void* face = nullptr;
  auto status = callbacks_.match_family_style_literal(
      userdata_, native_manager_, family, &style, &face);
  return AdoptTypeface(face, status);
}
TypefaceRef CallbackFontManager::matchFamilyStyleCharacter(
    const char* family, const tttext::FontStyle& font_style,
    const char** languages, int language_count, uint32_t character) {
  const textra_font_style_t style{font_style.GetWeight(), font_style.GetWidth(),
                                  font_style.GetSlant()};
  void* face = nullptr;
  auto status = callbacks_.match_family_style_character(
      userdata_, native_manager_, family, &style, languages,
      static_cast<uint32_t>(language_count), character, &face);
  return AdoptTypeface(face, status);
}
TypefaceRef CallbackFontManager::makeFromFile(const char* path,
                                              int32_t collection_index) {
  void* face = nullptr;
  auto status = callbacks_.make_from_file(userdata_, native_manager_, path,
                                          collection_index, &face);
  return AdoptTypeface(face, status);
}
TypefaceRef CallbackFontManager::legacyMakeTypeface(
    const char* family, tttext::FontStyle font_style) const {
  const textra_font_style_t style{font_style.GetWeight(), font_style.GetWidth(),
                                  font_style.GetSlant()};
  void* face = nullptr;
  auto status = callbacks_.legacy_make_typeface(userdata_, native_manager_,
                                                family, &style, &face);
  return AdoptTypeface(face, status);
}
TypefaceRef CallbackFontManager::legacyMakeTypefaceLiteral(
    const char* family, tttext::FontStyle font_style) const {
  const textra_font_style_t style{font_style.GetWeight(), font_style.GetWidth(),
                                  font_style.GetSlant()};
  void* face = nullptr;
  auto status = callbacks_.legacy_make_typeface_literal(
      userdata_, native_manager_, family, &style, &face);
  return AdoptTypeface(face, status);
}
TypefaceRef CallbackFontManager::createTypefaceFromPlatformFont(
    const void* native_font) {
  // Non-native shapers use the retained CallbackTypeface as their internal
  // platform-font token. Native shapers provide SDK fonts for import.
  if (!native_tokens_) {
    auto face = static_cast<const CallbackTypeface*>(
        static_cast<const ITypefaceHelper*>(native_font));
    return const_cast<CallbackTypeface*>(face)->shared_from_this();
  }
  void* face = nullptr;
  auto status = callbacks_.create_typeface_from_platform_font(
      userdata_, native_manager_, native_font, &face);
  return AdoptTypeface(face, status);
}

void* CallbackFontManager::getPlatformFontFromTypeface(TypefaceRef input) {
  auto face = std::static_pointer_cast<CallbackTypeface>(input);
  if (!native_tokens_) return face.get();
  const void* token = nullptr;
  callbacks_.platform_font(userdata_, native_manager_, face->NativeObject(),
                           &token);
  return const_cast<void*>(token);
}

}  // namespace backend
}  // namespace tttext
}  // namespace ttoffice
