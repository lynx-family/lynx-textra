// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#if defined(ENABLE_SKSHAPER)

#include <gtest/gtest.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>

#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "src/ports/shaper/skshaper/shaper_skshaper.h"
#include "src/textlayout/shape_cache.h"
#include "test/mocks.h"

namespace ttoffice {
namespace tttext {
namespace {

// Use HarfBuzz's real Unicode clustering, with deterministic font coverage.
// The empty SFNT has no substitution tables; glyphs and metrics come from the
// ITypefaceHelper callbacks, so these tests do not depend on installed fonts.
class CoverageTypeface final : public ITypefaceHelper {
 public:
  explicit CoverageTypeface(std::initializer_list<Unichar> coverage)
      : ITypefaceHelper(NextId()), coverage_(coverage) {}

  float GetHorizontalAdvance(GlyphID, float size) const override {
    return size;
  }
  void GetHorizontalAdvances(GlyphID[], uint32_t count, float widths[],
                             float size) const override {
    for (uint32_t i = 0; i < count; ++i) widths[i] = size;
  }
  void GetWidthBound(float* rect, GlyphID, float size) const override {
    rect[0] = rect[1] = 0;
    rect[2] = rect[3] = size;
  }
  void GetWidthBounds(float* rects, GlyphID glyphs[], uint32_t count,
                      float size) override {
    for (uint32_t i = 0; i < count; ++i) {
      GetWidthBound(rects + 4 * i, glyphs[i], size);
    }
  }
  const void* GetFontData() const override { return kFontData; }
  size_t GetFontDataSize() const override { return sizeof(kFontData); }
  int GetFontIndex() const override { return 0; }
  uint32_t GetUnitsPerEm() const override { return 1000; }
  uint16_t UnicharToGlyph(Unichar codepoint, uint32_t = 0) const override {
    ++glyph_queries;
    return coverage_.count(codepoint) != 0 ? 1 : 0;
  }
  void UnicharsToGlyphs(const Unichar* text, uint32_t count,
                        GlyphID* glyphs) const override {
    for (uint32_t i = 0; i < count; ++i) {
      glyphs[i] = UnicharToGlyph(text[i]);
    }
  }

  mutable size_t glyph_queries = 0;

 protected:
  void OnCreateFontInfo(FontInfo* info, float size) const override {
    info->SetAscent(-size);
    info->SetDescent(0);
  }

 private:
  static uint32_t NextId() {
    static uint32_t next_id = 0x70000000;
    return next_id++;
  }
  inline static constexpr unsigned char kFontData[12] = {0, 1, 0, 0};
  std::unordered_set<Unichar> coverage_;
};

class CoverageFontManager final : public IFontManager {
 public:
  CoverageFontManager(
      std::initializer_list<std::initializer_list<Unichar>> coverages) {
    for (const auto& coverage : coverages) {
      fonts.emplace_back(std::make_shared<CoverageTypeface>(coverage));
    }
  }
  int countFamilies() const override { return 0; }
  TypefaceRef matchFamilyStyle(const char[], const FontStyle&) override {
    return fonts.empty() ? nullptr : fonts.front();
  }
  TypefaceRef matchFamilyStyleCharacter(const char[], const FontStyle&,
                                        const char*[], int,
                                        uint32_t character) override {
    for (const auto& font : fonts) {
      if (font->UnicharToGlyph(character) != 0) return font;
    }
    return nullptr;
  }
  TypefaceRef makeFromFile(const char[], int) override { return nullptr; }
  TypefaceRef legacyMakeTypeface(const char[], FontStyle) const override {
    return fonts.empty() ? nullptr : fonts.front();
  }

