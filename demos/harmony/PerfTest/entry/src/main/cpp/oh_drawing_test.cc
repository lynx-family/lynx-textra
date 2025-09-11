// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "oh_drawing_test.h"

void OHBuildParagraph(void* ctx, const std::string& content) {
  auto* context = (OHContext*)ctx;
  OH_Drawing_TypographyStyle* typoStyle = OH_Drawing_CreateTypographyStyle();
  OH_Drawing_TextStyle* txtStyle = OH_Drawing_CreateTextStyle();
  OH_Drawing_SetTextStyleColor(txtStyle,
                               OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0xFF));
  OH_Drawing_SetTextStyleFontSize(txtStyle, 24);

  OH_Drawing_FontCollection* fc = OH_Drawing_CreateSharedFontCollection();
  OH_Drawing_TypographyCreate* handler =
      OH_Drawing_CreateTypographyHandler(typoStyle, fc);

  OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
  OH_Drawing_TypographyHandlerAddText(handler, content.c_str());
  OH_Drawing_TypographyHandlerPopTextStyle(handler);
  OH_Drawing_Typography* typography = OH_Drawing_CreateTypography(handler);

  context->typography_ = typography;

  // 释放内存
  OH_Drawing_DestroyTypographyStyle(typoStyle);
  OH_Drawing_DestroyTextStyle(txtStyle);
  OH_Drawing_DestroyFontCollection(fc);
  OH_Drawing_DestroyTypographyHandler(handler);
}
void OHLayoutParagraph(void* ctx, double width) {
  auto* context = (OHContext*)ctx;
  OH_Drawing_TypographyLayout(context->typography_, width);
}
void OHDrawParagraph(void* ctx, OH_Drawing_Canvas* canvas) {
  auto* context = (OHContext*)ctx;
  if (!context->typography_ || !canvas) return;
  OH_Drawing_TypographyPaint(context->typography_, canvas, 0, 0);
}
