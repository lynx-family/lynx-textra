// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/layout_drawer.h>
#include <textra/run_delegate.h>
#include <textra/text_layout.h>

#include "src/textlayout/paragraph_impl.h"
#include "test_src.h"
#include "test_utils.h"

TEST(ShapeTest, SmallShapeTest) {
  float width = 500;
  float height = 1000;
  ParagraphImpl paragraph;
  Style default_rpr;
  default_rpr.SetTextSize(24);
  paragraph.GetParagraphStyle().SetDefaultStyle(default_rpr);
  paragraph.AddTextRun(&default_rpr, "一二三");
  auto small_shape = std::make_unique<TestShape>(100, 50);
  paragraph.AddShapeRun({}, std::move(small_shape), true);
  paragraph.AddTextRun(&default_rpr, "四五六");
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);

  TestUtils::DrawLayoutPage(layout_page.get(), "small_shape_test_out.png");
}

TEST(ShapeTest, LargeShapeTest) {
  float width = 500;
  float height = 1000;
  ParagraphImpl paragraph;
  Style default_rpr;
  default_rpr.SetTextSize(24);
  paragraph.GetParagraphStyle().SetDefaultStyle(default_rpr);
  paragraph.AddTextRun(&default_rpr, "一二三");
  auto large_shape = std::make_unique<TestShape>(600, 300);
  paragraph.AddShapeRun({}, std::move(large_shape), true);
  paragraph.AddTextRun(&default_rpr, "四五六");
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  TestUtils::DrawLayoutPage(layout_page.get(), "large_shape_test_out.png");
}
