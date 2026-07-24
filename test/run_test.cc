// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/i_typeface_helper.h>
#include <textra/layout_definition.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/run/base_run.h"
#include "src/textlayout/run/ghost_run.h"
#include "src/textlayout/tt_shaper.h"
#include "test/test_utils.h"

using namespace ttoffice::tttext;

namespace {
class TestTypeface final : public ITypefaceHelper {
 public:
  explicit TestTypeface(float ascent, ttoffice::tttext::FontStyle font_style =
                                          ttoffice::tttext::FontStyle())
      : ITypefaceHelper(1), ascent_(ascent) {
    font_style_ = font_style;
  }

  float GetHorizontalAdvance(GlyphID glyph_id, float font_size) const override {
    return 0;
  }
  void GetHorizontalAdvances(GlyphID glyph_ids[], uint32_t count,
                             float widths[], float font_size) const override {}
  void GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                     float font_size) const override {}
  void GetWidthBounds(float* rect_ltrb, GlyphID glyph_ids[],
                      uint32_t glyph_count, float font_size) override {}
  const void* GetFontData() const override { return nullptr; }
  size_t GetFontDataSize() const override { return 0; }
  int GetFontIndex() const override { return 0; }
  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variationSelector = 0) const override {
    return 0;
  }
  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override {}
  uint32_t GetUnitsPerEm() const override { return 0; }

 protected:
  void OnCreateFontInfo(FontInfo* info, float font_size) const override {
    info->SetAscent(ascent_);
    info->SetDescent(0);
    info->SetFontSize(font_size);
  }

 private:
  float ascent_;
};

class TestableBaseRun final : public BaseRun {
 public:
  using BaseRun::BaseRun;

  void SetShapeResult(const ShapeResultRef& result, uint32_t char_count) {
    shape_result_.InitWithShapeResult(result, 0, char_count);
  }
};

class TestableParagraphImpl final : public ParagraphImpl {
 public:
  void SetBidiLevelsForTest(std::vector<uint8_t> bidi_levels) {
    bidi_level_ = std::move(bidi_levels);
  }
};

class MultiFontShapingResultReader final
    : public tttext::PlatformShapingResultReader {
 public:
  MultiFontShapingResultReader(float advance, std::vector<TypefaceRef> fonts)
      : advance_(advance), fonts_(std::move(fonts)) {}

  uint32_t GlyphCount() const override { return fonts_.size(); }
  uint32_t TextCount() const override { return fonts_.size(); }
  GlyphID ReadGlyphID(uint32_t idx) const override { return idx + 1; }
  float ReadAdvanceX(uint32_t idx) const override { return advance_; }
  uint32_t ReadIndices(uint32_t idx) const override { return idx; }
  TypefaceRef ReadFontId(uint32_t idx) const override { return fonts_.at(idx); }

 private:
  float advance_;
  std::vector<TypefaceRef> fonts_;
};

ShapeResultRef MakeShapeResult(uint32_t char_count, float advance,
                               const TypefaceRef& typeface) {
  TestShapingResultReader reader(char_count);
  for (uint32_t i = 0; i < char_count; i++) {
    reader.glyphs_[i] = i + 1;
    reader.advances_[i] = {advance, 0};
  }
  reader.font_ = typeface;

  auto result = std::make_shared<ShapeResult>(char_count, false);
  result->AppendPlatformShapingResult(reader);
  return result;
}

ShapeResultRef MakeMultiFontShapeResult(float advance,
                                        std::vector<TypefaceRef> fonts) {
  MultiFontShapingResultReader reader(advance, std::move(fonts));
  auto result = std::make_shared<ShapeResult>(reader.TextCount(), false);
  result->AppendPlatformShapingResult(reader);
  return result;
}
}  // namespace

