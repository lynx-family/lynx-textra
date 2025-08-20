// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.



#include "sk_minikin_font.h"


#include "core/SkFont.h"
#include "core/SkFontMetrics.h"
#include "core/SkPaint.h"
#include "core/SkStream.h"
#include "core/SkTypeface.h"
#include "ports/shaper/minikin/lib/MinikinExtent.h"
#include "ports/shaper/minikin/lib/MinikinPaint.h"
#include "ports/shaper/minikin/lib/MinikinRect.h"

namespace minikin {
SkMinikinFont::SkMinikinFont(sk_sp<SkTypeface> &typeface)
    : MinikinFont(typeface->uniqueID()), typeface_(std::move(typeface)) {
  auto stream = typeface_->openStream(&font_index_);
  font_data_ = stream->getMemoryBase();
  font_size_ = stream->getLength();
}
SkMinikinFont::~SkMinikinFont() = default;
float SkMinikinFont::GetHorizontalAdvance(uint32_t glyph_id,
                                          const MinikinPaint &paint,
                                          const FontFakery &fakery) const {
  SkFont font(typeface_, paint.size);
  SkScalar sk_scalar;
  SkGlyphID sk_glyph_id = glyph_id;
  font.getWidthsBounds(&sk_glyph_id, 1, &sk_scalar, nullptr, nullptr);
  return sk_scalar;
}
void SkMinikinFont::GetBounds(MinikinRect *bounds, uint32_t glyph_id,
                              const MinikinPaint &paint,
                              const FontFakery &fakery) const {
  SkFont font(typeface_, paint.size);
  SkRect sk_bounds;
  SkGlyphID sk_glyph_id = glyph_id;
  font.getBounds(&sk_glyph_id, 1, &sk_bounds, nullptr);
  bounds->mLeft = sk_bounds.left();
  bounds->mTop = sk_bounds.top();
  bounds->mRight = sk_bounds.right();
  bounds->mBottom = sk_bounds.bottom();
}
void SkMinikinFont::GetFontExtent(MinikinExtent *extent,
                                  const MinikinPaint &paint,
                                  const FontFakery &fakery) const {
  SkFont font(typeface_, paint.size);
  SkFontMetrics metrics;
  font.getMetrics(&metrics);
  extent->ascent = metrics.fAscent;
  extent->descent = metrics.fDescent;
}
sk_sp<SkTypeface> SkMinikinFont::GetSkTypeface() { return typeface_; }
uint16_t SkMinikinFont::UnicodeToGlyph(uint32_t codepoint,
                                       uint32_t variationSelector) const {
  if (variationSelector == 0) {
    return typeface_->unicharToGlyph(codepoint);
  } else {
    SkGlyphID glyph_id[2];
    SkUnichar unichars[2];
    unichars[0] = codepoint;
    unichars[1] = variationSelector;
    typeface_->unicharsToGlyphs(unichars, 2, glyph_id);
    TTASSERT(glyph_id[1] == 0);
    return glyph_id[0] != 0;
  }
}
void SkMinikinFont::GetFontInfo(TTFontInfo *info, float font_size) const {
  SkFont font(typeface_, font_size);
  SkFontMetrics metrics;
  font.getMetrics(&metrics);
  //  info->SetTop(metrics.fTop);
  //  info->SetLeading(metrics.fLeading);
  info->SetAscent(metrics.fAscent);
  info->SetDescent(metrics.fDescent);
  //  info->SetBottom(metrics.fBottom);
  info->SetFontSize(font_size);
  info->SetFontId(GetUniqueId());
}
}  // namespace minikin