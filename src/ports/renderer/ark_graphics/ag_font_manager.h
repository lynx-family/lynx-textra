// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_FONT_MANAGER_H_
#define SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_FONT_MANAGER_H_
#include <textra/i_font_manager.h>

#include <memory>

#include "src/ports/renderer/ark_graphics/ag_typeface_helper.h"

namespace ttoffice {
namespace tttext {
class AGFontManager : public IFontManager {
 public:
  AGFontManager() = default;
  ~AGFontManager() override = default;

  int countFamilies() const override { return 0; }

  TypefaceRef matchFamilyStyle(const char familyName[],
                               const FontStyle&) override {
    return nullptr;
  }

  TypefaceRef matchFamilyStyleCharacter(const char familyName[],
                                        const FontStyle&, const char* bcp47[],
                                        int bcp47Count,
                                        uint32_t character) override {
    return nullptr;
  }
  TypefaceRef makeFromFile(const char path[], int ttcIndex) override {
    return nullptr;
  }
  TypefaceRef legacyMakeTypeface(const char familyName[],
                                 FontStyle style) const override {
    return nullptr;
  }
  TypefaceRef createTypefaceFromPlatformFont(
      const void* platform_font) override {
    auto pf = const_cast<void*>(platform_font);
    auto* oh_font = static_cast<OH_Drawing_Font*>(pf);
    return std::make_shared<AGTypefaceHelper>(oh_font);
  }

  void* getPlatformFontFromTypeface(TypefaceRef typeface) override {
    auto ag_typeface_helper =
        std::static_pointer_cast<AGTypefaceHelper>(typeface);
    return ag_typeface_helper->GetTypeface();
  }
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_FONT_MANAGER_H_
