// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/native/lvgl/lvgl_font_manager.h"

#include "src/ports/shaper/lvgl/lvgl_typeface_cjk.h"
#include "src/ports/shaper/lvgl/lvgl_typeface_montserrat.h"

namespace ttoffice {
namespace tttext {
LVGLFontManager::LVGLFontManager() {}
LVGLFontManager::~LVGLFontManager() {}
int LVGLFontManager::countFamilies() const { return 2; }
TypefaceRef LVGLFontManager::matchFamilyStyle(const char familyName[],
                                              const FontStyle&) {
  return std::make_shared<LVGLTypefaceMontserrat>(0);
}
TypefaceRef LVGLFontManager::matchFamilyStyleCharacter(const char familyName[],
                                                       const FontStyle&,
                                                       const char* bcp47[],
                                                       int bcp47Count,
                                                       uint32_t character) {
  auto latin = std::make_shared<LVGLTypefaceMontserrat>(0);
  lv_font_glyph_dsc_t desc{};
  if (lv_font_get_glyph_dsc(latin->GetLvFont(16), &desc, character, 0) &&
      !desc.is_placeholder)
    return latin;
  auto cjk = std::make_shared<LVGLTypefaceCJK>(1);
  return lv_font_get_glyph_dsc(cjk->GetLvFont(16), &desc, character, 0) &&
                 !desc.is_placeholder
             ? cjk
             : nullptr;
}
TypefaceRef LVGLFontManager::makeFromFile(const char path[], int ttcIndex) {
  return nullptr;
}
TypefaceRef LVGLFontManager::legacyMakeTypeface(const char familyName[],
                                                FontStyle style) const {
  return nullptr;
}
}  // namespace tttext
}  // namespace ttoffice
