// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_H_

#include <textra/i_font_manager.h>
#include <textra/platform/skity/skity_typeface_helper.h>

#include <memory>
#include <skity/text/font_manager.hpp>
namespace ttoffice {
namespace tttext {
class SkityFontManager : public IFontManager {
 public:
  SkityFontManager() { font_manager_ = skity::FontManager::RefDefault(); }
  explicit SkityFontManager(std::shared_ptr<skity::FontManager> font_manager)
      : font_manager_(font_manager) {}
  ~SkityFontManager() = default;

 public:
  int countFamilies() const override { return 1; }
  TypefaceRef matchFamilyStyle(const char* familyName,
                               const FontStyle& style) override {
    return legacyMakeTypeface(familyName, style);
  }
  TypefaceRef matchGenericFamily(const char* generic,
                                 const FontStyle& style) override {
    skity::FontStyle skity_style(
        style.GetWeight(), style.GetWidth(),
        static_cast<skity::FontStyle::Slant>(style.GetSlant()));
    // Retain this adapter's actual sans-serif default policy; do not ask
    // dynamic or asset managers for a literal font named sans-serif.
    auto face = strcmp(generic, "sans-serif") == 0
                    ? font_manager_->GetDefaultTypeface(skity_style)
                    : font_manager_->MatchFamilyStyle(generic, skity_style);
    return face ? std::make_shared<skity::textlayout::SkityTypefaceHelper>(face)
                : nullptr;
  }
  TypefaceRef matchFamilyStyleLiteral(const char* family,
                                      const FontStyle& style) override {
    return legacyMakeTypefaceLiteral(family, style);
  }
  TypefaceRef legacyMakeTypefaceLiteral(const char* family,
                                        FontStyle style) const override {
    skity::FontStyle skity_style(
        style.GetWeight(), style.GetWidth(),
        static_cast<skity::FontStyle::Slant>(style.GetSlant()));
    // Bypass this adapter's sans-serif default policy for named families.
    auto face = font_manager_->MatchFamilyStyle(family, skity_style);
    return face ? std::make_shared<skity::textlayout::SkityTypefaceHelper>(face)
                : nullptr;
  }
  TypefaceRef matchFamilyStyleCharacter(const char* familyName,
                                        const FontStyle& style,
                                        const char** bcp47, int bcp47Count,
                                        uint32_t character) override {
    skity::FontStyle skity_style(
        style.GetWeight(), style.GetWidth(),
        static_cast<skity::FontStyle::Slant>(style.GetSlant()));
    auto typeface = font_manager_->MatchFamilyStyleCharacter(
        familyName, skity_style, bcp47, bcp47Count, character);

    return typeface ? std::make_shared<skity::textlayout::SkityTypefaceHelper>(
                          typeface)
                    : nullptr;
  }
  TypefaceRef makeFromFile(const char* path, int ttcIndex) override {
    auto typeface = font_manager_->MakeFromFile(path, ttcIndex);
    return typeface ? std::make_shared<skity::textlayout::SkityTypefaceHelper>(
                          typeface)
                    : nullptr;
  }
  TypefaceRef legacyMakeTypeface(const char* familyName,
                                 FontStyle style) const override {
    skity::FontStyle skity_style(
        style.GetWeight(), style.GetWidth(),
        static_cast<skity::FontStyle::Slant>(style.GetSlant()));
    std::shared_ptr<skity::Typeface> typeface = nullptr;

    auto len = strlen("sans-serif");
    if (strlen(familyName) == len &&
        strncmp(familyName, "sans-serif", len) == 0) {
      typeface = font_manager_->GetDefaultTypeface(skity_style);
    } else {
      typeface = font_manager_->MatchFamilyStyle(familyName, skity_style);
    }

    return typeface ? std::make_shared<skity::textlayout::SkityTypefaceHelper>(
                          typeface)
                    : nullptr;
  }

 protected:
  std::shared_ptr<skity::FontManager> font_manager_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_H_
