// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_IOS_TYPEFACE_CORETEXT_H_
#define PUBLIC_TEXTRA_PLATFORM_IOS_TYPEFACE_CORETEXT_H_

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#include <textra/i_typeface_helper.h>
#include <textra/macro.h>

#include <cmath>
namespace ttoffice {
namespace tttext {
/**
 * @brief A typeface helper implementation backed by Apple's CoreText framework.
 */
class TypefaceCoreText : public ITypefaceHelper {
 public:
  TypefaceCoreText() = delete;
  explicit TypefaceCoreText(CTFontRef font, uint32_t id)
      : ITypefaceHelper(id), font_ref_((CTFontRef)CFRetain(font)) {
#ifdef TTTEXT_DEBUG
    auto cf_name = CTFontCopyFullName(font_ref_);
    font_name_ = CFStringGetCStringPtr(cf_name, kCFStringEncodingUTF8);
    CFRelease(cf_name);
#endif
  }
  ~TypefaceCoreText() override {
    if (font_ref_ != nullptr) {
      CFRelease(font_ref_);
    }
  };

 public:
  float GetHorizontalAdvance(GlyphID glyph_id, float font_size) const override {
    TTASSERT(false);
    return 0;
  };

  void GetHorizontalAdvances(GlyphID glyph_ids[], uint32_t count,
                             float widths[], float font_size) const override {
    TTASSERT(false);
  };
  void GetWidthBounds(float* rect_ltrb, GlyphID glyphs[], uint32_t glyph_count,
                      float font_size) override {
    auto new_font =
        CTFontCreateCopyWithAttributes(font_ref_, font_size, nullptr, nullptr);
    auto bounding = CTFontGetBoundingRectsForGlyphs(
        new_font, kCTFontOrientationDefault, glyphs, nullptr, glyph_count);
    rect_ltrb[0] = bounding.origin.x;
    rect_ltrb[1] = -(bounding.origin.y + bounding.size.height);
    rect_ltrb[2] = bounding.origin.x + bounding.size.width;
    rect_ltrb[3] = -bounding.origin.y;
    CFRelease(new_font);
  };

  void GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                     float font_size) const override {
    TTASSERT(false);
  };

  const void* GetFontData() const override {
    TTASSERT(false);
    return nullptr;
  };

  size_t GetFontDataSize() const override {
    TTASSERT(false);
    return 0;
  };

  int GetFontIndex() const override {
    TTASSERT(false);
    return 0;
  };

  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variationSelector = 0) const override {
    TTASSERT(false);
    return 0;
  };

  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override {
    TTASSERT(false);
  };

  uint32_t GetUnitsPerEm() const override {
    TTASSERT(false);
    return 0;
  };

  void OnCreateFontInfo(FontInfo* info, float font_size) const override {
    if (font_size == CTFontGetSize(font_ref_)) {
      *info =
          FontInfo{static_cast<float>(-CTFontGetAscent(font_ref_)),
                   static_cast<float>(CTFontGetDescent(font_ref_)), font_size};
      return;
    }
    auto graphics_font = CTFontCopyGraphicsFont(font_ref_, nullptr);
    if (graphics_font == nullptr) {
      *info = FontInfo{0, 0, 0};
      return;
    }

    // Read metrics from the same underlying graphics font without constructing
    // a new CTFont. CGFont metrics are in font units; FontInfo stores a
    // negative ascent and a positive descent in layout units.
    const int units_per_em = CGFontGetUnitsPerEm(graphics_font);
    // Color fonts can use size-specific bitmap metrics rather than linear
    // scaling.
    const bool color_font =
        CTFontGetSymbolicTraits(font_ref_) & kCTFontColorGlyphsTrait;
    if (!color_font && units_per_em > 0 && font_size > 0 &&
        std::isfinite(font_size)) {
      const double scale = static_cast<double>(font_size) / units_per_em;
      *info =
          FontInfo{static_cast<float>(-CGFontGetAscent(graphics_font) * scale),
                   static_cast<float>(-CGFontGetDescent(graphics_font) * scale),
                   font_size};
      CFRelease(graphics_font);
      return;
    }

    // Preserve CoreText's size handling when direct scaling is unavailable.
    // Keep the descriptor null, as required for fonts with variation axes.
    auto ct_font = CTFontCreateWithGraphicsFont(graphics_font, font_size,
                                                nullptr, nullptr);
    CFRelease(graphics_font);
    *info = ct_font == nullptr
                ? FontInfo{0, 0, 0}
                : FontInfo{static_cast<float>(-CTFontGetAscent(ct_font)),
                           static_cast<float>(CTFontGetDescent(ct_font)),
                           font_size};
    if (ct_font != nullptr) {
      CFRelease(ct_font);
    }
  };

  CTFontRef GetFontRef() const { return font_ref_; }

 private:
  CTFontRef font_ref_ = nullptr;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_IOS_TYPEFACE_CORETEXT_H_
