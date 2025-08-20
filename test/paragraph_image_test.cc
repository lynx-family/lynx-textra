// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/platform/skity/skity_canvas_helper.h>
#include <textra/text_layout.h>

#include <memory>

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
    TestHelper(&ParagraphTest::TestLayoutMode, "layout_mode.png");
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
  void TestEmoji() { TestHelper(&ParagraphTest::TestEmoji, "emoji.png"); }
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
