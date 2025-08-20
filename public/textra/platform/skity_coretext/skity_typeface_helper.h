// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_TYPEFACE_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_TYPEFACE_HELPER_H_

#include <textra/i_typeface_helper.h>

#include <memory>
#include <skity/text/font.hpp>
#include <skity/text/typeface.hpp>
namespace skity {
namespace textlayout {
class SkityTypefaceHelper : public TTTypeface {
 public:
  explicit SkityTypefaceHelper(Typeface* typeface)
      : TTTypeface(typeface->TypefaceId()), typeface_(typeface) {}
  ~SkityTypefaceHelper() override = default;

 public:
  float GetHorizontalAdvance(GlyphID glyph_id, float font_size) const override {
    skity::Font font(typeface_, font_size);
    float sk_scalar;
    skity::GlyphID sk_glyph_id = glyph_id;
    font.GetWidths(&sk_glyph_id, 1, &sk_scalar, nullptr);
    return sk_scalar;
  }
  void GetHorizontalAdvances(GlyphID* glyph_ids, uint32_t count, float* widths,
                             float font_size) const override {
    skity::Font font(typeface_, font_size);
    font.GetWidths(glyph_ids, count, widths);
  }
  void GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                     float font_size) const override {
    skity::Font font(typeface_, font_size);
    skity::Rect rect;
    font.GetWidths(&glyph_id, 1, nullptr, &rect);
    rect_ltwh[0] = rect.Left();
    rect_ltwh[1] = rect.Top();
    rect_ltwh[2] = rect.Width();
    rect_ltwh[3] = rect.Height();
  }
  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variationSelector = 0) const override {
    return typeface_->UnicharToGlyph(codepoint);
  }
  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override {
    typeface_->UnicharsToGlyphs(unichars, count, glyphs);
  }
  const void* GetFontData() const override {
    return typeface_->GetData()->RawData();
  }
  size_t GetFontDataSize() const override {
    return typeface_->GetData()->Size();
  }
  int GetFontIndex() const override { return 0; }

  Typeface* GetTypeface() const { return typeface_; }
  uint32_t GetUnitsPerEm() const override { return typeface_->GetUnitsPerEm(); }

 protected:
  void OnCreateFontInfo(TTFontInfo* info, float font_size) const override {
    FontMetrics metrics;
    Font font(typeface_, font_size);
    font.GetMetrics(&metrics);
    info->SetAscent(metrics.ascent_);
    info->SetDescent(metrics.descent_);
    info->SetFontSize(font_size);
  }

 private:
  Typeface* typeface_;
  // int font_index_; not used for now.
};
}  // namespace textlayout
}  // namespace skity

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_TYPEFACE_HELPER_H_
