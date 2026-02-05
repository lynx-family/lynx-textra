// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/lvgl/lvgl_shaper.h"

#include <lvgl/src/font/lv_font.h>
#include <lvgl/src/font/lv_font_fmt_txt.h>
#include <textra/platform/lvgl/lvgl_typeface.h>

#include <memory>
#include <vector>

#include "src/ports/shaper/lvgl/lvgl_typeface_cjk.h"
#include "src/ports/shaper/lvgl/lvgl_typeface_montserrat.h"
#include "src/textlayout/utils/log_util.h"
#if LV_ENABLE_GLOBAL_CUSTOM
#include <third_party/lvgl/src/core/lv_global.h>
#ifdef __cplusplus
extern "C" {
#endif
static lv_global_t global;
lv_global_t* lv_global_default(void) { return &global; }
#ifdef __cplusplus
}
#endif
#endif
namespace ttoffice {
namespace tttext {
class LVGLShapingResult final : public PlatformShapingResultReader {
 public:
  ~LVGLShapingResult() override = default;
  uint32_t GlyphCount() const override {
    return static_cast<uint32_t>(glyphs_.size());
  }
  uint32_t TextCount() const override { return text_count_; }
  GlyphID ReadGlyphID(uint32_t idx) const override { return glyphs_[idx]; }
  float ReadAdvanceX(uint32_t idx) const override { return advances_[idx]; }
  uint32_t ReadIndices(uint32_t idx) const override { return indexes_[idx]; }
  TypefaceRef ReadFontId(uint32_t idx) const override { return font_lst_[idx]; }
  float ReadPositionX(uint32_t idx) const override {
    return advances_[idx] / 2;
  }
  std::vector<float> advances_;
  std::vector<GlyphID> glyphs_;
  std::vector<uint32_t> indexes_;
  uint32_t text_count_;
  std::vector<TypefaceRef> font_lst_;
};

LVGLShaper::LVGLShaper(FontmgrCollection font_collection) noexcept
    : TTShaper(font_collection) {}

void LVGLShaper::OnShapeText(const ShapeKey& key, ShapeResult* result) const {
  LVGLShapingResult reader;
  auto text = key.text_.c_str();
  auto text_len = key.text_.length();
  std::vector<std::shared_ptr<LVGLTypeface>> font_mgr_;
  font_mgr_.push_back(std::make_shared<LVGLTypefaceMontserrat>(0));
  font_mgr_.push_back(std::make_shared<LVGLTypefaceCJK>(1));
  // auto typeface = std::make_shared<LVGLTypefaceMontserrat>(0);
  // auto lv_font = typeface->GetLvFont(key.style_.GetFontSize());
  reader.advances_.reserve(text_len);
  reader.glyphs_.reserve(text_len);
  reader.font_lst_.reserve(text_len);

  auto& u16str = reader.glyphs_;
  auto& indexes = reader.indexes_;
  uint32_t char32_index = 0;
  for (auto utf32 : key.text_) {
    if (utf32 <= 0xD7FF || (utf32 >= 0xE000 && utf32 <= 0xFFFF)) {
      u16str.push_back(static_cast<GlyphID>(utf32));
      indexes.push_back(char32_index);
    } else if (utf32 >= 0x10000 && utf32 <= 0x10FFFF) {
      utf32 -= 0x10000;
      u16str.push_back(static_cast<char16_t>(0xD800 | ((utf32 >> 10) & 0x3FF)));
      u16str.push_back(static_cast<char16_t>(0xDC00 | (utf32 & 0x3FF)));
      indexes.push_back(char32_index);
      indexes.push_back(char32_index);
    } else {
      LogUtil::E("U32StringToU16 Invalid UTF-32 encoding");
    }
    char32_index++;
  }
  size_t iter = 0;
  lv_font_glyph_dsc_t font_desc;
  while (iter < text_len) {
    for (auto& typeface : font_mgr_) {
      auto lv_font = typeface->GetLvFont(key.style_.GetFontSize());
      auto ret = lv_font_get_glyph_dsc_fmt_txt(
          lv_font, &font_desc, text[iter],
          iter < text_len - 1 ? text[iter + 1] : 0);
      if (ret) {
        reader.font_lst_.push_back(typeface);
        break;
      }
      font_desc.adv_w = 0;
    }
    reader.advances_.emplace_back(font_desc.adv_w);
    iter++;
  }
  reader.text_count_ = static_cast<uint32_t>(text_len);
  result->AppendPlatformShapingResult(reader);
}
}  // namespace tttext
}  // namespace ttoffice
