// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/platform/lvgl/lvgl_typeface.h>

namespace ttoffice {
namespace tttext {
LVGLTypeface::LVGLTypeface(uint32_t unique_id) : ITypefaceHelper(unique_id) {}
LVGLTypeface::~LVGLTypeface() = default;
float LVGLTypeface::GetHorizontalAdvance(GlyphID glyph_id,
                                         float font_size) const {
  TTASSERT(false);
  return 0.0f;
}
void LVGLTypeface::GetHorizontalAdvances(GlyphID glyph_ids[], uint32_t count,
                                         float widths[],
                                         float font_size) const {
  TTASSERT(false);
}
void LVGLTypeface::GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                                 float font_size) const {
  TTASSERT(false);
}
void LVGLTypeface::GetWidthBounds(float* rect_ltrb, GlyphID glyphs[],
                                  uint32_t glyph_count, float font_size) {
  TTASSERT(false);
}
const void* LVGLTypeface::GetFontData() const { TTASSERT(false); }
size_t LVGLTypeface::GetFontDataSize() const { TTASSERT(false); }
int LVGLTypeface::GetFontIndex() const { TTASSERT(false); }
uint16_t LVGLTypeface::UnicharToGlyph(Unichar codepoint,
                                      uint32_t variationSelector) const {
  TTASSERT(false);
}
void LVGLTypeface::UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                                    GlyphID* glyphs) const {
  TTASSERT(false);
}
uint32_t LVGLTypeface::GetUnitsPerEm() const { TTASSERT(false); }
void LVGLTypeface::OnCreateFontInfo(FontInfo* info, float font_size) const {
  auto lv_font = GetLvFont(font_size);
  *info = FontInfo(lv_font->base_line - lv_font->line_height,
                   lv_font->base_line, font_size);
}
}  // namespace tttext
}  // namespace ttoffice
