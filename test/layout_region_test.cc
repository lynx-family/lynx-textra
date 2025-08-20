// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <textra/layout_definition.h>
#include <textra/layout_region.h>
#include <textra/tttext_context.h>

#include <memory>

#include "text_line_impl.h"

using namespace ::testing;
using namespace ttoffice::tttext;

namespace {
class MockLayoutPageListener : public LayoutPageListener {
 public:
  MOCK_METHOD(void, OnLineLayouted,
              (LayoutRegion * page, uint32_t line_idx,
               bool last_line_in_paragraph, float available_height,
               LayoutResult* result));
};
}  // namespace

TEST(LayoutRegionTest, Constructor) {
  const float width = 100.f;
  const float height = 50.f;
  auto region = std::make_unique<LayoutRegion>(width, height);
  EXPECT_FLOAT_EQ(region->GetPageWidth(), width);
  EXPECT_FLOAT_EQ(region->GetPageHeight(), height);
}

TEST(LayoutRegionTest, GetLineCount) {
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  EXPECT_EQ(region->GetLineCount(), 0u);

  ParagraphImpl paragraph;
  paragraph.AddTextRun(nullptr, "text");
  auto line = std::make_unique<TextLineImpl>(&paragraph, region.get(),
                                             LayoutPosition{0, 0});
  line->SetLayouted();
  TTTextContext context;
  region->AddLine(std::move(line), context);
  EXPECT_EQ(region->GetLineCount(), 1u);
}

TEST(LayoutRegionTest, AddLineNotifiesListener) {
  MockLayoutPageListener listener;
  auto region = std::make_unique<LayoutRegion>(
      100.f, 50.f, LayoutMode::kDefinite, LayoutMode::kDefinite, &listener);
  ParagraphImpl paragraph;
  paragraph.AddTextRun(nullptr, "text");
  auto line = std::make_unique<TextLineImpl>(&paragraph, region.get(),
                                             LayoutPosition{0, 0});
  line->SetLayouted();

  EXPECT_CALL(listener, OnLineLayouted(region.get(), 0, _, _, _)).Times(1);
  TTTextContext context;
  region->AddLine(std::move(line), context);
}
