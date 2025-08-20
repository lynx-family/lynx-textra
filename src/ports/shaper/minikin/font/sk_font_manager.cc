// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


#include "sk_font_manager.h"

#include "core/SkFontMgr.h"
#include "core/SkFontStyle.h"
#include "core/SkTypeface.h"
#include "log_util.h"
#include "sk_minikin_font.h"
namespace ttoffice {
namespace tttext {
std::shared_ptr<IFontManager> IFontManager::RefDefault() {
  static auto skia_font_manager = std::make_shared<SkFontManager>();
  return skia_font_manager;
}
SkFontManager::SkFontManager() : sk_font_mgr_(SkFontMgr::RefDefault()) {}
SkFontManager::~SkFontManager() = default;
int SkFontManager::countFamilies() const {
  return sk_font_mgr_->countFamilies();
}
std::shared_ptr<FontFamily> SkFontManager::matchFamilyStyle(
    const char* familyName, const FontStyle& style) {
  auto typeface = sk_sp<SkTypeface>(
      sk_font_mgr_->matchFamilyStyle(familyName, ToSkFontStyle(style)));
  return RegisterUniqueFont(typeface, style);
}
std::shared_ptr<FontFamily> SkFontManager::matchFamilyStyleCharacter(
    const char* familyName, const FontStyle& style, const char** bcp47,
    int bcp47Count, uint32_t character) {
  LogUtil::D("matchFamilyStyleCharacter char:%d ", character);
  auto typeface = sk_sp<SkTypeface>(sk_font_mgr_->matchFamilyStyleCharacter(
      familyName, ToSkFontStyle(style), bcp47, bcp47Count, character));
  return RegisterUniqueFont(typeface, style);
}
std::shared_ptr<FontFamily> SkFontManager::makeFromFile(const char* path,
                                                        int ttcIndex) {
  //  auto typeface = sk_font_mgr_->makeFromFile(path, ttcIndex);
  //  return RegisterUniqueFont(typeface);
  return nullptr;
}
SkFontStyle SkFontManager::ToSkFontStyle(const FontStyle& font_style) const {
  return SkFontStyle((SkFontStyle::Weight)font_style.weight(),
                     SkFontStyle::Width::kNormal_Width,
                     font_style.slant() == FontStyle::Slant::ITALIC
                         ? SkFontStyle::Slant::kItalic_Slant
                         : SkFontStyle::Slant::kUpright_Slant);
}
std::shared_ptr<FontFamily> SkFontManager::RegisterUniqueFont(
    sk_sp<SkTypeface>& typeface, const FontStyle& style) {
  auto unique_id = typeface->uniqueID();
  //  auto iter = font_map_.find(unique_id);
  //  if (iter != font_map_.end()) {
  //    LogUtil::D("RegisterUniqueFont id:%d duplicated", unique_id);
  //    return iter->second;
  //  }
  LogUtil::D("RegisterUniqueFont id:%d success", unique_id);
  auto minikinFont = std::make_shared<SkMinikinFont>(typeface);
  std::vector<minikin::Font> fonts = {
      minikin::Font::Builder(minikinFont).setStyle(style).build()};
  return std::make_shared<minikin::FontFamily>(
      0, minikin::FamilyVariant::DEFAULT, std::move(fonts), false);
}
}  // namespace tttext
}  // namespace ttoffice
