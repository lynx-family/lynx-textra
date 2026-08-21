// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/paragraph_style.h>
#include <textra/platform/skity/skity_canvas_helper.h>
#include <textra/text_layout.h>

#include <cmath>
#include <memory>
#include <utility>

#include "demos/darwin/macos/ttreaderdemo/paragraph_test.h"
#include "gtest/gtest.h"
#include "test_utils.h"

// Image-based test that runs a subset of the tests in paragraph_test.h that are
// appropriate for LynxTextra
class ParagraphImageTest : public ::testing::Test {
 public:
  /**
   * @brief A test helper function to call a specific function in ParagraphTest
   * and draw the output to an PNG image. The image will later be compared
   * with the a baseline/golden image.
   *
   * @param func_ptr A pointer to the function in ParagraphTest to be tested.
   * @param out_file_name The name of the output image file.
   */
  template <typename ParagraphTestFuncPtr>
  void TestHelper(ParagraphTestFuncPtr func_ptr, const char* out_file_name) {
    ParagraphTest test(ShaperType::kSelfRendering,
                       &TestUtils::getFontmgrCollection());
    skity::Bitmap bitmap(test.test_width, test.test_height,
                         skity::AlphaType::kPremul_AlphaType,
                         skity::ColorType::kRGBA);
    auto canvas = skity::Canvas::MakeSoftwareCanvas(&bitmap);
    canvas->DrawColor(skity::Color_WHITE);
    SkityCanvasHelper canvas_helper(canvas.get());

    (&test->*func_ptr)(&canvas_helper, test.test_width);
    std::shared_ptr<skity::Codec> codec = skity::Codec::MakePngCodec();
    std::shared_ptr<skity::Data> data = codec->Encode(bitmap.GetPixmap().get());
    data->WriteToFile(out_file_name);
  }

  // TestXxx() methods below use friend declaration to access private member
  // functions in ParagraphTest
  void TestSupSub() { TestHelper(&ParagraphTest::TestSupSub, "sup_sub.png"); }
  void TestMultipleParagraph() {
    TestHelper(&ParagraphTest::TestMultiParagraph, "multi_paragraph.png");
  }
  void TestDecoration() {
    TestHelper(&ParagraphTest::TestDecoration, "decoration.png");
  }
  void TestOneString() {
    TestHelper(&ParagraphTest::TestOneString, "one_string.png");
  }
  void TestLineSpacing() {
    TestHelper(&ParagraphTest::TestLineSpacing, "line_spacing.png");
  }
  void TestMaxLine() {
    TestHelper(&ParagraphTest::TestMaxLine, "max_line.png");
  }
  void TestPageHeight() {
    TestHelper(&ParagraphTest::TestPageHeight, "page_height.png");
  }
  void TestEllipsis() {
    TestHelper(&ParagraphTest::TestEllipsis, "ellipsis.png");
  }
  void TestWordBreak() {
    TestHelper(&ParagraphTest::TestWordBreak, "word_break.png");
  }
  void TestLigature() {
    TestHelper(&ParagraphTest::TestLigature, "ligature.png");
  }
  void ShapeWithOffsetYTest() {
    TestHelper(&ParagraphTest::ShapeWithOffsetYTest, "shape_with_offset_y.png");
  }
  void TestFontStyle() {
    TestHelper(&ParagraphTest::TestFontStyle, "font_style.png");
  }
  void TestCRLF() { TestHelper(&ParagraphTest::TestCRLF, "crlf.png"); }
  void TestLineBreak() {
    TestHelper(&ParagraphTest::TestLineBreak, "line_break.png");
  }
  void TestLayoutMode() {
    // TestHelper(&ParagraphTest::TestLayoutMode, "layout_mode.png");
  }
  void TestLayoutMode2() {
    TestHelper(&ParagraphTest::TestLayoutMode2, "layout_mode2.png");
  }
  void TestRTLText() {
    TestHelper(&ParagraphTest::TestRTLText, "rtl_text.png");
  }
  void TestMultiStyleInWord() {
    TestHelper(&ParagraphTest::TestMultiStyleInWord, "multi_style_in_word.png");
  }
  void TestLetterSpacing() {
    TestHelper(&ParagraphTest::TestLetterSpacing, "letter_spacing.png");
  }
  void TestLineHeight() {
    TestHelper(&ParagraphTest::TestLineHeight, "line_height.png");
  }
  void TestFontCollection() {
    TestHelper(&ParagraphTest::TestFontCollection, "font_collection.png");
  }
  void TestLineHeightOverride() {
    TestHelper(&ParagraphTest::TestLineHeightOverride,
               "line_height_override.png");
  }
  void TestEmoji() {
    // TestHelper(&ParagraphTest::TestEmoji, "emoji.png");
  }
  void TestAlignment() {
    TestHelper(&ParagraphTest::TestAlignment, "alignment.png");
  }
  void TestBaselineOffset() {
    TestHelper(&ParagraphTest::TestBaselineOffset, "baseline_offset.png");
  }
  void TestPieceDraw() {
    TestHelper(&ParagraphTest::TestPieceDraw, "piece_draw.png");
  }
  void TestWordBoundary() {
    TestHelper(&ParagraphTest::TestWordBoundary, "word_boundary.png");
  }
  void TestRunDelegateEllipsis() {
    TestHelper(&ParagraphTest::TestRunDelegateEllipsis,
               "run_delegate_ellipsis.png");
  }
  void TestWhiteSpaceBreakLine() {
    TestHelper(&ParagraphTest::TestWhiteSpaceBreakLine,
               "white_space_break_line.png");
  }
  void TestLayoutedWidth() {
    TestHelper(&ParagraphTest::TestLayoutedWidth, "layouted_width.png");
  }

