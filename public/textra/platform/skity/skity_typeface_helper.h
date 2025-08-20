// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_TYPEFACE_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_TYPEFACE_HELPER_H_

#include <textra/i_typeface_helper.h>

#include <memory>
#include <skity/text/font.hpp>
#include <skity/text/typeface.hpp>
#include <utility>
namespace skity {
namespace textlayout {
class SkityTypefaceHelper : public tttext::ITypefaceHelper {
 public:
  explicit SkityTypefaceHelper(std::shared_ptr<Typeface> typeface)
      : tttext::ITypefaceHelper(typeface->TypefaceId()),
        typeface_(std::move(typeface)) {
    skity::FontStyle skity_font_style = typeface_->GetFontStyle();
    font_style_ = tttext::FontStyle(
        static_cast<tttext::FontStyle::Weight>(skity_font_style.weight()),
        static_cast<tttext::FontStyle::Width>(skity_font_style.width()),
        static_cast<tttext::FontStyle::Slant>(skity_font_style.slant()));
  }
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
  void GetWidthBounds(float* rect_ltrb, tttext::GlyphID glyphs[],
                      uint32_t glyph_count, float font_size) override {
    skity::Font font(typeface_, font_size);
    skity::Rect* rect = new skity::Rect[glyph_count];
    font.GetWidths(glyphs, glyph_count, nullptr, rect);
    auto r = 0.f;
    for (auto k = 0u; k < glyph_count; k++) {
      rect_ltrb[1] = std::fmin(rect_ltrb[1], rect[k].Top());
      rect_ltrb[3] = std::fmax(rect_ltrb[3], rect[k].Bottom());
      r += rect[k].Width();
    }
    rect_ltrb[0] = rect[0].Left();
    rect_ltrb[2] = r;
    delete[] rect;
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

  std::shared_ptr<Typeface> GetTypeface() const { return typeface_; }
  uint32_t GetUnitsPerEm() const override { return typeface_->GetUnitsPerEm(); }

 protected:
  void OnCreateFontInfo(tttext::FontInfo* info,
                        float font_size) const override {
    FontMetrics metrics;
    Font font(typeface_, font_size);
    font.GetMetrics(&metrics);
    info->SetAscent(metrics.ascent_);
    info->SetDescent(metrics.descent_);
    info->SetFontSize(font_size);
  }

 private:
  std::shared_ptr<Typeface> typeface_;
  // int font_index_; not used for now.
};
}  // namespace textlayout
}  // namespace skity

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_TYPEFACE_HELPER_H_
