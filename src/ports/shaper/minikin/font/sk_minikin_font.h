// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


#ifndef TEXTLAYOUT_SRC_TEXTLAYOUT_SHAPER_SK_MINIKIN_FONT_H_
#define TEXTLAYOUT_SRC_TEXTLAYOUT_SHAPER_SK_MINIKIN_FONT_H_

#include "core/SkRefCnt.h"
#include "font_info.h"
#include "ports/shaper/minikin/lib/Macros.h"
#include "ports/shaper/minikin/lib/MinikinFont.h"
#include "painter.h"

class SkTypeface;

namespace minikin {
class SkMinikinFont : public MinikinFont {
 public:
  explicit SkMinikinFont(sk_sp<SkTypeface>& typeface);
  ~SkMinikinFont() override;

  // MinikinFont overrides.
  float GetHorizontalAdvance(uint32_t glyph_id, const MinikinPaint& paint,
                             const FontFakery& fakery) const override;
  void GetBounds(MinikinRect* bounds, uint32_t glyph_id,
                 const MinikinPaint& paint,
                 const FontFakery& fakery) const override;
  void GetFontExtent(MinikinExtent* extent, const MinikinPaint& paint,
                     const FontFakery& fakery) const override;

  const void* GetFontData() const override { return font_data_; }
  size_t GetFontSize() const override { return font_size_; }
  int GetFontIndex() const override { return font_index_; }
  const std::vector<minikin::FontVariation>& GetAxes() const override {
    return axes_;
  }

 public:
  sk_sp<SkTypeface> GetSkTypeface();
  uint16_t UnicodeToGlyph(uint32_t codepoint,
                          uint32_t variationSelector) const override;
  void GetFontInfo(TTFontInfo* info, float font_size) const override;

 private:
  sk_sp<SkTypeface> typeface_;
  const void* font_data_;
  size_t font_size_;
  int font_index_;
  std::vector<minikin::FontVariation> axes_;

  MINIKIN_PREVENT_COPY_AND_ASSIGN(SkMinikinFont);
};
}  // namespace minikin

#endif  // TEXTLAYOUT_SRC_TEXTLAYOUT_SHAPER_SK_MINIKIN_FONT_H_