  void TestCharacterVerticalAlignment() {
    TestHelper(&ParagraphTest::TestCharacterVerticalAlignment,
               "character_vertical_alignment.png");
  }
  void TestParagraphVerticalAlignment() {
    TestHelper(&ParagraphTest::TestParagraphVerticalAlignment,
               "paragraph_vertical_alignment.png");
  }
  void TestAlignWithBBox() {
    TestHelper(&ParagraphTest::TestAlignWithBBox, "align_with_bbox.png");
  }
  void TestModifyHAlignAfterLayout() {
    TestHelper(&ParagraphTest::TestModifyHAlignAfterLayout,
               "modify_halign_after_layout.png");
  }
  void TestApplyStyleRange() {
    TestHelper(&ParagraphTest::TestApplyStyleInRange,
               "apply_style_in_range.png");
  }
  void TestTextShadow() {
    TestHelper(&ParagraphTest::TestTextShadow, "text_shadow.png");
  }
  void TestHalfLeading() {
    TestHelper(&ParagraphTest::TestHalfLeading, "half_leading.png");
  }

  void TestVerticalAlignMultipleLinesCase(float length_alignment,
                                          const char* out_file_name) {
    constexpr float kWidth = 220.f;
    constexpr float kFontSize = 14.f;
    constexpr float kLineHeight = 30.f;
    constexpr float kObjectSize = 20.f;

    auto layout_text = [&](uint32_t object_color) {
      auto paragraph = std::make_unique<ParagraphImpl>();
      ParagraphStyle paragraph_style;
      Style text_style;
      text_style.SetTextSize(kFontSize);
      paragraph_style.SetDefaultStyle(text_style);
      paragraph_style.SetLineHeightInPx(kLineHeight, RulerType::kExact);
      paragraph_style.SetInlineVerticalAlignmentMode(
          InlineVerticalAlignmentMode::kLineBox);
      paragraph->SetParagraphStyle(&paragraph_style);
      paragraph->AddTextRun(&text_style, "x", 1);

      class CaseShape final : public RunDelegate {
       public:
        explicit CaseShape(uint32_t color) : color_(color) {}
        float GetAscent() const override { return -kObjectSize; }
        float GetDescent() const override { return 0.f; }
        float GetAdvance() const override { return kObjectSize; }
        void Draw(ICanvasHelper* canvas, float x, float y) override {
          auto painter = canvas->CreatePainter();
          painter->SetFillColor(color_);
          canvas->DrawRect(x, y, x + kObjectSize, y + kObjectSize,
                           painter.get());
        }

       private:
        uint32_t color_;
      };

      const CharacterVerticalAlignment alignments[] = {
          CharacterVerticalAlignment::kMiddle,
          CharacterVerticalAlignment::kMiddle,
          CharacterVerticalAlignment::kTop,
          CharacterVerticalAlignment::kBottom,
          CharacterVerticalAlignment::kTextTop,
          CharacterVerticalAlignment::kTextBottom,
          CharacterVerticalAlignment::kSuperScript,
          CharacterVerticalAlignment::kBaseLine,
          CharacterVerticalAlignment::kSubScript,
      };
      for (const float baseline_offset :
           {-length_alignment, length_alignment}) {
        Style length_style = text_style;
        length_style.SetBaselineOffset(baseline_offset);
        paragraph->AddShapeRun(
            &length_style, std::make_shared<CaseShape>(object_color), false);
        for (const auto alignment : alignments) {
          Style object_style = text_style;
          object_style.SetVerticalAlignment(alignment);
          paragraph->AddShapeRun(
              &object_style, std::make_shared<CaseShape>(object_color), false);
        }
      }
      paragraph->AddTextRun(&text_style, "e", 1);

      TTTextContext context;
      TextLayout layout(TestUtils::getRealShaper());
      auto region = std::make_unique<LayoutRegion>(kWidth, 300.f);
      layout.Layout(paragraph.get(), region.get(), context);
      return std::make_pair(std::move(paragraph), std::move(region));
    };

    auto [image_paragraph, image_region] = layout_text(0xFF41A5F5);
    auto [view_paragraph, view_region] = layout_text(0xFFFF7043);
    const float image_height = image_region->GetLayoutedHeight();
    const float view_height = view_region->GetLayoutedHeight();
    skity::Bitmap bitmap(
        static_cast<uint32_t>(kWidth),
        static_cast<uint32_t>(std::ceil(image_height + view_height)),
        skity::AlphaType::kPremul_AlphaType, skity::ColorType::kRGBA);
    auto canvas = skity::Canvas::MakeSoftwareCanvas(&bitmap);
    canvas->DrawColor(skity::Color_WHITE);
    SkityCanvasHelper canvas_helper(canvas.get());
    TestUtils::DrawLayoutRegionOnCanvas(canvas_helper, *image_region);
    canvas_helper.Translate(0.f, image_height);
    TestUtils::DrawLayoutRegionOnCanvas(canvas_helper, *view_region);

    auto codec = skity::Codec::MakePngCodec();
    auto data = codec->Encode(bitmap.GetPixmap().get());
    data->WriteToFile(out_file_name);
  }
};

