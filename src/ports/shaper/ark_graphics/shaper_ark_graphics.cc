// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/ark_graphics/shaper_ark_graphics.h"

#include <dlfcn.h>
#include <native_drawing/drawing_font.h>
#include <native_drawing/drawing_point.h>
#include <native_drawing/drawing_text_line.h>
#include <native_drawing/drawing_text_lineTypography.h>
#include <native_drawing/drawing_text_run.h>
#include <native_drawing/drawing_text_typography.h>
#include <textra/i_typeface_helper.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "src/ports/renderer/ark_graphics/ag_typeface_helper.h"

namespace ttoffice {
namespace tttext {

// clang-format off
using OHOS_CreateLineTypography = OH_Drawing_LineTypography* (*)(OH_Drawing_TypographyCreate*);
using OHOS_LineTypographyCreateLine = OH_Drawing_TextLine* (*)(OH_Drawing_LineTypography*, size_t, size_t);
using OHOS_TextLineGetGlyphCount = double (*)(OH_Drawing_TextLine*);
using OHOS_TextLineGetGlyphRuns = OH_Drawing_Array* (*)(OH_Drawing_TextLine*);
using OHOS_GetDrawingArraySize = size_t (*)(OH_Drawing_Array*);
using OHOS_GetRunByIndex = OH_Drawing_Run* (*)(OH_Drawing_Array*, size_t);
using OHOS_GetRunGlyphCount = uint32_t (*)(OH_Drawing_Run*);
using OHOS_GetRunGlyphs = OH_Drawing_Array* (*)(OH_Drawing_Run*, int64_t, int64_t);
using OHOS_GetRunGlyphAdvances = OH_Drawing_Array* (*)(OH_Drawing_Run*, uint32_t, uint32_t);
using OHOS_GetRunPositions = OH_Drawing_Array* (*)(OH_Drawing_Run*, int64_t, int64_t);
using OHOS_GetRunStringIndices = OH_Drawing_Array* (*)(OH_Drawing_Run*, int64_t, int64_t);
using OHOS_GetRunFont = OH_Drawing_Font* (*)(OH_Drawing_Run*);
using OHOS_GetRunGlyphsByIndex = uint16_t (*)(OH_Drawing_Array*, size_t);
using OHOS_GetRunPositionsByIndex = OH_Drawing_Point* (*)(OH_Drawing_Array*, size_t);
using OHOS_GetRunStringIndicesByIndex = uint64_t (*)(OH_Drawing_Array*, size_t);
using OHOS_GetRunGlyphAdvanceByIndex = OH_Drawing_Point* (*)(OH_Drawing_Array*, size_t);
using OHOS_DestroyRunStringIndices = void (*)(OH_Drawing_Array*);
using OHOS_DestroyRunPositions = void (*)(OH_Drawing_Array*);
using OHOS_DestroyRunGlyphs = void (*)(OH_Drawing_Array*);
using OHOS_DestroyRuns = void (*)(OH_Drawing_Array*);
using OHOS_DestroyTextLine = void (*)(OH_Drawing_TextLine*);
using OHOS_DestroyLineTypography = void (*)(OH_Drawing_LineTypography*);
using OHOS_TypographyHandlerAddEncodedText = void (*)(OH_Drawing_TypographyCreate*, const void*,size_t, OH_Drawing_TextEncoding);
// clang-format on

struct OHOS_ShapingFuncPtr {
  OHOS_CreateLineTypography CreateLineTypography_;
  OHOS_LineTypographyCreateLine LineTypographyCreateLine_;
  OHOS_TextLineGetGlyphCount TextLineGetGlyphCount_;
  OHOS_TextLineGetGlyphRuns TextLineGetGlyphRuns_;
  OHOS_GetDrawingArraySize GetDrawingArraySize_;
  OHOS_GetRunByIndex GetRunByIndex_;
  OHOS_GetRunGlyphCount GetRunGlyphCount_;
  OHOS_GetRunGlyphs GetRunGlyphs_;
  OHOS_GetRunGlyphAdvances GetRunGlyphAdvances_;
  OHOS_GetRunPositions GetRunPositions_;
  OHOS_GetRunStringIndices GetRunStringIndices_;
  OHOS_GetRunFont GetRunFont_;
  OHOS_GetRunGlyphsByIndex GetRunGlyphsByIndex_;
  OHOS_GetRunPositionsByIndex GetRunPositionsByIndex_;
  OHOS_GetRunStringIndicesByIndex GetRunStringIndicesByIndex_;
  OHOS_GetRunGlyphAdvanceByIndex GetRunGlyphAdvanceByIndex_;
  OHOS_DestroyRunStringIndices DestroyRunStringIndices_;
  OHOS_DestroyRunPositions DestroyRunPositions_;
  OHOS_DestroyRunGlyphs DestroyRunGlyphs_;
  OHOS_DestroyRuns DestroyRuns_;
  OHOS_DestroyTextLine DestroyTextLine_;
  OHOS_DestroyLineTypography DestroyLineTypography_;
  OHOS_TypographyHandlerAddEncodedText TypographyHandlerAddEncodedText_;
};
static std::unique_ptr<OHOS_ShapingFuncPtr> ohos_shaping_funcs_ = nullptr;
class AGShapingResult : public PlatformShapingResultReader {
 public:
  AGShapingResult() = default;
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
  std::vector<std::array<float, 2>> ct_advances_;
  std::vector<uint32_t> ct_indices_;
  std::vector<std::array<float, 2>> ct_position_;
  std::vector<TypefaceRef> typeface_;
  uint32_t text_length_{0};
};
ShaperArkGraphics::ShaperArkGraphics(
    const FontmgrCollection& font_collections) noexcept
    : TTShaper(font_collections),
      shared_font_collection_(OH_Drawing_CreateSharedFontCollection()) {
  typography_style_ = OH_Drawing_CreateTypographyStyle();

  if (ohos_shaping_funcs_ == nullptr) {
    ohos_shaping_funcs_ = std::make_unique<OHOS_ShapingFuncPtr>();
    void* handle = dlopen("libnative_drawing.so", RTLD_LAZY);
    if (handle) {
      // clang-format off
      ohos_shaping_funcs_->CreateLineTypography_ = (OHOS_CreateLineTypography)dlsym(handle,"OH_Drawing_CreateLineTypography");
      ohos_shaping_funcs_->LineTypographyCreateLine_ = (OHOS_LineTypographyCreateLine)dlsym(handle,"OH_Drawing_LineTypographyCreateLine");
      ohos_shaping_funcs_->TextLineGetGlyphCount_ = (OHOS_TextLineGetGlyphCount)dlsym(handle,"OH_Drawing_TextLineGetGlyphCount");
      ohos_shaping_funcs_->TextLineGetGlyphRuns_ = (OHOS_TextLineGetGlyphRuns)dlsym(handle,"OH_Drawing_TextLineGetGlyphRuns");
      ohos_shaping_funcs_->GetDrawingArraySize_ = (OHOS_GetDrawingArraySize)dlsym(handle,"OH_Drawing_GetDrawingArraySize");
      ohos_shaping_funcs_->GetRunByIndex_ = (OHOS_GetRunByIndex)dlsym(handle,"OH_Drawing_GetRunByIndex");
      ohos_shaping_funcs_->GetRunGlyphCount_ = (OHOS_GetRunGlyphCount)dlsym(handle,"OH_Drawing_GetRunGlyphCount");
      ohos_shaping_funcs_->GetRunGlyphs_ = (OHOS_GetRunGlyphs)dlsym(handle,"OH_Drawing_GetRunGlyphs");
      ohos_shaping_funcs_->GetRunGlyphAdvances_ = (OHOS_GetRunGlyphAdvances)dlsym(handle,"OH_Drawing_GetRunGlyphAdvances");
      ohos_shaping_funcs_->GetRunPositions_ = (OHOS_GetRunPositions)dlsym(handle,"OH_Drawing_GetRunPositions");
      ohos_shaping_funcs_->GetRunStringIndices_ = (OHOS_GetRunStringIndices)dlsym(handle,"OH_Drawing_GetRunStringIndices");
      ohos_shaping_funcs_->GetRunFont_ = (OHOS_GetRunFont)dlsym(handle,"OH_Drawing_GetRunFont");
      ohos_shaping_funcs_->GetRunGlyphsByIndex_ = (OHOS_GetRunGlyphsByIndex)dlsym(handle,"OH_Drawing_GetRunGlyphsByIndex");
      ohos_shaping_funcs_->GetRunPositionsByIndex_ = (OHOS_GetRunPositionsByIndex)dlsym(handle,"OH_Drawing_GetRunPositionsByIndex");
      ohos_shaping_funcs_->GetRunStringIndicesByIndex_ = (OHOS_GetRunStringIndicesByIndex)dlsym(handle,"OH_Drawing_GetRunStringIndicesByIndex");
      ohos_shaping_funcs_->GetRunGlyphAdvanceByIndex_ = (OHOS_GetRunGlyphAdvanceByIndex)dlsym(handle,"OH_Drawing_GetRunGlyphAdvanceByIndex");
      ohos_shaping_funcs_->DestroyRunStringIndices_ = (OHOS_DestroyRunStringIndices)dlsym(handle,"OH_Drawing_DestroyRunStringIndices");
      ohos_shaping_funcs_->DestroyRunPositions_ = (OHOS_DestroyRunPositions)dlsym(handle,"OH_Drawing_DestroyRunPositions");
      ohos_shaping_funcs_->DestroyRunGlyphs_ = (OHOS_DestroyRunGlyphs)dlsym(handle,"OH_Drawing_DestroyRunGlyphs");
      ohos_shaping_funcs_->DestroyRuns_ = (OHOS_DestroyRuns)dlsym(handle,"OH_Drawing_DestroyRuns");
      ohos_shaping_funcs_->DestroyTextLine_ = (OHOS_DestroyTextLine)dlsym(handle,"OH_Drawing_DestroyTextLine");
      ohos_shaping_funcs_->DestroyLineTypography_ = (OHOS_DestroyLineTypography)dlsym(handle,"OH_Drawing_DestroyLineTypography");
      ohos_shaping_funcs_->TypographyHandlerAddEncodedText_ = (OHOS_TypographyHandlerAddEncodedText)dlsym(handle,"OH_Drawing_TypographyHandlerAddEncodedText");
      // clang-format on
    }
  }
}
ShaperArkGraphics::~ShaperArkGraphics() {
  OH_Drawing_DestroyTypographyStyle(typography_style_);
  OH_Drawing_DestroyFontCollection(shared_font_collection_);
}

void ShaperArkGraphics::OnShapeText(const ShapeKey& key,
                                    ShapeResult* result) const {
  if (ohos_shaping_funcs_->GetRunFont_ != NULL) {
    ShapingTextWithHighAPILevel(key, result);
  } else {
    ShapingTextWithLowAPILevel(key, result);
  }
}

void ShaperArkGraphics::ShapingTextWithHighAPILevel(const ShapeKey& key,
                                                    ShapeResult* result) const {
  auto typography_handler = OH_Drawing_CreateTypographyHandler(
      OH_Drawing_CreateTypographyStyle(), shared_font_collection_);
  auto text_style = OH_Drawing_CreateTextStyle();
  OH_Drawing_SetTextStyleFontSize(text_style, key.style_.GetFontSize());
  auto& fd = key.style_.GetFontDescriptor();
  const char** families = new const char*[fd.font_family_list_.size()];
  for (int k = 0; k < fd.font_family_list_.size(); k++) {
    families[k] = fd.font_family_list_[k].c_str();
  }
  OH_Drawing_SetTextStyleFontFamilies(
      text_style, static_cast<int32_t>(fd.font_family_list_.size()), families);
  if (fd.font_style_.GetSlant() != FontStyle::kUpright_Slant)
    OH_Drawing_SetTextStyleFontStyle(text_style, FONT_STYLE_ITALIC);
  OH_Drawing_SetTextStyleFontWeight(text_style,
                                    (int)(fd.font_style_.GetWeight() / 100));
  OH_Drawing_TypographyHandlerPushTextStyle(typography_handler, text_style);
  ohos_shaping_funcs_->TypographyHandlerAddEncodedText_(
      typography_handler, key.text_.c_str(), key.text_.length() * 4,
      TEXT_ENCODING_UTF32);
  auto line_typo =
      ohos_shaping_funcs_->CreateLineTypography_(typography_handler);
  auto line = ohos_shaping_funcs_->LineTypographyCreateLine_(
      line_typo, 0, key.text_.length());
  uint32_t glyph_count =
      static_cast<uint32_t>(ohos_shaping_funcs_->TextLineGetGlyphCount_(line));
  AGShapingResult shaping_result;
  shaping_result.ct_advances_.resize(glyph_count);
  shaping_result.ct_glyphs_.resize(glyph_count);
  shaping_result.ct_indices_.resize(glyph_count);
  shaping_result.ct_position_.resize(glyph_count, {0, 0});
  shaping_result.typeface_.resize(glyph_count);

  auto glyph_runs = ohos_shaping_funcs_->TextLineGetGlyphRuns_(line);
  auto glyph_runs_count = OH_Drawing_GetDrawingArraySize(glyph_runs);
  auto glyph_idx = 0;
  for (auto k = 0; k < glyph_runs_count; k++) {
    auto* run = ohos_shaping_funcs_->GetRunByIndex_(glyph_runs, k);
    auto glyph_cnt = ohos_shaping_funcs_->GetRunGlyphCount_(run);
    auto glyph_array = ohos_shaping_funcs_->GetRunGlyphs_(run, 0, glyph_cnt);
    auto advance_array =
        ohos_shaping_funcs_->GetRunGlyphAdvances_(run, 0, glyph_cnt);
    auto position_array =
        ohos_shaping_funcs_->GetRunPositions_(run, 0, glyph_cnt);
    auto indices_array =
        ohos_shaping_funcs_->GetRunStringIndices_(run, 0, glyph_cnt);
    auto font = ohos_shaping_funcs_->GetRunFont_(run);
    auto tf_helper = std::make_shared<AGTypefaceHelper>(font);

    for (auto j = 0; j < glyph_cnt; j++) {
      shaping_result.ct_glyphs_[glyph_idx + j] =
          ohos_shaping_funcs_->GetRunGlyphsByIndex_(glyph_array, j);
      auto pos =
          ohos_shaping_funcs_->GetRunPositionsByIndex_(position_array, j);
      auto& ppos = shaping_result.ct_position_[glyph_idx + j];
      OH_Drawing_PointGetX(pos, &ppos[0]);
      OH_Drawing_PointGetY(pos, &ppos[1]);
      shaping_result.ct_indices_[glyph_idx + j] = static_cast<uint32_t>(
          ohos_shaping_funcs_->GetRunStringIndicesByIndex_(indices_array, j));
      auto adv =
          ohos_shaping_funcs_->GetRunGlyphAdvanceByIndex_(advance_array, j);
      auto& aadv = shaping_result.ct_advances_[glyph_idx + j];
      OH_Drawing_PointGetX(adv, &aadv[0]);
      shaping_result.typeface_[glyph_idx + j] = tf_helper;
    }
    glyph_idx += glyph_cnt;
    ohos_shaping_funcs_->DestroyRunStringIndices_(indices_array);
    ohos_shaping_funcs_->DestroyRunPositions_(position_array);
    ohos_shaping_funcs_->DestroyRunGlyphs_(glyph_array);
  }
  delete[] families;

  ohos_shaping_funcs_->DestroyRuns_(glyph_runs);
  ohos_shaping_funcs_->DestroyTextLine_(line);
  ohos_shaping_funcs_->DestroyLineTypography_(line_typo);
  OH_Drawing_DestroyTypographyHandler(typography_handler);

  shaping_result.text_length_ = static_cast<uint32_t>(key.text_.length());
  result->AppendPlatformShapingResult(shaping_result);
}

void ShaperArkGraphics::ShapingTextWithLowAPILevel(const ShapeKey& key,
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
