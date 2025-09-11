// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "oh_drawing_newapi_test.h"

#include <native_drawing/drawing_point.h>
#include <native_drawing/drawing_text_run.h>

#include <cstdint>

inline bool IsUtf8CharStart(const char* s) {
  return ((uint8_t)*s & 0xC0u) != 0x80u;
}
inline int CalcCharCount(const char* s, int len) {
  int count = 0;
  int idx = 0;
  while (idx < len) {
    if (IsUtf8CharStart(s + idx++)) count++;
  }
  return count;
}

void OHNewBuildParagraph(void* ctx, const std::string& content) {
  auto* context = (OHNewContext*)ctx;
  auto shared_font_collection_ = OH_Drawing_CreateSharedFontCollection();
  auto typography_handler = OH_Drawing_CreateTypographyHandler(
      OH_Drawing_CreateTypographyStyle(), shared_font_collection_);
  auto text_style = OH_Drawing_CreateTextStyle();
  OH_Drawing_SetTextStyleFontSize(text_style, 24);
  //  auto &fd = key.style_.GetFontDescriptor();
  const char** families = new const char*[1];
  const char* family = "";
  //  for (int k = 0; k < fd.font_family_list_.size(); k++) {
  //    families[k] = fd.font_family_list_[k].c_str();
  //  }
  OH_Drawing_SetTextStyleFontFamilies(text_style, 1, &family);
  //  if (fd.font_style_.GetSlant() != FontStyle::kUpright_Slant)
  //    OH_Drawing_SetTextStyleFontStyle(text_style, FONT_STYLE_ITALIC);
  OH_Drawing_SetTextStyleFontWeight(text_style, FONT_WEIGHT_400);
  OH_Drawing_TypographyHandlerPushTextStyle(typography_handler, text_style);
  OH_Drawing_TypographyHandlerAddEncodedText(typography_handler,
                                             content.c_str(), content.length(),
                                             TEXT_ENCODING_UTF8);

  context->typography_handler_ = typography_handler;
  context->content_ = content;
  context->char_count_ = CalcCharCount(content.c_str(), content.length());
}
void OHNewLayoutParagraph(void* ctx, double width) {
  auto* context = (OHNewContext*)ctx;
  auto line_typo =
      OH_Drawing_CreateLineTypography(context->typography_handler_);
  auto line =
      OH_Drawing_LineTypographyCreateLine(line_typo, 0, context->char_count_);
  auto glyph_count = OH_Drawing_TextLineGetGlyphCount(line);

  if (0) {
    auto glyph_runs = OH_Drawing_TextLineGetGlyphRuns(line);
    auto glyph_runs_count = OH_Drawing_GetDrawingArraySize(glyph_runs);
    auto glyph_idx = 0;
    for (auto k = 0; k < glyph_runs_count; k++) {
      auto* run = OH_Drawing_GetRunByIndex(glyph_runs, k);
      auto glyph_cnt = OH_Drawing_GetRunGlyphCount(run);
      auto glyph_array = OH_Drawing_GetRunGlyphs(run, 0, glyph_cnt);
      auto advance_array = OH_Drawing_GetRunGlyphAdvances(run, 0, glyph_cnt);
      auto position_array = OH_Drawing_GetRunPositions(run, 0, glyph_cnt);
      auto indices_array = OH_Drawing_GetRunStringIndices(run, 0, glyph_cnt);
      auto font = OH_Drawing_GetRunFont(run);

      for (auto j = 0; j < glyph_cnt; j++) {
        auto glyph = OH_Drawing_GetRunGlyphsByIndex(glyph_array, j);
        auto pos = OH_Drawing_GetRunPositionsByIndex(position_array, j);
        float ppos[2];
        OH_Drawing_PointGetX(pos, &ppos[0]);
        OH_Drawing_PointGetY(pos, &ppos[1]);
        auto indice = static_cast<uint32_t>(
            OH_Drawing_GetRunStringIndicesByIndex(indices_array, j));
        auto adv = OH_Drawing_GetRunGlyphAdvanceByIndex(advance_array, j);
        float aadv[2];
        OH_Drawing_PointGetX(adv, &aadv[0]);
        //      shaping_result.typeface_[glyph_idx + j] = tf_helper;
      }
      glyph_idx += glyph_cnt;
      //    ohos_shaping_funcs_->DestroyRunStringIndices_(indices_array);
      //    ohos_shaping_funcs_->DestroyRunPositions_(position_array);
      //    ohos_shaping_funcs_->DestroyRunGlyphs_(glyph_array);
    }
    //  delete[] families;
    //
    //  ohos_shaping_funcs_->DestroyRuns_(glyph_runs);
    //  ohos_shaping_funcs_->DestroyTextLine_(line);
    //  ohos_shaping_funcs_->DestroyLineTypography_(line_typo);
  }
  context->line_ = line;
}
void OHNewDrawParagraph(void* ctx, OH_Drawing_Canvas* canvas) {
  auto* context = (OHNewContext*)ctx;
  OH_Drawing_TextLinePaint(context->line_, canvas, 0, 0);
}
