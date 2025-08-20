// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/ark_graphics/shaper_ark_graphics.h"

#include <native_drawing/drawing_font.h>
#include <textra/i_typeface_helper.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "src/ports/renderer/ark_graphics/ag_typeface_helper.h"

namespace ttoffice {
namespace tttext {
class AGShapingResult : public PlatformShapingResultReader {
 public:
  AGShapingResult() {}
  ~AGShapingResult() override = default;

 public:
  uint32_t GlyphCount() const override {
    return static_cast<uint32_t>(ct_glyphs_.size());
  }
  uint32_t TextCount() const override { return text_length_; }
  GlyphID ReadGlyphID(uint32_t idx) const override { return ct_glyphs_[idx]; }
  float ReadAdvanceX(uint32_t idx) const override {
    return ct_advances_[idx][0];
  }
  float ReadAdvanceY(uint32_t idx) const override {
    return ct_advances_[idx][1];
  }
  float ReadPositionX(uint32_t idx) const override {
    return ct_position_[idx][0];
  }
  float ReadPositionY(uint32_t idx) const override {
    return ct_position_[idx][1];
  }
  uint32_t ReadIndices(uint32_t idx) const override {
    return static_cast<uint32_t>(ct_indices_[idx]);
  }
  TypefaceRef ReadFontId(uint32_t idx) const override { return typeface_[idx]; }

 public:
  std::vector<GlyphID> ct_glyphs_;
  std::vector<std::array<int, 2>> ct_advances_;
  std::vector<uint32_t> ct_indices_;
  std::vector<std::array<float, 2>> ct_position_;
  std::vector<TypefaceRef> typeface_;
  uint32_t text_length_{0};
};

void ShaperArkGraphics::OnShapeText(const ShapeKey& key,
                                    ShapeResult* result) const {
  auto font = OH_Drawing_FontCreate();
  OH_Drawing_FontSetTextSize(font, key.style_.GetFontSize());
  auto tf_helper = std::make_shared<AGTypefaceHelper>(font);
  auto text_len = static_cast<uint32_t>(OH_Drawing_FontCountText(
      font, key.text_.data(), key.text_.length() * 4, TEXT_ENCODING_UTF32));
  TTASSERT(text_len == static_cast<uint32_t>(key.text_.length()));
  AGShapingResult shaping_result;
  shaping_result.ct_advances_.resize(text_len);
  shaping_result.ct_glyphs_.resize(text_len);
  shaping_result.ct_indices_.resize(text_len);
  shaping_result.ct_position_.resize(text_len, {0, 0});
  shaping_result.typeface_.resize(text_len, tf_helper);

  auto* glyphs = shaping_result.ct_glyphs_.data();
  std::vector<float> glyph_advances(text_len);
  auto glyph_count = OH_Drawing_FontTextToGlyphs(
      font, key.text_.data(), static_cast<uint32_t>(key.text_.length() * 4),
      TEXT_ENCODING_UTF32, glyphs, text_len);
  TTASSERT(glyph_count == text_len);
  OH_Drawing_FontGetWidths(font, glyphs, glyph_count, glyph_advances.data());

  auto advance_x = 0.f;
  for (auto k = 0u; k < text_len; k++) {
    shaping_result.ct_indices_[k] = k;
    shaping_result.ct_position_[k][0] = advance_x;
    shaping_result.ct_position_[k][1] = 0;
    shaping_result.ct_advances_[k][0] = glyph_advances[k];
    shaping_result.ct_advances_[k][1] = 0;
    advance_x += glyph_advances[k];
  }
  shaping_result.text_length_ = text_len;
  result->AppendPlatformShapingResult(shaping_result);
}
}  // namespace tttext
}  // namespace ttoffice
