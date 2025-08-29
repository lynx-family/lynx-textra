// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_DARWIN_MACOS_TTREADERDEMO_PARAGRAPH_TEST_H_
#define DEMOS_DARWIN_MACOS_TTREADERDEMO_PARAGRAPH_TEST_H_

#include <textra/i_canvas_helper.h>
#include <textra/i_font_manager.h>
#include <textra/paragraph.h>
#include <textra/run_delegate.h>
#include <textra/text_layout.h>

#include <cassert>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace tttext;
class TestShape : public RunDelegate {
 private:
  float width_ = 20;
  float height_ = 20;

 public:
  TestShape() = default;
  TestShape(float width, float height) : width_(width), height_(height) {}
  float GetAscent() const override { return -height_; }
  float GetDescent() const override { return 0; }
  float GetAdvance() const override { return width_; }
  void Draw(ICanvasHelper* canvas, float x, float y) override {
    auto paint = canvas->CreatePainter();
    paint->SetFillStyle(FillStyle::kFill);
    paint->SetColor(0xFF00FF00);
    float left = x;
    float top = y;
    canvas->DrawRect(left, top, left + width_, top + height_, paint.get());
  }
};

class TestFontMgr : public IFontManager {
 public:
  explicit TestFontMgr(std::vector<TypefaceRef> typeface_lst)
      : typeface_lst_(std::move(typeface_lst)) {}
  int countFamilies() const override { return typeface_lst_.size(); }
  TypefaceRef matchFamilyStyle(const char* familyName,
                               const FontStyle& style) override {
    assert(countFamilies() > 0);
    return typeface_lst_[0];
  }
  TypefaceRef matchFamilyStyleCharacter(const char* familyName,
                                        const FontStyle& style,
                                        const char** bcp47, int bcp47Count,
                                        uint32_t character) override {
    for (auto typeface : typeface_lst_) {
      if (typeface->UnicharToGlyph(character, 0) != 0) {
        return typeface;
      }
    }
    return nullptr;
  }
  TypefaceRef makeFromFile(const char* path, int ttcIndex) override {
    return nullptr;
  }
  TypefaceRef legacyMakeTypeface(const char* familyName,
                                 FontStyle style) const override {
    return typeface_lst_[0];
  }
  TypefaceRef createTypefaceFromPlatformFont(
      const void* platform_font) override {
    return nullptr;
  }
  void* getPlatformFontFromTypeface(TypefaceRef typeface) override {
    return typeface.get();
  }

 private:
  std::vector<TypefaceRef> typeface_lst_;
};

class ParagraphImageTest;
class ParagraphTest {
  friend ParagraphImageTest;

 public:
  explicit ParagraphTest(ShaperType type, FontmgrCollection* font_collection);

 private:
  void DrawParagraph(
      ICanvasHelper* canvas, Paragraph& paragraph, float width,
      LayoutMode width_mode = LayoutMode::kDefinite,
      LayoutMode height_mode = LayoutMode::kDefinite, uint32_t char_start = 0,
      uint32_t char_end = std::numeric_limits<uint32_t>::max()) const;

  // public:
  //  static std::unique_ptr<TTPainter> CreatePainter();

 public:
  static std::vector<std::pair<uint32_t, std::string>> GetTestCaseIdAndName();
  void TestWithId(ICanvasHelper* canvas, float width, uint32_t test_id);
  void DefaultTest(ICanvasHelper* canvas);
  void TestSkParagraph(ICanvasHelper* canvas, const std::string& text);
  void TestTTParagraph(ICanvasHelper* canvas, const std::string& text,
                       float width) const;

 private:
  using TestCaseFunction = void (ParagraphTest::*)(ICanvasHelper*, float) const;
  static std::vector<std::pair<std::string, TestCaseFunction>> GetTestCases();
  void DisplayInfo(ICanvasHelper* canvas, float width) const;
  void BlockTest(ICanvasHelper* canvas, float width) const;
  void ParagraphBlockTest(ICanvasHelper* canvas, float width) const;
  void TestSupSub(ICanvasHelper* canvas, float width) const;
  void TestMultiStyleInWord(ICanvasHelper* canvas, float width) const;
  void TestMultiParagraph(ICanvasHelper* canvas, float width) const;
  void TestLargeImg(ICanvasHelper* canvas, float width) const;
  void TestOneString(ICanvasHelper* canvas, float width) const;
  void TestLineSpacing(ICanvasHelper* canvas, float width) const;
  void TestDecoration(ICanvasHelper* canvas, float width) const;
  void TestMaxLine(ICanvasHelper* canvas, float width) const;
  void TestPageHeight(ICanvasHelper* canvas, float width) const;
  void TestGhostContent(ICanvasHelper* canvas, float width) const;
  void TestLetterSpacing(ICanvasHelper* canvas, float width) const;
  void TestEllipsis(ICanvasHelper* canvas, float width) const;
  void TestWordBreak(ICanvasHelper* canvas, float width) const;
  void TestLigature(ICanvasHelper* canvas, float width) const;
  void ShapeWithOffsetYTest(ICanvasHelper* canvas, float width) const;
  void TestFontStyle(ICanvasHelper* canvas, float width) const;
  void TestCRLF(ICanvasHelper* canvas, float width) const;
  void TestLineBreak(ICanvasHelper* canvas, float width) const;
  void TestLayoutMode(ICanvasHelper* canvas, float width) const;
  void TestLayoutMode2(ICanvasHelper* canvas, float width) const;
  void TestRTLText(ICanvasHelper* canvas, float width) const;
  void TestLineHeight(ICanvasHelper* canvas, float width) const;
  void TestFontCollection(ICanvasHelper* canvas, float width) const;
  void TestLineHeightOverride(ICanvasHelper* canvas, float width) const;
  void TestEmoji(ICanvasHelper* canvas, float width) const;
  void TestIntrinsicWidth(ICanvasHelper* canvas, float width) const;
  void TestLongestLine(ICanvasHelper* canvas, float width) const;
  void TestAlignment(ICanvasHelper* canvas, float width) const;
  void TestBaselineOffset(ICanvasHelper* canvas, float width) const;
  void TestPieceDraw(ICanvasHelper* canvas, float width) const;
  void TestWordBoundary(ICanvasHelper* canvas, float width) const;
  void TestRunDelegateEllipsis(ICanvasHelper* canvas, float width) const;
  void TestWhiteSpaceBreakLine(ICanvasHelper* canvas, float width) const;
  void TestLayoutedWidth(ICanvasHelper* canvas, float width) const;
  void TestCharacterVerticalAlignment(ICanvasHelper* canvas, float width) const;
  void TestParagraphVerticalAlignment(ICanvasHelper* canvas, float width) const;
  void TestItalicFont(ICanvasHelper* canvas, float width) const;
  void TestCJKBreak(ICanvasHelper* canvas, float width) const;
  void TestAlignWithBBox(ICanvasHelper* canvas, float width) const;
  void TestModifyHAlignAfterLayout(ICanvasHelper* canvas, float width) const;

 private:
  uint32_t test_width = 490;
  uint32_t test_height = 1000;
  mutable bool draw_page_bound_ = true;
  mutable bool draw_layouted_bound_ = true;

 private:
  ShaperType shaper_type_;
  FontmgrCollection* font_collection_;
  std::unique_ptr<Painter> painter_;
};

#endif  // DEMOS_DARWIN_MACOS_TTREADERDEMO_PARAGRAPH_TEST_H_
