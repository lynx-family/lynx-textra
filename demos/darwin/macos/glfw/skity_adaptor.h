// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_DARWIN_MACOS_GLFW_SKITY_ADAPTOR_H_
#define DEMOS_DARWIN_MACOS_GLFW_SKITY_ADAPTOR_H_

#include <textra/i_font_manager.h>
#include <textra/platform/skity/skity_typeface_helper.h>

#include <memory>
#include <skity/text/font_manager.hpp>
#include <skity/text/typeface.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace tttext;

inline auto TF_DEFAULT = skity::Typeface::MakeFromFile(
    FONT_ROOT "Noto_Sans_CJK/NotoSansCJK-Regular.ttc");
inline auto TF_MENLO = skity::Typeface::MakeFromFile(
    FONT_ROOT "JetBrains_Mono/JetBrainsMono-VariableFont_wght.ttf");
inline auto TF_HELVETICA = skity::Typeface::MakeFromFile(
    FONT_ROOT "Inter/Inter-VariableFont_opsz,wght.ttf");
inline auto TF_EMOJI = skity::Typeface::MakeFromFile(
    FONT_ROOT "Noto_Color_Emoji/NotoColorEmoji-Regular.ttf");
inline auto TF_ARABIC = skity::Typeface::MakeFromFile(
    FONT_ROOT "Noto_Kufi_Arabic/NotoKufiArabic-VariableFont_wght.ttf");
inline auto TF_HEBREW = skity::Typeface::MakeFromFile(
    FONT_ROOT "Noto_Sans_Hebrew/NotoSansHebrew-VariableFont_wdth,wght.ttf");
inline auto TF_THAI = skity::Typeface::MakeFromFile(
    FONT_ROOT "Noto_Sans_Thai/NotoSansThai-VariableFont_wdth,wght.ttf");
inline auto TF_URDU = skity::Typeface::MakeFromFile(
    FONT_ROOT "Noto_Nastaliq_Urdu/NotoNastaliqUrdu-VariableFont_wght.ttf");
inline auto TFH_DEFAULT =
    std::make_shared<skity::textlayout::SkityTypefaceHelper>(TF_DEFAULT);
inline auto TFH_MENLO =
    std::make_shared<skity::textlayout::SkityTypefaceHelper>(TF_MENLO);
inline auto TFH_HELVETICA =
    std::make_shared<skity::textlayout::SkityTypefaceHelper>(TF_HELVETICA);
inline auto TFH_EMOJI =
    std::make_shared<skity::textlayout::SkityTypefaceHelper>(TF_EMOJI);
inline auto TFH_ARABIC =
    std::make_shared<skity::textlayout::SkityTypefaceHelper>(TF_ARABIC);
inline auto TFH_HEBREW =
    std::make_shared<skity::textlayout::SkityTypefaceHelper>(TF_HEBREW);
inline auto TFH_THAI =
    std::make_shared<skity::textlayout::SkityTypefaceHelper>(TF_THAI);
inline auto TFH_URDU =
    std::make_shared<skity::textlayout::SkityTypefaceHelper>(TF_URDU);

class SkityTestFontManager : public IFontManager {
 public:
  SkityTestFontManager() {
    std::vector<std::pair<FontDescriptor, const char*>> font_list = {
        {FontDescriptor{{"sans-serif"}, FontStyle::Normal(), 0},
         FONT_ROOT "Noto_Sans_CJK/NotoSansCJK-Regular.ttc"},
        {FontDescriptor{{"NotoSansCJK"}, FontStyle::Normal(), 0},
         FONT_ROOT "Noto_Sans_CJK/NotoSansCJK-Regular.ttc"},
        {FontDescriptor{{"NotoColorEmoji"}, FontStyle::Normal(), 0},
         FONT_ROOT "Noto_Color_Emoji/NotoColorEmoji-Regular.ttf"},
        {FontDescriptor{{"NotoKufiArabic"}, FontStyle::Normal(), 0},
         FONT_ROOT "Noto_Kufi_Arabic/NotoKufiArabic-VariableFont_wght.ttf"},
        {FontDescriptor{{"NotoSansHebrew"}, FontStyle::Normal(), 0}, FONT_ROOT
         "Noto_Sans_Hebrew/NotoSansHebrew-VariableFont_wdth,wght.ttf"},
        {FontDescriptor{{"NotoSansThai"}, FontStyle::Normal(), 0},
         FONT_ROOT "Noto_Sans_Thai/NotoSansThai-VariableFont_wdth,wght.ttf"},
        {FontDescriptor{{"Inter"}, FontStyle::Normal(), 0},
         FONT_ROOT "Inter/Inter-VariableFont_opsz,wght.ttf"},
        {FontDescriptor{{"NotoSansSC"}, FontStyle::Normal(), 0},
         FONT_ROOT "Noto_Sans_SC/NotoSansSC-VariableFont_wght.ttf"},
        {FontDescriptor{{"Menlo"}, FontStyle::Normal(), 0},
         FONT_ROOT "JetBrains_Mono/JetBrainsMono-VariableFont_wght.ttf"},
    };
    for (auto& item : font_list) {
      font_map_[item.first] =
          SkityTestFontManager::makeFromFile(item.second, 0);
    }
  }

  ~SkityTestFontManager() override = default;

 public:
  int countFamilies() const override { return font_map_.size(); }
  TypefaceRef matchFamilyStyle(const char familyName[],
                               const FontStyle&) override {
    FontDescriptor desc{{familyName}, FontStyle::Normal(), 0};
    auto it = font_map_.find(desc);
    return it == font_map_.end() ? nullptr : it->second;
  }
  TypefaceRef matchFamilyStyleCharacter(const char familyName[],
                                        const FontStyle&, const char* bcp47[],
                                        int bcp47Count,
                                        uint32_t character) override {
    for (auto typeface : font_map_) {
      if (typeface.second->UnicharToGlyph(character, 0) != 0) {
        return typeface.second;
      }
    }
    return nullptr;
  };
  TypefaceRef makeFromFile(const char path[], int ttcIndex) override {
    if (auto skity_typeface = skity::Typeface::MakeFromFile(path)) {
      return std::make_shared<skity::textlayout::SkityTypefaceHelper>(
          skity_typeface);
    }
    return nullptr;
  }
  TypefaceRef legacyMakeTypeface(const char familyName[],
                                 FontStyle style) const override {
    return nullptr;
  }
  TypefaceRef createTypefaceFromPlatformFont(
      const void* platform_font) override {
    return nullptr;
  }
  void* getPlatformFontFromTypeface(TypefaceRef typeface) override {
    return reinterpret_cast<void*>(typeface->GetFontIndex());
  }

 private:
  std::shared_ptr<skity::FontManager> font_manager_;
  std::unordered_map<FontDescriptor, TypefaceRef, FontDescriptor::Hasher>
      font_map_;
};
#endif  // DEMOS_DARWIN_MACOS_GLFW_SKITY_ADAPTOR_H_
