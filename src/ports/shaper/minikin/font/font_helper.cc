// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


#include "font_helper.h"

#include "shaper/minikin/lib/MinikinExtent.h"
#include "shaper/minikin/lib/MinikinPaint.h"
#include "shaper/minikin/lib/MinikinRect.h"

namespace minikin {

FontHelper::FontHelper(std::shared_ptr<ttoffice::ITypefaceHelper> typeface)
    : MinikinFont(typeface->GetUniqueId()), typeface_(typeface) {}

FontHelper::~FontHelper() = default;

float FontHelper::GetHorizontalAdvance(uint32_t glyph_id,
                                       const MinikinPaint &paint,
                                       const FontFakery &fakery) const {
  return typeface_->GetHorizontalAdvance(glyph_id, paint.size);
}

void FontHelper::GetBounds(MinikinRect *bounds, uint32_t glyph_id,
                           const MinikinPaint &paint,
                           const FontFakery &fakery) const {
  TTRectF rect_f;
  typeface_->GetWidthBound(rect_f, glyph_id, paint.size);
  bounds->mLeft = rect_f.GetLeft();
  bounds->mTop = rect_f.GetTop();
  bounds->mRight = rect_f.GetRight();
  bounds->mBottom = rect_f.GetBottom();
}

void FontHelper::GetFontExtent(MinikinExtent *extent, const MinikinPaint &paint,
                               const FontFakery &fakery) const {
  TTFontInfo info;
  typeface_->GetFontInfo(info, paint.size);
  extent->ascent = info.GetAscent();
  extent->descent = info.GetDescent();
}

std::shared_ptr<ttoffice::ITypefaceHelper> FontHelper::GetTypeface() const {
  return typeface_;
}

uint16_t FontHelper::UnicodeToGlyph(uint32_t codepoint,
                                    uint32_t variationSelector) const {
  return typeface_->UnicodeToGlyph(codepoint, variationSelector);
//  if (variationSelector == 0) {
//  } else {
//    ttoffice::GlyphID glyph_id[2];
//    ttoffice::Unichar unichars[2];
//    unichars[0] = codepoint;
//    unichars[1] = variationSelector;
//    //    typeface_->unicharsToGlyphs(unichars, 2, glyph_id);
//    assert(glyph_id[1] == 0);
//    return glyph_id[0] != 0;
//  }
}
void FontHelper::GetFontInfo(TTFontInfo &info, float font_size) const {
  typeface_->GetFontInfo(info, font_size);
}

}  // namespace minikin