TEST_F(ParagraphImageTest, TestSupSub) { TestSupSub(); }
TEST_F(ParagraphImageTest, TestMultipleParagraph) { TestMultipleParagraph(); }
TEST_F(ParagraphImageTest, TestDecoration) { TestDecoration(); }
TEST_F(ParagraphImageTest, TestOneString) { TestOneString(); }
TEST_F(ParagraphImageTest, TestLineSpacing) { TestLineSpacing(); }
TEST_F(ParagraphImageTest, TestMaxLine) { TestMaxLine(); }
TEST_F(ParagraphImageTest, TestPageHeight) { TestPageHeight(); }
TEST_F(ParagraphImageTest, TestEllipsis) { TestEllipsis(); }
TEST_F(ParagraphImageTest, TestWordBreak) { TestWordBreak(); }
TEST_F(ParagraphImageTest, TestLigature) { TestLigature(); }
TEST_F(ParagraphImageTest, ShapeWithOffsetYTest) { ShapeWithOffsetYTest(); }
TEST_F(ParagraphImageTest, TestFontStyle) { TestFontStyle(); }
TEST_F(ParagraphImageTest, TestCRLF) { TestCRLF(); }
TEST_F(ParagraphImageTest, TestLineBreak) { TestLineBreak(); }
TEST_F(ParagraphImageTest, TestLayoutMode) { TestLayoutMode(); }
TEST_F(ParagraphImageTest, TestLayoutMode2) { TestLayoutMode2(); }
TEST_F(ParagraphImageTest, TestRTLText) { TestRTLText(); }
TEST_F(ParagraphImageTest, TestMultiStyleInWord) { TestMultiStyleInWord(); }
TEST_F(ParagraphImageTest, TestLetterSpacing) { TestLetterSpacing(); }
TEST_F(ParagraphImageTest, TestLineHeight) { TestLineHeight(); }
TEST_F(ParagraphImageTest, TestFontCollection) { TestFontCollection(); }
TEST_F(ParagraphImageTest, TestLineHeightOverride) { TestLineHeightOverride(); }
TEST_F(ParagraphImageTest, TestEmoji) { TestEmoji(); }
TEST_F(ParagraphImageTest, TestAlignment) { TestAlignment(); }
TEST_F(ParagraphImageTest, TestBaselineOffset) { TestBaselineOffset(); }
TEST_F(ParagraphImageTest, TestPieceDraw) { TestPieceDraw(); }
TEST_F(ParagraphImageTest, TestWordBoundary) { TestWordBoundary(); }
TEST_F(ParagraphImageTest, TestRunDelegateEllipsis) {
  TestRunDelegateEllipsis();
}
TEST_F(ParagraphImageTest, TestWhiteSpaceBreakLine) {
  TestWhiteSpaceBreakLine();
}
TEST_F(ParagraphImageTest, TestLayoutedWidth) { TestLayoutedWidth(); }
TEST_F(ParagraphImageTest, TestCharacterVerticalAlignment) {
  TestCharacterVerticalAlignment();
}
TEST_F(ParagraphImageTest, TestParagraphVerticalAlignment) {
  TestParagraphVerticalAlignment();
}
TEST_F(ParagraphImageTest, TestAlignWithBBox) { TestAlignWithBBox(); }
TEST_F(ParagraphImageTest, TestModifyHAlignAfterLayout) {
  TestModifyHAlignAfterLayout();
}
TEST_F(ParagraphImageTest, TestApplyStyleRange) { TestApplyStyleRange(); }
TEST_F(ParagraphImageTest, TestTextShadow) { TestTextShadow(); }
TEST_F(ParagraphImageTest, TestHalfLeading) { TestHalfLeading(); }
TEST_F(ParagraphImageTest, VerticalAlignMultipleLinesCaseBeforeClick) {
  TestVerticalAlignMultipleLinesCase(
      0.f, "vertical_align_multiple_lines_before_click.png");
}
TEST_F(ParagraphImageTest, VerticalAlignMultipleLinesCaseAfterClick) {
  TestVerticalAlignMultipleLinesCase(
      20.f, "vertical_align_multiple_lines_after_click.png");
}