  std::vector<std::shared_ptr<CoverageTypeface>> fonts;
};

class OneLineShaperFallbackTest : public testing::TestWithParam<bool> {
 protected:
  std::shared_ptr<ShapeResult> Shape(
      const std::u32string& text,
      const std::shared_ptr<CoverageFontManager>& manager) {
    FontmgrCollection collection(manager);
    ShaperSkShaper shaper(collection);
    const ShapeStyle style(FontDescriptor(), 16.f);
    const ShapeKey key(text.data(), static_cast<uint32_t>(text.size()), &style,
                       GetParam());
    auto result = std::make_shared<ShapeResult>(
        static_cast<uint32_t>(text.size()), GetParam());
    shaper.OnShapeText(key, result.get());
    EXPECT_EQ(result->CharCount(), text.size());
    EXPECT_GT(result->GlyphCount(), 0u);
    EXPECT_LE(result->GlyphCount(), text.size() * 2);
    for (uint32_t i = 0; i < result->GlyphCount(); ++i) {
      EXPECT_LT(result->GlyphToChar(i), text.size());
    }
    size_t queries = 0;
    for (const auto& font : manager->fonts) queries += font->glyph_queries;
    EXPECT_LT(queries, 1000u);
    return result;
  }
};

TEST_P(OneLineShaperFallbackTest, MixedScriptMarksWithPartialCoverage) {
  // No font covers the entire protected mark cluster, and U+A9D4 is missing.
  // Expanding missing pieces creates [2,6) and [2,7); they must be merged
  // before reporting progress or trying another fallback font.
  const std::u32string text =
      U"\u2728\u5B89\u098F\u06E3\u06D6\u0E34\uA9D4\u7136\u2728";
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{
          {0x2728}, {0x5B89, 0x7136}, {0x098F}, {0x06E3, 0x06D6}, {0x0E34}});
  auto result = Shape(text, manager);
  // Preserve all characters that the available fonts can display, even when
  // no single font covers this mixed-script combining sequence.
  const size_t expected_fonts[] = {0, 1, 2, 3, 3, 4, 0, 1, 0};
  for (uint32_t i = 0; i < text.size(); ++i) {
    if (i == 6) continue;  // U+A9D4 has no font in this fixture.
    EXPECT_EQ(result->FontByCharId(i), manager->fonts[expected_fonts[i]]);
    EXPECT_NE(result->Glyphs(result->CharToGlyph(i)), 0);
  }
  EXPECT_EQ(result->Glyphs(result->CharToGlyph(6)), 0);
}

TEST_P(OneLineShaperFallbackTest, MissingGlyphsRemainFinite) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{{}});
  auto result = Shape(U"\u5B89\u098F\u06E3\u06D6\u0E34", manager);
  for (uint32_t i = 0; i < result->GlyphCount(); ++i) {
    EXPECT_EQ(result->Glyphs(i), 0);
  }
}

TEST_P(OneLineShaperFallbackTest, CompleteClusterUsesFallbackFont) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{
          {U'A', U'B'}, {0x098F, 0x06E3, 0x06D6, 0x0E34}});
  auto result = Shape(U"A\u098F\u06E3\u06D6\u0E34B", manager);
  for (uint32_t i = 1; i < 5; ++i) {
    EXPECT_EQ(result->FontByCharId(i), manager->fonts[1]);
  }
  EXPECT_EQ(result->FontByCharId(0), manager->fonts[0]);
  EXPECT_EQ(result->FontByCharId(5), manager->fonts[0]);
  for (uint32_t i = 0; i < result->GlyphCount(); ++i) {
    EXPECT_NE(result->Glyphs(i), 0);
  }
}

TEST_P(OneLineShaperFallbackTest, MissingMarkDoesNotHideSupportedBase) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{{U'A', U'B'}});
  auto result = Shape(U"A\u0301B", manager);
  EXPECT_NE(result->Glyphs(result->CharToGlyph(0)), 0);
  EXPECT_EQ(result->Glyphs(result->CharToGlyph(1)), 0);
  EXPECT_NE(result->Glyphs(result->CharToGlyph(2)), 0);
}

