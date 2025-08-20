// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_SKSHAPER_FONT_H_
#define SRC_PORTS_SHAPER_SKSHAPER_FONT_H_

#include <textra/i_typeface_helper.h>

#include <memory>

#include "src/textlayout/utils/tt_rectf.h"

namespace ttoffice {
namespace tttext {
class Font final {
 public:
  Font() = delete;
  explicit Font(std::shared_ptr<ITypefaceHelper> typeface, float size)
      : typeface_(typeface), size_(size) {}
  ~Font() = default;

 public:
  const TypefaceRef GetTypeface() const { return typeface_; }
  float GetSize() const { return size_; }
  float GetHorizontalAdvance(GlyphID glyph_id) const {
    return typeface_->GetHorizontalAdvance(glyph_id, size_);
  }
  void GetHorizontalAdvances(GlyphID glyph_ids[], uint32_t count,
                             float widths[]) const {
    typeface_->GetHorizontalAdvances(glyph_ids, count, widths, size_);
  }
  void GetWidthBound(float* rect_ltwh, GlyphID glyph_id) const {
    typeface_->GetWidthBound(rect_ltwh, glyph_id, size_);
  }
  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variationSelector = 0) const {
    return typeface_->UnicharToGlyph(codepoint, variationSelector);
  }
  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const {
    typeface_->UnicharsToGlyphs(unichars, count, glyphs);
  }

  void GetFontInfo(FontInfo* info) const {
    *info = typeface_->GetFontInfo(size_);
  }

  void SetSubpixel(bool subpixel) { is_subpixel_ = subpixel; }
  bool IsSubpixel() const { return is_subpixel_; }
  void SetScaleX(float s) { scale_x_ = s; }
  float GetScaleX() const { return scale_x_; }
  void SetFakeBold(bool fake_bold) { fake_bold_ = fake_bold; }
  bool IsFakeBold() const { return fake_bold_; }
  void SetSkewX(float skew_x) { skew_x_ = skew_x; }
  float SkewX() const { return skew_x_; }

 private:
  std::shared_ptr<ITypefaceHelper> typeface_;
  float size_ = 24;
  bool is_subpixel_ = false;
  bool fake_bold_ = false;
  float scale_x_ = 1.0f;
  float skew_x_ = 0.f;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_SKSHAPER_FONT_H_
