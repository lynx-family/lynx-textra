// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_TYPEFACE_HELPER_H_
#define SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_TYPEFACE_HELPER_H_

#include <native_drawing/drawing_font.h>
#include <native_drawing/drawing_types.h>
#include <textra/i_typeface_helper.h>
#include <textra/macro.h>

namespace ttoffice {
namespace tttext {
class AGTypefaceHelper : public ITypefaceHelper {
 public:
  explicit AGTypefaceHelper(OH_Drawing_Font* font)
      : ITypefaceHelper(0), font_(font) {}
  ~AGTypefaceHelper() override = default;

 public:
  float GetHorizontalAdvance(GlyphID glyph_id, float font_size) const override {
    TTASSERT(false);
    return 0;
  }
  void GetHorizontalAdvances(GlyphID* glyph_ids, uint32_t count, float* widths,
                             float font_size) const override {
    TTASSERT(false);
  }
  void GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                     float font_size) const override {
    TTASSERT(false);
  }
  void GetWidthBounds(float* rect_ltrb, GlyphID glyphs[], uint32_t glyph_count,
                      float font_size) override {
    TTASSERT(false);
  }
  const void* GetFontData() const override { return nullptr; }
  size_t GetFontDataSize() const override { return 0; }
  int GetFontIndex() const override { return 0; }
  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variationSelector = 0) const override {
    GlyphID glyph;
    OH_Drawing_FontTextToGlyphs(font_, &codepoint, 4, TEXT_ENCODING_UTF32,
                                &glyph, 1);
    return glyph;
  }
  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override {
    OH_Drawing_FontTextToGlyphs(font_, unichars, 4 * count, TEXT_ENCODING_UTF32,
                                glyphs, count);
  }
  uint32_t GetUnitsPerEm() const override { return 0; }
  void OnCreateFontInfo(FontInfo* info, float font_size) const override {
    OH_Drawing_Font_Metrics metrics{};
    OH_Drawing_FontSetTextSize(font_, font_size);
    OH_Drawing_FontGetMetrics(font_, &metrics);
    info->SetAscent(metrics.ascent);
    info->SetDescent(metrics.descent);
    info->SetLeading(metrics.leading);
    info->SetFontSize(font_size);
  }

  OH_Drawing_Font* GetTypeface() const { return font_; }

 private:
  OH_Drawing_Font* font_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_TYPEFACE_HELPER_H_
