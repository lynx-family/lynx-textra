// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/platform/lvgl/lvgl_font_manager.h>

namespace ttoffice {
namespace tttext {
LVGLFontManager::LVGLFontManager() {}
LVGLFontManager::~LVGLFontManager() {}
int LVGLFontManager::countFamilies() const { return 0; }
TypefaceRef LVGLFontManager::matchFamilyStyle(const char familyName[],
                                              const FontStyle&) {
  return nullptr;
}
TypefaceRef LVGLFontManager::matchFamilyStyleCharacter(const char familyName[],
                                                       const FontStyle&,
                                                       const char* bcp47[],
                                                       int bcp47Count,
                                                       uint32_t character) {
  return nullptr;
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
