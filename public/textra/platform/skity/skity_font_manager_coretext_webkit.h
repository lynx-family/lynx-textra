// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_CORETEXT_WEBKIT_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_CORETEXT_WEBKIT_H_

#import <CoreText/CTFont.h>
#include <textra/platform/skity/skity_font_manager_coretext.h>

#include <memory>
#include <skity/text/ports/typeface_ct.hpp>
namespace ttoffice {
namespace tttext {
class SkityFontManagerCoreTextWebkit : public SkityFontManagerCoreText {
 public:
  SkityFontManagerCoreTextWebkit() : SkityFontManagerCoreText() {}
  explicit SkityFontManagerCoreTextWebkit(
      std::shared_ptr<skity::FontManager> font_manager)
      : SkityFontManagerCoreText(font_manager) {}
  ~SkityFontManagerCoreTextWebkit() override = default;

 public:
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
      typeface =
          font_manager_->MatchFamily(familyName)->MatchStyle(skity_style);
    }

    return typeface ? std::make_shared<skity::textlayout::SkityTypefaceHelper>(
                          typeface)
                    : nullptr;
  }
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_MANAGER_CORETEXT_WEBKIT_H_