TEST_P(OneLineShaperFallbackTest, PartialEmojiRemainsProtectedDuringRetry) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{
          {U'A', U' '}, {0x0301}, {0x1F3F3, 0xFE0F, 0x200D}, {0x1F308}});
  auto result = Shape(U"A\u0301 \U0001F3F3\uFE0F\u200D\U0001F308", manager);
  EXPECT_EQ(result->FontByCharId(0), manager->fonts[0]);
  EXPECT_NE(result->Glyphs(result->CharToGlyph(0)), 0);
  EXPECT_EQ(result->FontByCharId(1), manager->fonts[1]);
  EXPECT_NE(result->Glyphs(result->CharToGlyph(1)), 0);
  for (uint32_t i = 4; i < 7; ++i) {
    EXPECT_EQ(result->FontByCharId(i), result->FontByCharId(3));
  }
}

TEST_P(OneLineShaperFallbackTest, AdjacentEmojiClustersUseSeparateFonts) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{
          {}, {0x1F3F3, 0xFE0F, 0x200D, 0x1F308}, {0x1F1E8, 0x1F1F3}});
  auto result =
      Shape(U"\U0001F3F3\uFE0F\u200D\U0001F308\U0001F1E8\U0001F1F3", manager);
  for (uint32_t i = 0; i < 4; ++i) {
    EXPECT_EQ(result->FontByCharId(i), manager->fonts[1]);
  }
  for (uint32_t i = 4; i < 6; ++i) {
    EXPECT_EQ(result->FontByCharId(i), manager->fonts[2]);
  }
}

TEST_P(OneLineShaperFallbackTest,
       MissingFontsPreserveTextAndRetryAfterLoading) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{});
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  const std::u32string text = U"A\tB";
  auto missing = shaper.ShapeText(text.data(), text.size(), &style, GetParam());
  ASSERT_TRUE(missing);
  EXPECT_EQ(missing->CharCount(), text.size());
  EXPECT_EQ(missing->GlyphCount(), 0u);
  EXPECT_EQ(missing->FontByCharId(0), nullptr);
  ShapeResultPiece piece;
  piece.InitWithShapeResult(missing, 0, text.size());
  EXPECT_EQ(piece.CharCount(), text.size());
  EXPECT_EQ(piece.GlyphCount(), 0u);

  // Neither the failed family lookup nor the empty shape may be cached.
  manager->fonts.emplace_back(std::make_shared<CoverageTypeface>(
      std::initializer_list<Unichar>{U'A', U'B'}));
  auto loaded = shaper.ShapeText(text.data(), text.size(), &style, GetParam());
  ASSERT_TRUE(loaded);
  EXPECT_NE(loaded, missing);
  ASSERT_GT(loaded->GlyphCount(), 0u);
  EXPECT_EQ(loaded->FontByCharId(0), manager->fonts.front());
  EXPECT_NE(loaded->Glyphs(loaded->CharToGlyph(0)), 0);
}

TEST(OneLineShaperMissingFonts, LayoutAndDrawingPreserveLogicalText) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{});
  FontmgrCollection collection(manager);
  TextLayout engine(&collection, RenderingBackend::kSkity);
  Style style;
  style.SetTextSize(16);
  auto paragraph = Paragraph::Create();
  paragraph->AddTextRun(&style, "ABC");
  LayoutRegion region(100, 100);
  TTTextContext context;
  EXPECT_EQ(engine.Layout(paragraph.get(), &region, context),
            LayoutResult::kNormal);
  EXPECT_EQ(paragraph->GetCharCount(), 3u);
  EXPECT_GT(region.GetLineCount(), 0u);
  testing::NiceMock<MockCanvasHelper> canvas;
  ON_CALL(canvas, CreatePainter()).WillByDefault([] {
    return std::make_unique<Painter>();
  });
  EXPECT_CALL(canvas, DrawGlyphs(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  LayoutDrawer drawer(&canvas);
  drawer.DrawLayoutPage(&region);
}

INSTANTIATE_TEST_SUITE_P(Directions, OneLineShaperFallbackTest,
                         testing::Bool());

}  // namespace
}  // namespace tttext
}  // namespace ttoffice

#endif  // ENABLE_SKSHAPER
