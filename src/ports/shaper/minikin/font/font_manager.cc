// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


#include "font_manager.h"

#include "font_helper.h"
#include "shaper/minikin/lib/FontFamily.h"

extern char* MAIN_FONT_PATH;

namespace ttoffice {
namespace tttext {

std::shared_ptr<IFontManager> ttoffice::textlayout::IFontManager::RefDefault() {
  static auto skity_font_manager = std::make_shared<FontManager>();
  return skity_font_manager;
}

FontManager::FontManager() {
  typeface_ = ITypefaceHelper::MakeFromFile(MAIN_FONT_PATH);
}

FontManager::~FontManager() = default;

int FontManager::countFamilies() const {
  // return skity_font_mgr_->countFamilies();
  return 1;
}

std::shared_ptr<minikin::FontFamily> FontManager::matchFamilyStyle(
    const char* familyName, const minikin::FontStyle& style) {
  return RegisterUniqueFont(typeface_, style);
}

std::shared_ptr<minikin::FontFamily> FontManager::matchFamilyStyleCharacter(
    const char* familyName, const minikin::FontStyle& style, const char** bcp47,
    int bcp47Count, uint32_t character) {
  return RegisterUniqueFont(typeface_, style);
}
std::shared_ptr<minikin::FontFamily> FontManager::makeFromFile(const char* path,
                                                               int ttcIndex) {
  //  auto typeface = sk_font_mgr_->makeFromFile(path, ttcIndex);
  //  return RegisterUniqueFont(typeface);
  return nullptr;
}

std::shared_ptr<minikin::FontFamily> FontManager::RegisterUniqueFont(
    std::shared_ptr<ITypefaceHelper> typeface,
    const minikin::FontStyle& style) {
  auto minikinFont = std::make_shared<minikin::FontHelper>(typeface);
  std::vector<minikin::Font> fonts = {
      minikin::Font::Builder(minikinFont).setStyle(style).build()};
  return std::make_shared<minikin::FontFamily>(
      0, minikin::FamilyVariant::DEFAULT, std::move(fonts), false);
}

}  // namespace tttext
}  // namespace ttoffice
