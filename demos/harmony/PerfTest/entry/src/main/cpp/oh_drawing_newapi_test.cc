// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "oh_drawing_newapi_test.h"

#include <native_drawing/drawing_point.h>
#include <native_drawing/drawing_text_run.h>
#include <native_drawing/drawing_text_blob.h>
#include <native_drawing/drawing_brush.h>
#include <hilog/log.h>
#include <sys/types.h>

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

void OHNewBuildParagraph(void* ctx) {
  auto* context = (OHNewContext*)ctx;
  auto shared_font_collection_ = OH_Drawing_CreateSharedFontCollection();
  auto typography_handler = OH_Drawing_CreateTypographyHandler(
      OH_Drawing_CreateTypographyStyle(), shared_font_collection_);

  context->typography_handler_ = typography_handler;
  context->content_ = "";
  context->char_count_ = 0;
}
void OHNewAppendContent(void* ctx, const std::string& text, uint32_t font_size,
                        uint32_t color) {
  auto* context = (OHNewContext*)ctx;
  auto typography_handler = context->typography_handler_;
  auto text_style = OH_Drawing_CreateTextStyle();
  OH_Drawing_SetTextStyleFontSize(text_style, font_size);
  auto alpha = (color & 0xFF000000) >> 24;
  auto red = (color & 0x00FF0000) >> 16;
  auto green = (color & 0x0000FF00) >> 8;
  auto blue = (color & 0x000000FF);
  OH_Drawing_SetTextStyleColor(
      text_style, OH_Drawing_ColorSetArgb(alpha, red, green, blue));
  OH_Drawing_TypographyHandlerPushTextStyle(typography_handler, text_style);
  OH_Drawing_TypographyHandlerAddEncodedText(typography_handler, text.c_str(),
                                             text.length(), TEXT_ENCODING_UTF8);
  OH_Drawing_TypographyHandlerPopTextStyle(typography_handler);
  OH_Drawing_DestroyTextStyle(text_style);
  context->content_ += text;
  context->char_count_ += CalcCharCount(text.c_str(), text.length());
}
void OHNewLayoutParagraph(void* ctx, double width) {
  auto* context = (OHNewContext*)ctx;
  auto line_typo =
      OH_Drawing_CreateLineTypography(context->typography_handler_);
  auto line =
      OH_Drawing_LineTypographyCreateLine(line_typo, 0, context->char_count_);
  auto glyph_count = OH_Drawing_TextLineGetGlyphCount(line);

  auto& result = context->glyph_struct_;
  result.clear();
  context->line_info_.clear();

  if (1) {
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
        OH_Drawing_PointGetY(adv, &aadv[1]);
        result.emplace_back(GlyphStruct{.glyph_id_ = glyph, .advance_ = aadv[0], .advance_y_ = aadv[1], .font_ = font});
      }
      glyph_idx += glyph_cnt;
    }
  }
  float w = 0;
  uint32_t start = 0;
  for (auto k = 0; k < result.size(); k++) {
    if (w + result[k].advance_ > width) {
      context->line_info_.emplace_back(std::make_pair(start, k));
      start = k;
      w = 0;
    }
    w += result[k].advance_;
  }
  if (start < result.size()) {
    context->line_info_.emplace_back(std::make_pair(start, result.size()));
  }
  context->line_ = line;
}
void OHNewCreateCanvas(void* context, OH_Drawing_Bitmap* bitmap){
  
}
void OHNewDrawParagraph(void* ctx, OH_Drawing_Canvas* canvas) {
  auto* context = (OHNewContext*)ctx;

  OH_Drawing_CanvasSave(canvas);
  auto y = 0.f;
  for (auto k = 0; k < context->line_info_.size(); k++) {
    auto glyph_start = context->line_info_[k].first;
    auto glyph_end = context->line_info_[k].second;
    auto glyph_count = glyph_end - glyph_start;
    auto& gi = context->glyph_struct_[glyph_start];
    auto font = gi.font_;
    y += OH_Drawing_FontGetTextSize(font);

    OH_Drawing_TextBlobBuilder* builder = OH_Drawing_TextBlobBuilderCreate();
    const OH_Drawing_RunBuffer* runBuffer =
        OH_Drawing_TextBlobBuilderAllocRunPos(builder, font, glyph_count,
                                              nullptr);
    auto x = 0.f;
    for (auto idx = 0; idx < glyph_count; idx++) {
      auto& glyph_struct = context->glyph_struct_[glyph_start + idx];
      runBuffer->glyphs[idx] = glyph_struct.glyph_id_;
      runBuffer->pos[idx * 2] = x;
      runBuffer->pos[idx * 2 + 1] = y;
      x += glyph_struct.advance_;
    }
    OH_Drawing_TextBlob* textBlob = OH_Drawing_TextBlobBuilderMake(builder);
    auto* brush = OH_Drawing_BrushCreate();
    OH_Drawing_BrushSetColor(brush, 0xFF0000FF);
    OH_Drawing_CanvasAttachBrush(canvas, brush);
    OH_Drawing_CanvasDrawTextBlob(canvas, textBlob, 0, 0);
    OH_Drawing_CanvasDetachBrush(canvas);
    OH_Drawing_BrushDestroy(brush);
    OH_Drawing_TextBlobDestroy(textBlob);
    OH_Drawing_TextBlobBuilderDestroy(builder);
  }
  OH_Drawing_CanvasRestore(canvas);
}
