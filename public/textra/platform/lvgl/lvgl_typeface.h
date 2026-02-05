// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_LVGL_LVGL_TYPEFACE_H_
#define PUBLIC_TEXTRA_PLATFORM_LVGL_LVGL_TYPEFACE_H_
#include <lvgl/src/font/lv_font.h>

#include "textra/i_typeface_helper.h"

namespace ttoffice {
namespace tttext {
class LVGLTypeface : public ITypefaceHelper {
 public:
  explicit LVGLTypeface(uint32_t unique_id);
  ~LVGLTypeface() override;

 public:
  float GetHorizontalAdvance(GlyphID glyph_id, float font_size) const override;
  void GetHorizontalAdvances(GlyphID glyph_ids[], uint32_t count,
                             float widths[], float font_size) const override;
  void GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                     float font_size) const override;
  void GetWidthBounds(float* rect_ltrb, GlyphID glyphs[], uint32_t glyph_count,
                      float font_size) override;
  const void* GetFontData() const override;
  size_t GetFontDataSize() const override;
  int GetFontIndex() const override;
  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variationSelector) const override;
  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override;
  uint32_t GetUnitsPerEm() const override;

  virtual const lv_font_t* GetLvFont(float font_size) const {
    return &lv_font_source_han_sans_sc_16_cjk;
  }

 protected:
  void OnCreateFontInfo(FontInfo* info, float font_size) const override;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_LVGL_LVGL_TYPEFACE_H_
