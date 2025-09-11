// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_TEXTRA_TEST_H_
#define DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_TEXTRA_TEST_H_

#include <native_drawing/drawing_canvas.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>

class TextraContext {
 public:
  std::unique_ptr<tttext::TTTextContext> context_;
  std::unique_ptr<tttext::Paragraph> paragraph_;
  std::unique_ptr<tttext::LayoutRegion> region_;
  std::unique_ptr<tttext::TextLayout> layout_;
};

void TextraBuildParagraph(void* context, const std::string& content);
void TextraLayoutParagraph(void* context, double width);
void TextraDrawParagraph(void* context, OH_Drawing_Canvas* canvas);
#endif  // DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_TEXTRA_TEST_H_
