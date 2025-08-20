// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_IOS_TYPEFACE_CORETEXT_H_
#define PUBLIC_TEXTRA_PLATFORM_IOS_TYPEFACE_CORETEXT_H_

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#include <textra/i_typeface_helper.h>
#include <textra/macro.h>
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

  // CTFontCreateCopyWithAttributes or CTFontCreateCopyWithSymbolicTraits cannot
  // be used on 10.10 and later, as they will return different underlying fonts
  // depending on the size requested. It is not possible to use descriptors with
  // CTFontCreateWithFontDescriptor, since that does not work with non-system
  // fonts. As a result, create the strike specific CTFonts from the underlying
  // CGFont.

  void OnCreateFontInfo(FontInfo* info, float font_size) const override {
    auto baseCGFont = CTFontCopyGraphicsFont(font_ref_, nullptr);
    // The last parameter (CTFontDescriptorRef attributes) *must* be nullptr.
    // If non-nullptr then with fonts with variation axes, the copy will fail in
    // CGFontVariationFromDictCallback when it assumes kCGFontVariationAxisName
    // is CFNumberRef which it quite obviously is not. Because we cannot setup
    // the CTFont descriptor to match, the same restriction applies here as
    // other uses of CTFontCreateWithGraphicsFont which is that such CTFonts
    // should not escape the scaler context, since they aren't 'normal'.

    auto ct_font =
        CTFontCreateWithGraphicsFont(baseCGFont, font_size, nullptr, nullptr);
    *info = ct_font == nullptr
                ? FontInfo{0, 0, 0}
                : FontInfo{static_cast<float>(-CTFontGetAscent(ct_font)),
                           static_cast<float>(CTFontGetDescent(ct_font)),
                           font_size};
    CFRelease(ct_font);
  };

  CTFontRef GetFontRef() const { return font_ref_; }

 private:
  CTFontRef font_ref_ = nullptr;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_IOS_TYPEFACE_CORETEXT_H_
