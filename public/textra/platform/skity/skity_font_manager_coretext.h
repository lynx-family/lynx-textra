// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_CORETEXT_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_CORETEXT_H_

#import <CoreText/CTFont.h>
#include <textra/platform/skity/skity_font_manager.h>

#include <memory>
#include <skity/text/ports/typeface_ct.hpp>
namespace ttoffice {
namespace tttext {
class SkityFontManagerCoreText : public SkityFontManager {
 public:
  SkityFontManagerCoreText() : SkityFontManager() {}
  explicit SkityFontManagerCoreText(
      std::shared_ptr<skity::FontManager> font_manager)
      : SkityFontManager(font_manager) {}
  ~SkityFontManagerCoreText() override = default;

 public:
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

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_CORETEXT_H_
