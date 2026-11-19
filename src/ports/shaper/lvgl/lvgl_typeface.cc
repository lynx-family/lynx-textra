// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/native/lvgl/lvgl_typeface.h"

namespace ttoffice {
namespace tttext {
LVGLTypeface::LVGLTypeface(uint32_t unique_id) : ITypefaceHelper(unique_id) {}
LVGLTypeface::~LVGLTypeface() = default;
float LVGLTypeface::GetHorizontalAdvance(GlyphID glyph_id,
                                         float font_size) const {
  lv_font_glyph_dsc_t desc{};
  return lv_font_get_glyph_dsc(GetLvFont(font_size), &desc, glyph_id, 0)
             ? desc.adv_w
             : 0;
}
void LVGLTypeface::GetHorizontalAdvances(GlyphID* glyphs, uint32_t count,
                                         float* widths, float size) const {
  auto font = GetLvFont(size);
  for (uint32_t i = 0; i < count; ++i) {
    lv_font_glyph_dsc_t d{};
    widths[i] = lv_font_get_glyph_dsc(font, &d, glyphs[i],
                                      i + 1 < count ? glyphs[i + 1] : 0)
                    ? d.adv_w
                    : 0;
  }
}
void LVGLTypeface::GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                                 float font_size) const {
  TTASSERT(false);
}
void LVGLTypeface::GetWidthBounds(float* rect_ltrb, GlyphID glyphs[],
                                  uint32_t glyph_count, float font_size) {
  TTASSERT(false);
}
const void* LVGLTypeface::GetFontData() const { return nullptr; }
size_t LVGLTypeface::GetFontDataSize() const { return 0; }
int LVGLTypeface::GetFontIndex() const { return 0; }
uint16_t LVGLTypeface::UnicharToGlyph(Unichar ch, uint32_t selector) const {
  if (selector || ch > 0xffff) return 0;
  lv_font_glyph_dsc_t d{};
  return lv_font_get_glyph_dsc(GetLvFont(16), &d, ch, 0) && !d.is_placeholder
             ? static_cast<uint16_t>(ch)
             : 0;
}
void LVGLTypeface::UnicharsToGlyphs(const Unichar* chars, uint32_t count,
                                    GlyphID* glyphs) const {
  for (uint32_t i = 0; i < count; ++i) glyphs[i] = UnicharToGlyph(chars[i], 0);
}
uint32_t LVGLTypeface::GetUnitsPerEm() const { return 0; }
void LVGLTypeface::OnCreateFontInfo(FontInfo* info, float font_size) const {
  auto lv_font = GetLvFont(font_size);
  *info = FontInfo(lv_font->base_line - lv_font->line_height,
                   lv_font->base_line, font_size);
}
}  // namespace tttext
}  // namespace ttoffice
