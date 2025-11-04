// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_ARK_GRAPHICS_AG_FONT_MANAGER_H_
#define PUBLIC_TEXTRA_PLATFORM_ARK_GRAPHICS_AG_FONT_MANAGER_H_
#include <native_drawing/drawing_font_mgr.h>
#include <native_drawing/drawing_text_typography.h>
#include <textra/i_font_manager.h>
#include <textra/platform/ark_graphics/ag_typeface_helper.h>

#include <memory>

namespace ttoffice {
namespace tttext {
class AGFontManager : public IFontManager {
 public:
  AGFontManager() { font_mgr_ = OH_Drawing_FontMgrCreate(); }
  ~AGFontManager() override {
    if (font_mgr_) {
      OH_Drawing_FontMgrDestroy(font_mgr_);
    }
  };

  int countFamilies() const override {
    return OH_Drawing_FontMgrGetFamilyCount(font_mgr_);
  }

  TypefaceRef matchFamilyStyle(const char familyName[],
                               const FontStyle& font_style) override {
    auto oh_font_style = ConvertOHDrawingFontStyle(font_style);
    auto oh_typeface = OH_Drawing_FontMgrMatchFamilyStyle(font_mgr_, familyName,
                                                          oh_font_style);
    return oh_typeface == nullptr
               ? std::make_shared<AGTypefaceHelper>(OH_Drawing_FontCreate())
               : std::make_shared<AGTypefaceHelper>(oh_typeface);
  }

  TypefaceRef matchFamilyStyleCharacter(const char familyName[],
                                        const FontStyle& font_style,
                                        const char* bcp47[], int bcp47Count,
                                        uint32_t character) override {
    auto oh_font_style = ConvertOHDrawingFontStyle(font_style);
    auto oh_typeface = OH_Drawing_FontMgrMatchFamilyStyleCharacter(
        font_mgr_, familyName, oh_font_style, bcp47, bcp47Count, character);
    return oh_typeface == nullptr
               ? std::make_shared<AGTypefaceHelper>(OH_Drawing_FontCreate())
               : std::make_shared<AGTypefaceHelper>(oh_typeface);
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

  static OH_Drawing_FontStyleStruct ConvertOHDrawingFontStyle(
      const FontStyle& style) {
    OH_Drawing_FontStyleStruct oh_font_style{};
    switch (style.GetWeight()) {
      case FontStyle::kInvisible_Weight:
        oh_font_style.weight = FONT_WEIGHT_100;
        break;
      case FontStyle::kThin_Weight:
        oh_font_style.weight = FONT_WEIGHT_100;
        break;
      case FontStyle::kExtraLight_Weight:
        oh_font_style.weight = FONT_WEIGHT_200;
        break;
      case FontStyle::kLight_Weight:
        oh_font_style.weight = FONT_WEIGHT_300;
        break;
      case FontStyle::kNormal_Weight:
        oh_font_style.weight = FONT_WEIGHT_400;
        break;
      case FontStyle::kMedium_Weight:
        oh_font_style.weight = FONT_WEIGHT_500;
        break;
      case FontStyle::kSemiBold_Weight:
        oh_font_style.weight = FONT_WEIGHT_600;
        break;
      case FontStyle::kBold_Weight:
        oh_font_style.weight = FONT_WEIGHT_700;
        break;
      case FontStyle::kExtraBold_Weight:
        oh_font_style.weight = FONT_WEIGHT_800;
        break;
      case FontStyle::kBlack_Weight:
        oh_font_style.weight = FONT_WEIGHT_900;
        break;
      case FontStyle::kExtraBlack_Weight:
        oh_font_style.weight = FONT_WEIGHT_900;
        break;
    }
    switch (style.GetSlant()) {
      case FontStyle::kUpright_Slant:
        oh_font_style.slant = FONT_STYLE_NORMAL;
        break;
      case FontStyle::kItalic_Slant:
        oh_font_style.slant = FONT_STYLE_ITALIC;
        break;
      case FontStyle::kOblique_Slant:
        oh_font_style.slant = FONT_STYLE_OBLIQUE;
        break;
    }
    switch (style.GetWeight()) {
      case FontStyle::kInvisible_Weight:
        oh_font_style.weight = FONT_WEIGHT_100;
        break;
      case FontStyle::kThin_Weight:
        oh_font_style.weight = FONT_WEIGHT_100;
        break;
      case FontStyle::kExtraLight_Weight:
        oh_font_style.weight = FONT_WEIGHT_200;
        break;
      case FontStyle::kLight_Weight:
        oh_font_style.weight = FONT_WEIGHT_300;
        break;
      case FontStyle::kNormal_Weight:
        oh_font_style.weight = FONT_WEIGHT_400;
        break;
      case FontStyle::kMedium_Weight:
        oh_font_style.weight = FONT_WEIGHT_500;
        break;
      case FontStyle::kSemiBold_Weight:
        oh_font_style.weight = FONT_WEIGHT_600;
        break;
      case FontStyle::kBold_Weight:
        oh_font_style.weight = FONT_WEIGHT_700;
        break;
      case FontStyle::kExtraBold_Weight:
        oh_font_style.weight = FONT_WEIGHT_800;
        break;
      case FontStyle::kBlack_Weight:
        oh_font_style.weight = FONT_WEIGHT_900;
        break;
      case FontStyle::kExtraBlack_Weight:
        oh_font_style.weight = FONT_WEIGHT_900;
        break;
    }
    return oh_font_style;
  }

 private:
  OH_Drawing_FontMgr* font_mgr_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_ARK_GRAPHICS_AG_FONT_MANAGER_H_