TEST(BaseRun, Constructor) {
  ParagraphImpl paragraph;
  const Style style;
  const uint32_t start_char_pos = 0;
  const uint32_t end_char_pos = 2;
  const RunType type = RunType::kTextRun;

  BaseRun run(&paragraph, style.GetImpl(), start_char_pos, end_char_pos, type);
  EXPECT_EQ(run.GetParagraph(), &paragraph);
  // EXPECT_EQ(run.GetLayoutStyle(), style); // no operator==
  EXPECT_FLOAT_EQ(run.GetStartCharPos(), start_char_pos);
  EXPECT_FLOAT_EQ(run.GetEndCharPos(), end_char_pos);
  EXPECT_EQ(run.GetType(), type);
}

TEST(BaseRun, FontFamilyOrderParticipatesInShapingMerge) {
  TestableParagraphImpl paragraph;
  paragraph.SetBidiLevelsForTest({0, 0});
  Style style_ab;
  style_ab.SetFontDescriptor(
      FontDescriptor{{"A", "B"}, FontStyle::Normal(), 0});
  Style style_ba;
  style_ba.SetFontDescriptor(
      FontDescriptor{{"B", "A"}, FontStyle::Normal(), 0});
  BaseRun run_ab(&paragraph, style_ab.GetImpl(), 0, 1, RunType::kTextRun);
  BaseRun run_ba(&paragraph, style_ba.GetImpl(), 1, 2, RunType::kTextRun);

  EXPECT_FALSE(run_ba.CanBeAppendToShaping(run_ab));
}

TEST(BaseRun, AssignmentOperator) {
  ParagraphImpl paragraph;
  const Style style;
  const uint32_t start_char_pos = 0;
  const uint32_t end_char_pos = 2;
  const RunType type = RunType::kTextRun;
  BaseRun expected_run(&paragraph, style.GetImpl(), start_char_pos,
                       end_char_pos, type);
  BaseRun actual_run;
  actual_run = expected_run;
  // EXPECT_EQ(actual_run.GetParagraph(), &paragraph); // Should it copy?
  // EXPECT_EQ(actual_run.GetLayoutStyle(), style); // no operator==
  EXPECT_FLOAT_EQ(actual_run.GetStartCharPos(), start_char_pos);
  EXPECT_FLOAT_EQ(actual_run.GetEndCharPos(), end_char_pos);
  EXPECT_EQ(actual_run.GetType(), type);
}

TEST(BaseRun, GetTypeMethod) {
  using CheckRunTypeMethod = bool (BaseRun::*)() const;
  const std::unordered_map<RunType, CheckRunTypeMethod> map = {
      {RunType::kTextRun, &BaseRun::IsTextRun},
      {RunType::kGhostRun, &BaseRun::IsGhostRun},
      {RunType::kSpaceRun, &BaseRun::IsTextRun},
      {RunType::kTabRun, &BaseRun::IsTextRun},
      {RunType::kInlineObject, &BaseRun::IsObjectRun},
      {RunType::kFloatObject, &BaseRun::IsObjectRun},
      {RunType::kBlockStart, &BaseRun::IsBlockRun},
      {RunType::kBlockEnd, &BaseRun::IsBlockRun},
      {RunType::kControlRun, &BaseRun::IsControlRun},
      {RunType::kCRRun, &BaseRun::IsControlRun},
      {RunType::kFFRun, &BaseRun::IsControlRun},
      {RunType::kLFRun, &BaseRun::IsControlRun},
      {RunType::kNLFRun, &BaseRun::IsControlRun},
      {RunType::kCRLFRun, &BaseRun::IsControlRun},
  };
  for (const auto& pair : map) {
    ParagraphImpl paragraph;
    BaseRun run(&paragraph, Style().GetImpl(), 0, 2, pair.first);
    EXPECT_TRUE((run.*pair.second)());
  }
}

TEST(BaseRun, BoundaryType) {
  ParagraphImpl paragraph;
  BaseRun run(&paragraph, Style().GetImpl(), 0, 2, RunType::kTextRun);
  run.SetBoundaryType(BoundaryType::kWord);
  EXPECT_EQ(run.GetBoundaryType(), BoundaryType::kWord);
}

