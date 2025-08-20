// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include "mocks.h"
#include "test_utils.h"

using namespace ttoffice::tttext;
using namespace ::testing;

TEST(LayoutDrawer, DrawTextLine) {
  // Arrange
  const char* text = "Hello World!";
  const uint32_t text_length = strlen(text);
  ParagraphImpl para;
  Style style;
  style.SetTextSize(1.f);
  para.AddTextRun(&style, text);
  TextLayout layout(TestUtils::getTestShaper());
  auto page = std::make_unique<LayoutRegion>(100.f, 100.f);
  TTTextContext context;
  layout.Layout(&para, page.get(), context);

  ASSERT_GT(page->GetLineCount(), 0u);
  TextLine* text_line = page->GetLine(0);
  ASSERT_NE(text_line, nullptr);
  NiceMock<MockCanvasHelper> canvas_helper;
  LayoutDrawer drawer(&canvas_helper);
  ON_CALL(canvas_helper, CreatePainter()).WillByDefault(Invoke([]() {
    return std::make_unique<Painter>();
  }));

  // Assert and Act
  EXPECT_CALL(canvas_helper,
              DrawGlyphs(_, text_length, _, nullptr, 0, _, _, _, _, _))
      .Times(1);
  drawer.DrawTextLine(text_line, 0, text_length);
  Mock::VerifyAndClearExpectations(&canvas_helper);

  // TODO: Fix DrawTextLine(_, char_start_in_line, char_end_in_line) not working
  // EXPECT_CALL(canvas_helper,
  //             DrawGlyphs(_, 1, _, StrEq("H"), 1, _, _, _, _, _))
  //     .Times(1);
  // drawer.DrawTextLine(text_line, 0, 1);
  // Mock::VerifyAndClearExpectations(&canvas_helper);
}

TEST(LayoutDrawer, DrawLayoutPage_Text) {
  // Arrange
  const float width = 100.f;
  const float height = 100.f;
  const char* text = "Hello World!";
  const uint32_t text_length = strlen(text);
  ParagraphImpl para;
  Style style;
  style.SetTextSize(1.f);
  para.AddTextRun(&style, "Hello World!");
  TTTextContext context;
  std::unique_ptr<LayoutRegion> page =
      std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getTestShaper());
  layout.Layout(&para, page.get(), context);
  // Assert
  NiceMock<MockCanvasHelper> canvas_helper;
  LayoutDrawer drawer(&canvas_helper);
  EXPECT_CALL(canvas_helper, CreatePainter()).WillOnce(Invoke([]() {
    return std::make_unique<Painter>();
  }));
  EXPECT_CALL(canvas_helper,
              DrawGlyphs(_, text_length, _, nullptr, 0, _, _, _, _, _))
      .Times(1);
  // Act
  drawer.DrawLayoutPage(page.get());
}

TEST(LayoutDrawer, DrawLayoutPage_TextWithUnderline) {
  // Arrange
  const float width = 100.f;
  const float height = 100.f;
  const char* text = "Hello World!";
  const uint32_t text_length = strlen(text);
  ParagraphImpl para;
  Style style;
  style.SetTextSize(1.f);
  style.SetDecorationType(DecorationType::kUnderLine);
  style.SetDecorationStyle(LineType::kSolid);
  style.SetDecorationColor(TTColor(TTColor::BLACK()));
  para.AddTextRun(&style, "Hello World!");
  TTTextContext context;
  std::unique_ptr<LayoutRegion> page =
      std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getTestShaper());
  layout.Layout(&para, page.get(), context);
  // Assert
  NiceMock<MockCanvasHelper> canvas_helper;
  LayoutDrawer drawer(&canvas_helper);
  EXPECT_CALL(canvas_helper, CreatePainter()).WillRepeatedly(Invoke([]() {
    return std::make_unique<Painter>();
  }));
  EXPECT_CALL(canvas_helper, DrawLine(0, _, text_length, _, _)).Times(1);
  // Act
  drawer.DrawLayoutPage(page.get());
}

TEST(LayoutDrawer, DrawLayoutPage_RunDelegate) {
  // Arrange
  const float width = 100.f;
  const float height = 100.f;

  class MockTestShape : public TestShape {
   public:
    MockTestShape(float width, float height) : TestShape(width, height) {}
    MOCK_METHOD(void, Draw, (ICanvasHelper*, float, float), (override));
  };
  auto run_delegate = std::make_shared<NiceMock<MockTestShape>>(30.f, 10.f);

  ParagraphImpl para;
  Style style;
  style.SetTextSize(1.f);
  para.AddGhostShapeRun(&style, run_delegate);
  // Note: ParagraphImpl assumes that it has at least one character, otherwise
  // it crashes, thus adding "text" run to make it non-empty.
  para.AddTextRun(&style, "text");

  TTTextContext context;
  std::unique_ptr<LayoutRegion> page =
      std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getTestShaper());
  layout.Layout(&para, page.get(), context);

  // Assert
  NiceMock<MockCanvasHelper> canvas_helper;
  LayoutDrawer drawer(&canvas_helper);
  auto painter = std::make_unique<Painter>();
  EXPECT_CALL(canvas_helper, CreatePainter())
      .WillOnce(Return(ByMove(std::move(painter))));
  EXPECT_CALL(*run_delegate, Draw(&canvas_helper, _, _)).Times(1);
  // Act
  drawer.DrawLayoutPage(page.get());
}
