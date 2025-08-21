// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_FONT_MANAGER_CORETEXT_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_FONT_MANAGER_CORETEXT_H_
#import <CoreText/CTFont.h>
#include <textra/i_font_manager.h>
#include <textra/platform/skity_coretext/skity_typeface_helper.h>

#include <memory>
#include <skity/text/font_manager.hpp>
#include <skity/text/ports/typeface_ct.hpp>
namespace ttoffice {
namespace tttext {
class SkityFontManager : public IFontManager {
 public:
  SkityFontManager() { font_manager_ = skity::FontManager::RefDefault(); }
  ~SkityFontManager() = default;

 public:
  int countFamilies() const override { return 1; }
  TypefaceRef matchFamilyStyle(const char* familyName,
                               const FontStyle& style) override {
    return legacyMakeTypeface(familyName, style);
  }
  TypefaceRef matchFamilyStyleCharacter(const char* familyName,
                                        const FontStyle& style,
                                        const char** bcp47, int bcp47Count,
                                        uint32_t character) override {
    skity::FontStyle skity_style(
        style.weight(), style.width(),
        static_cast<skity::FontStyle::Slant>(style.slant()));
    return std::make_shared<skity::textlayout::SkityTypefaceHelper>(
        font_manager_->MatchFamilyStyleCharacter(familyName, skity_style, bcp47,
                                                 bcp47Count, character));
  }
  TypefaceRef makeFromFile(const char* path, int ttcIndex) override {
    return std::make_shared<skity::textlayout::SkityTypefaceHelper>(
        font_manager_->MakeFromFile(path, ttcIndex));
  }
  TypefaceRef legacyMakeTypeface(const char* familyName,
                                 FontStyle style) const override {
    skity::FontStyle skity_style(
        style.weight(), style.width(),
        static_cast<skity::FontStyle::Slant>(style.slant()));
    return std::make_shared<skity::textlayout::SkityTypefaceHelper>(
        font_manager_->MatchFamilyStyle(familyName, skity_style));
  }
  TypefaceRef createTypefaceFromPlatformFont(
      const void* platform_font) override {
    CTFontRef font_ref = (CTFontRef)platform_font;
    TypefaceRef typeface =
        std::make_shared<skity::textlayout::SkityTypefaceHelper>(
            skity::TypefaceCT::TypefaceFromCTFont(font_ref));
    return typeface;
  }
  void* getPlatformFontFromTypeface(TypefaceRef typeface) override {
    auto skity_typeface_helper =
        std::static_pointer_cast<skity::textlayout::SkityTypefaceHelper>(
            typeface);
    CTFontRef ct_font = skity::TypefaceCT::CTFontFromTypeface(
        skity_typeface_helper->GetTypeface());
    return (void*)(ct_font);  // NOLINT
  }

 private:
  std::shared_ptr<skity::FontManager> font_manager_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_FONT_MANAGER_CORETEXT_H_