TEST(TextRun, LastNoneSpaceCharPos) {
  ParagraphImpl paragraph;
  const char content[] = " 1 3 ";
  paragraph.AddTextRun(nullptr, content);
  BaseRun run(&paragraph, Style().GetImpl(), 0, strlen(content),
              RunType::kTextRun);
  EXPECT_EQ(run.LastNoneSpaceCharPos(), 4u);
}

TEST(GhostRun, GhostContent) {
  ParagraphImpl paragraph;
  std::unique_ptr<TTShaper> shaper = TestUtils::getTestShaper();
  paragraph.SetShaper(shaper.get());
  std::u32string ghost_content(U"ghost content");
  GhostRun run(&paragraph, Style().GetImpl(), 0, ghost_content.c_str(),
               ghost_content.size());
  EXPECT_EQ(run.GetGhostContent().ToString(), "ghost content");
}

TEST(ObjectRun, RunDelegate) {
  ParagraphImpl paragraph;
  BaseRun run(&paragraph, Style().GetImpl(), 0, 2, RunType::kInlineObject);
  auto delegate = std::make_shared<TestShape>();
  run.SetRunDelegate(delegate);
  EXPECT_EQ(run.GetRunDelegate(), delegate.get());
}

TEST(BaseRun, GetWidthAddsTextSkewExtraWidth) {
  ParagraphImpl paragraph;
  Style style;
  style.SetTextSkew(-0.25f);
  TestableBaseRun run(&paragraph, style.GetImpl(), 0, 2, RunType::kTextRun);
  auto typeface = std::make_shared<TestTypeface>(-20.f);
  run.SetShapeResult(MakeShapeResult(2, 10.f, typeface), 2);

  EXPECT_FLOAT_EQ(run.GetWidth(0, 2), 25.f);
}

TEST(BaseRun, GetWidthAddsItalicExtraWidth) {
  ParagraphImpl paragraph;
  Style style;
  style.SetItalic(true);
  TestableBaseRun run(&paragraph, style.GetImpl(), 0, 2, RunType::kTextRun);
  auto typeface = std::make_shared<TestTypeface>(-20.f);
  run.SetShapeResult(MakeShapeResult(2, 10.f, typeface), 2);

  EXPECT_FLOAT_EQ(run.GetWidth(0, 2), 25.f);
}

TEST(BaseRun, GetWidthUsesFirstRunFontForSkewExtraWidth) {
  ParagraphImpl paragraph;
  Style style;
  style.SetTextSkew(-0.25f);
  TestableBaseRun run(&paragraph, style.GetImpl(), 0, 2, RunType::kTextRun);
  auto first_typeface = std::make_shared<TestTypeface>(-20.f);
  auto second_typeface = std::make_shared<TestTypeface>(-100.f);
  run.SetShapeResult(
      MakeMultiFontShapeResult(10.f, {first_typeface, second_typeface}), 2);

  EXPECT_FLOAT_EQ(run.GetWidth(0, 2), 25.f);
  EXPECT_FLOAT_EQ(run.GetWidth(1, 1), 15.f);
}

TEST(BaseRun, MeasureRunByWidthIncludesSkewExtraWidth) {
  ParagraphImpl paragraph;
  Style style;
  style.SetTextSkew(-0.25f);
  TestableBaseRun run(&paragraph, style.GetImpl(), 0, 2, RunType::kTextRun);
  auto typeface = std::make_shared<TestTypeface>(-20.f);
  run.SetShapeResult(MakeShapeResult(2, 10.f, typeface), 2);

  uint32_t break_pos = 0;
  EXPECT_FLOAT_EQ(run.MeasureRunByWidth(break_pos, 14.f), 0.f);
  EXPECT_EQ(break_pos, 0u);

  break_pos = 0;
  EXPECT_FLOAT_EQ(run.MeasureRunByWidth(break_pos, 24.f), 15.f);
  EXPECT_EQ(break_pos, 1u);
}
