// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_OH_DRAWING_NEWAPI_TEST_H_
#define DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_OH_DRAWING_NEWAPI_TEST_H_

#include <native_drawing/drawing_bitmap.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_path.h>
#include <native_drawing/drawing_pen.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_text_line.h>
#include <native_drawing/drawing_text_lineTypography.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_types.h>

#include <cstddef>
#include <string>

class OHNewContext {
 public:
  OHNewContext() = default;
  ~OHNewContext() {
    if (typography_handler_)
      OH_Drawing_DestroyTypographyHandler(typography_handler_);
    if (line_typography_) OH_Drawing_DestroyLineTypography(line_typography_);
    if (line_) OH_Drawing_DestroyTextLine(line_);
  }
  OH_Drawing_TypographyCreate* typography_handler_ = nullptr;
  OH_Drawing_LineTypography* line_typography_ = nullptr;
  OH_Drawing_TextLine* line_ = nullptr;
  std::string content_ = "";
  uint32_t char_count_ = 0;
};

void OHNewBuildParagraph(void* context, const std::string& content);
void OHNewLayoutParagraph(void* context, double width);
void OHNewDrawParagraph(void* context, OH_Drawing_Canvas* canvas);

#endif  // DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_OH_DRAWING_NEWAPI_TEST_H_
