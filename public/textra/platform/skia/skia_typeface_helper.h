// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_TYPEFACE_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_TYPEFACE_HELPER_H_

#include <textra/i_typeface_helper.h>

#include <cassert>
#include <memory>
#include <vector>

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"

namespace ttoffice {
namespace tttext {
class SkiaTypefaceHelper : public ITypefaceHelper {
 public:
  SkiaTypefaceHelper() = delete;
  explicit SkiaTypefaceHelper(sk_sp<SkTypeface> sk_typeface)
      : ITypefaceHelper(sk_typeface->uniqueID()),
        ttc_index_(0),
        sk_typeface_(sk_typeface),
        asset_(sk_typeface_->openStream(&ttc_index_)) {
#ifdef TTTEXT_DEBUG
    SkString skname;
    sk_typeface_->getFamilyName(&skname);
    font_name_ = skname.c_str();
#endif
    assert(sk_typeface != nullptr);
  }
  ~SkiaTypefaceHelper() override = default;

 public:
  float GetHorizontalAdvance(GlyphID glyph_id, float font_size) const override {
    SkFont font(sk_typeface_, font_size);
    SkScalar width;
    font.getWidths(&glyph_id, 1, &width, nullptr);
    return width;
  }
  void GetHorizontalAdvances(GlyphID* glyph_ids, uint32_t count, float* widths,
                             float font_size) const override {
    SkFont font(sk_typeface_, font_size);
    font.getWidths(glyph_ids, count, widths);
  }
  void GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                     float font_size) const override {
    SkFont font(sk_typeface_, font_size);
    SkRect rect;
    font.getWidths(&glyph_id, 1, nullptr, &rect);
    rect_ltwh[0] = rect.left();
    rect_ltwh[1] = rect.top();
    rect_ltwh[2] = rect.width();
    rect_ltwh[3] = rect.height();
  }
  void GetWidthBounds(float* rect_ltrb, GlyphID glyphs[], uint32_t glyph_count,
                      float font_size) {}
  const void* GetFontData() const override {
    return asset_ == nullptr ? nullptr : asset_->getMemoryBase();
  }
  size_t GetFontDataSize() const override {
    return asset_ == nullptr ? 0 : asset_->getLength();
  }
  int GetFontIndex() const override { return 0; }
  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variationSelector = 0) const override {
    return sk_typeface_->unicharToGlyph(codepoint);
  }
  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override {
    std::vector<SkUnichar> sk_unichars(count);
    for (uint32_t k = 0; k < count; ++k) {
      sk_unichars[k] = unichars[k];
    }
    sk_typeface_->unicharsToGlyphs(sk_unichars.data(), count, glyphs);
  }
  uint32_t GetUnitsPerEm() const override {
    return sk_typeface_->getUnitsPerEm();
  }

 protected:
  void OnCreateFontInfo(tttext::FontInfo* info,
                        float font_size) const override {
    SkFont font(sk_typeface_, font_size);
    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    info->SetFontSize(font_size);
    //  info->SetFontId(sk_typeface_->uniqueID());
    info->SetAscent(metrics.fAscent);
    info->SetDescent(metrics.fDescent);
    info->SetLeading(metrics.fLeading);
    info->SetBottom(metrics.fBottom);
    info->SetTop(metrics.fTop);
  }

 public:
  sk_sp<SkTypeface> GetSkTypeface() const { return sk_typeface_; }

 private:
  int ttc_index_;
  sk_sp<SkTypeface> sk_typeface_;
  std::unique_ptr<SkStreamAsset> asset_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_TYPEFACE_HELPER_H_
