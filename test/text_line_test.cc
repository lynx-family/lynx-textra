// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/paragraph_style.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <cstdint>
#include <memory>
#include <utility>

#include "mocks.h"
#include "src/textlayout/style_attributes.h"
#include "test_utils.h"

using namespace ::testing;

namespace ttoffice {
namespace tttext {
class TextraTest : public ::testing::Test {
 protected:
  void SetUp() override {
    textlayout_ = std::make_unique<TextLayout>(TestUtils::getTestShaper());
    region_ = std::make_unique<LayoutRegion>(200, LAYOUT_MAX_UNITS);
    paragraph_ = Paragraph::Create();
  }

  void TearDown() override {}

  TTTextContext context_;
  FontmgrCollection font_collection_;
  std::unique_ptr<TextLayout> textlayout_;
  std::unique_ptr<LayoutRegion> region_;
  std::unique_ptr<Paragraph> paragraph_;
};

TEST_F(TextraTest, TestLineBoundRect) {
  Style style;
  style.SetTextSize(1);

  paragraph_->AddTextRun(
      &style,
      "Coding agents are transforming the software development lifecycle by "
      "taking on the mechanical, multi-step work that has traditionally slowed "
      "engineering teams down.");
  textlayout_->Layout(paragraph_.get(), region_.get(), context_);
  std::array<float, 4> rect;
  region_->GetLine(0)->GetBoundingRectForLine(rect.data());
  EXPECT_EQ(rect[0], rect[0]);
}
}  // namespace tttext
}  // namespace ttoffice