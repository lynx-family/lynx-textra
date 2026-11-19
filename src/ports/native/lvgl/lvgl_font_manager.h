// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_NATIVE_LVGL_LVGL_FONT_MANAGER_H_
#define SRC_PORTS_NATIVE_LVGL_LVGL_FONT_MANAGER_H_
#include <textra/i_font_manager.h>

namespace ttoffice {
namespace tttext {
class LVGLFontManager : public IFontManager {
 public:
  LVGLFontManager();
  ~LVGLFontManager() override;

 public:
  int countFamilies() const override;
  TypefaceRef matchFamilyStyle(const char familyName[],
                               const FontStyle&) override;
  TypefaceRef matchFamilyStyleCharacter(const char familyName[],
                                        const FontStyle&, const char* bcp47[],
                                        int bcp47Count,
                                        uint32_t character) override;
  TypefaceRef makeFromFile(const char path[], int ttcIndex) override;
  TypefaceRef legacyMakeTypeface(const char familyName[],
                                 FontStyle style) const override;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_NATIVE_LVGL_LVGL_FONT_MANAGER_H_
