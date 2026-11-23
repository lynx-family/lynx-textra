// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#if defined(ENABLE_SKSHAPER)

#include <gtest/gtest.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>

#include <array>
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
  void GetWidthBounds(float* rect, GlyphID[], uint32_t count,
                      float size) override {
    rect[0] = rect[1] = 0;
    rect[2] = size * static_cast<float>(count);
    rect[3] = count == 0 ? 0 : size;
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

TEST(CoverageTypefaceBounds, WritesOneAggregateRect) {
  CoverageTypeface typeface({U'A', U'B'});
  GlyphID glyphs[2]{1, 1};
  constexpr float kSize = 16;
  constexpr float kSentinel = 12345;
  for (uint32_t count : {0u, 1u, 2u}) {
    SCOPED_TRACE(count);
    std::array<float, 8> output;
    output.fill(kSentinel);
    typeface.GetWidthBounds(output.data(), glyphs, count, kSize);
    EXPECT_FLOAT_EQ(output[0], 0);
    EXPECT_FLOAT_EQ(output[1], 0);
    EXPECT_FLOAT_EQ(output[2], kSize * static_cast<float>(count));
    EXPECT_FLOAT_EQ(output[3], count == 0 ? 0 : kSize);
    for (size_t i = 4; i < output.size(); ++i) {
      EXPECT_FLOAT_EQ(output[i], kSentinel);
    }
  }
}

class OneLineShaperFallbackTest : public testing::TestWithParam<bool> {};

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
