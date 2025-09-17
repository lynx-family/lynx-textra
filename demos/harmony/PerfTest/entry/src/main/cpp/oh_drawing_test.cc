// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "oh_drawing_test.h"

void OHBuildParagraph(void* ctx) {
  auto* context = (OHContext*)ctx;
  OH_Drawing_TypographyStyle* typoStyle = OH_Drawing_CreateTypographyStyle();
  OH_Drawing_FontCollection* fc = OH_Drawing_CreateSharedFontCollection();
  OH_Drawing_TypographyCreate* handler =
      OH_Drawing_CreateTypographyHandler(typoStyle, fc);
  context->handler_ = handler;

  // 释放内存
  OH_Drawing_DestroyTypographyStyle(typoStyle);
  OH_Drawing_DestroyFontCollection(fc);
}
void OHAppendContent(void* ctx, const std::string& text, uint32_t font_size,
                     uint32_t color) {
  auto* context = (OHContext*)ctx;
  auto handler = context->handler_;
  OH_Drawing_TextStyle* txtStyle = OH_Drawing_CreateTextStyle();
  auto alpha = (color & 0xFF000000) >> 24;
  auto red = (color & 0x00FF0000) >> 16;
  auto green = (color & 0x0000FF00) >> 8;
  auto blue = (color & 0x000000FF);
  OH_Drawing_SetTextStyleColor(txtStyle,
                               OH_Drawing_ColorSetArgb(0xFF, red, green, blue));
  OH_Drawing_SetTextStyleFontSize(txtStyle, font_size);

  OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
  OH_Drawing_TypographyHandlerAddEncodedText(handler, text.c_str(),
                                             text.length(), TEXT_ENCODING_UTF8);
  OH_Drawing_TypographyHandlerPopTextStyle(handler);

  OH_Drawing_DestroyTextStyle(txtStyle);
}
void OHLayoutParagraph(void* ctx, double width) {
  auto* context = (OHContext*)ctx;
  OH_Drawing_Typography* typography =
      OH_Drawing_CreateTypography(context->handler_);
  OH_Drawing_TypographyLayout(typography, width);
  context->typography_ = typography;
}
void OHDrawParagraph(void* ctx, OH_Drawing_Canvas* canvas) {
  auto* context = (OHContext*)ctx;
  if (!context->typography_ || !canvas) return;
  OH_Drawing_TypographyPaint(context->typography_, canvas, 0, 0);
}