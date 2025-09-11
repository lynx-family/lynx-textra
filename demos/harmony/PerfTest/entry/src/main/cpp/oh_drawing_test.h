// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_OH_DRAWING_TEST_H_
#define DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_OH_DRAWING_TEST_H_

#include <native_drawing/drawing_bitmap.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_path.h>
#include <native_drawing/drawing_pen.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_types.h>

#include <string>

class OHContext {
 public:
  OHContext() = default;
  ~OHContext() {
    if (handler_) OH_Drawing_DestroyTypographyHandler(handler_);
    if (fc_) OH_Drawing_DestroyFontCollection(fc_);
    if (typography_) OH_Drawing_DestroyTypography(typography_);
  }
  OH_Drawing_TypographyCreate* handler_ = nullptr;
  OH_Drawing_FontCollection* fc_ = nullptr;
  OH_Drawing_Typography* typography_ = nullptr;
};

void OHBuildParagraph(void* context, const std::string& content);
void OHLayoutParagraph(void* context, double width);
void OHDrawParagraph(void* context, OH_Drawing_Canvas* canvas);
#endif  // DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_OH_DRAWING_TEST_H_
