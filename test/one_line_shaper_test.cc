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

#include "src/ports/shaper/skshaper/script_run_iterator.h"
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
    if (purge_cache_on_advance) {
      purge_cache_on_advance = false;
      SkShaper::PurgeCaches();
    }
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
  const void* GetFontData() const override {
    ++font_data_queries;
    return kFontData;
  }
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
  mutable size_t font_data_queries = 0;
  mutable bool purge_cache_on_advance = false;

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

class CoverageFontManager : public IFontManager {
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

class TwoFontIterator final : public SkShaper::FontRunIterator {
 public:
  TwoFontIterator(Font first, Font second) : first_(first), second_(second) {}
  void consume() override { end_ = end_ == 0 ? 3 : 4; }
  size_t endOfCurrentRun() const override { return end_; }
  bool atEnd() const override { return end_ == 4; }
  const Font& currentFont() const override {
    return end_ <= 3 ? first_ : second_;
  }

 private:
  Font first_;
  Font second_;
  size_t end_ = 0;
};

class CollectRuns final : public SkShaper::RunHandler {
 public:
  void beginLine() override {}
  void runInfo(const RunInfo&) override {}
  void commitRunInfo() override {}
  Buffer runBuffer(const RunInfo& info) override {
    ranges.emplace_back(info.utf8Range.begin(), info.utf8Range.end());
    fonts.push_back(info.fFont.GetTypeface());
    glyphs.resize(info.glyphCount);
    advances.resize(info.glyphCount);
    clusters.resize(info.glyphCount);
    return {glyphs.data(), advances.data(), clusters.data()};
  }
  void commitRunBuffer(const RunInfo&) override {
    indices.insert(indices.end(), clusters.begin(), clusters.end());
    widths.insert(widths.end(), advances.begin(), advances.end());
  }
  void commitLine() override {}
  std::vector<std::pair<size_t, size_t>> ranges;
  std::vector<TypefaceRef> fonts;
  std::vector<uint32_t> indices;
  std::vector<float> widths;

 private:
  std::vector<GlyphID> glyphs;
  std::vector<float> advances;
  std::vector<uint32_t> clusters;
};

class ShaperRunIntersectionTest : public testing::TestWithParam<bool> {};
TEST_P(ShaperRunIntersectionTest, CombinesFontAndScriptBoundaries) {
  auto shaper = GetParam() ? SkShaper::MakeShapeDontWrapOrReorder()
                           : SkShaper::MakeShapeDontWrapOrReorderForPlatform();
  if (!shaper) GTEST_SKIP() << "Backend not built/available";
  auto first = std::make_shared<CoverageTypeface>(
      std::initializer_list<Unichar>{U'A', U'中', U'B', U'C'});
  auto second =
      std::make_shared<CoverageTypeface>(std::initializer_list<Unichar>{U'C'});
  for (bool ltr : {false, true}) {
    TwoFontIterator fonts(Font(first, 16), Font(second, 16));
    UnicodeScriptRunIterator scripts(U"A中BC", 4);
    CollectRuns collector;
    shaper->shape(U"A中BC", 4, &fonts, &scripts, ltr, 1000, &collector);
    EXPECT_EQ(collector.ranges, (std::vector<std::pair<size_t, size_t>>{
                                    {0, 1}, {1, 2}, {2, 3}, {3, 4}}));
    EXPECT_EQ(collector.indices, (std::vector<uint32_t>{0, 1, 2, 3}));
    ASSERT_EQ(collector.fonts.size(), 4u);
    EXPECT_EQ(collector.fonts[2], first);
    EXPECT_EQ(collector.fonts[3], second);
  }
}
INSTANTIATE_TEST_SUITE_P(Backends, ShaperRunIntersectionTest, testing::Bool());

std::vector<float> ShapeAtSize(
    const std::shared_ptr<CoverageTypeface>& typeface, float size) {
  auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
  SkShaper::TrivialFontRunIterator fonts(Font(typeface, size), 1);
  UnicodeScriptRunIterator scripts(U"A", 1);
  CollectRuns collector;
  shaper->shape(U"A", 1, &fonts, &scripts, true, 1000, &collector);
  return collector.widths;
}

class HarfBuzzFontCacheTest : public testing::Test {
 protected:
  void SetUp() override {
    if (!ShaperSkShaper::Preload()) GTEST_SKIP() << "HarfBuzz unavailable";
    SkShaper::PurgeCaches();
  }
  void TearDown() override { SkShaper::PurgeCaches(); }
};

TEST_F(HarfBuzzFontCacheTest, ReusesBaseFontWithIndependentSizes) {
  auto typeface =
      std::make_shared<CoverageTypeface>(std::initializer_list<Unichar>{U'A'});
  const auto first = ShapeAtSize(typeface, 12);
  ASSERT_EQ(first.size(), 1u);
  EXPECT_FLOAT_EQ(first[0], 12);
  const auto data_queries = typeface->font_data_queries;
  EXPECT_GT(data_queries, 0u);
  for (float size : {48.f, 12.f, 24.f}) {
    const auto widths = ShapeAtSize(typeface, size);
    ASSERT_EQ(widths.size(), 1u);
    EXPECT_FLOAT_EQ(widths[0], size);
    EXPECT_EQ(typeface->font_data_queries, data_queries);
  }
}

TEST_F(HarfBuzzFontCacheTest, RetainsFontDataUntilPurge) {
  std::weak_ptr<CoverageTypeface> weak;
  {
    auto typeface = std::make_shared<CoverageTypeface>(
        std::initializer_list<Unichar>{U'A'});
    weak = typeface;
    ASSERT_EQ(ShapeAtSize(typeface, 16).size(), 1u);
  }
  EXPECT_FALSE(weak.expired());
  SkShaper::PurgeCaches();
  EXPECT_TRUE(weak.expired());
}

TEST_F(HarfBuzzFontCacheTest, EvictsOldFontsAndReleasesOwners) {
  std::vector<std::weak_ptr<CoverageTypeface>> fonts;
  for (uint32_t i = 0; i < 101; ++i) {
    auto typeface = std::make_shared<CoverageTypeface>(
        std::initializer_list<Unichar>{U'A'});
    ASSERT_EQ(ShapeAtSize(typeface, 16).size(), 1u);
    fonts.push_back(typeface);
    if (i == 99) EXPECT_FALSE(fonts.front().expired());
  }
  EXPECT_TRUE(fonts.front().expired());
  EXPECT_FALSE(fonts.back().expired());
  SkShaper::PurgeCaches();
  for (const auto& font : fonts) EXPECT_TRUE(font.expired());
}

TEST_F(HarfBuzzFontCacheTest, ActiveSubfontSurvivesCachePurge) {
  auto typeface =
      std::make_shared<CoverageTypeface>(std::initializer_list<Unichar>{U'A'});
  typeface->purge_cache_on_advance = true;
  const auto widths = ShapeAtSize(typeface, 16);
  ASSERT_EQ(widths.size(), 1u);
  EXPECT_FLOAT_EQ(widths[0], 16);
  EXPECT_FALSE(typeface->purge_cache_on_advance);
  std::weak_ptr<CoverageTypeface> weak = typeface;
  typeface.reset();
  EXPECT_TRUE(weak.expired());
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

TEST_P(OneLineShaperFallbackTest, SingleFontMixedScriptsKeepGlobalIndices) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{
          {U'A', U'中', U'ا', U'ก', U'꧔'}});
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  const std::u32string text = U"A中اก꧔";
  auto result = shaper.ShapeText(text.data(), text.size(), &style, GetParam());
  ASSERT_TRUE(result);
  ASSERT_EQ(result->CharCount(), text.size());
  ASSERT_EQ(result->GlyphCount(), text.size());
  for (uint32_t i = 0; i < text.size(); ++i) {
    EXPECT_EQ(result->GlyphToChar(i), i);
    EXPECT_EQ(result->CharToGlyph(i), i);
    EXPECT_NE(result->Glyphs(i), 0);
  }
}

TEST_P(OneLineShaperFallbackTest, PreselectedFontsPreserveSpacesAndIndices) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{{U'A', U'B', U' '},
                                                            {U'中', U'文'}});
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  const std::u32string text = U"A 中文 B中文";
  auto result = shaper.ShapeText(text.data(), text.size(), &style, GetParam());
  ASSERT_TRUE(result);
  ASSERT_EQ(result->GlyphCount(), text.size());
  for (uint32_t i = 0; i < text.size(); ++i) {
    EXPECT_EQ(result->GlyphToChar(i), i);
    EXPECT_NE(result->Glyphs(i), 0);
    EXPECT_EQ(result->FontByCharId(i),
              manager->fonts[text[i] == U'中' || text[i] == U'文' ? 1 : 0]);
  }
}

TEST_P(OneLineShaperFallbackTest, SupportedPrimaryStaysOnNormalPath) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{
          {U'A', U'中', U'文', U' '}, {U'中', U'文'}});
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  const std::u32string text = U"A 中文";
  auto result = shaper.ShapeText(text.data(), text.size(), &style, GetParam());
  ASSERT_TRUE(result);
  ASSERT_EQ(result->CharCount(), text.size());
  for (uint32_t i = 0; i < text.size(); ++i) {
    EXPECT_EQ(result->FontByCharId(i), manager->fonts[0]);
  }
  EXPECT_EQ(manager->fonts[1]->glyph_queries, 0u);
}

TEST_P(OneLineShaperFallbackTest, FirstHanProbeDoesNotSuppressLaterFallback) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{{U'A', U'中', U' '},
                                                            {U'文'}});
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  auto result = shaper.ShapeText(U"A中 文", 4, &style, GetParam());
  ASSERT_TRUE(result);
  ASSERT_EQ(result->CharCount(), 4u);
  EXPECT_EQ(result->FontByCharId(1), manager->fonts[0]);
  EXPECT_EQ(result->FontByCharId(3), manager->fonts[1]);
}

TEST_P(OneLineShaperFallbackTest, PreselectionPreservesMissingGlyphs) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{{U'A'}});
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  auto result = shaper.ShapeText(U"A中", 2, &style, GetParam());
  ASSERT_TRUE(result);
  ASSERT_EQ(result->CharCount(), 2u);
  ASSERT_EQ(result->GlyphCount(), 2u);
  EXPECT_NE(result->Glyphs(result->CharToGlyph(0)), 0);
  EXPECT_EQ(result->Glyphs(result->CharToGlyph(1)), 0);
}

class OrderedFamilyManager final : public CoverageFontManager {
 public:
  OrderedFamilyManager() : CoverageFontManager({{U'A'}, {U'中'}, {U'中'}}) {}
  TypefaceRef matchFamilyStyle(const char family[], const FontStyle&) override {
    return fonts[std::string(family) == "secondary" ? 1 : 0];
  }
  TypefaceRef matchFamilyStyleCharacter(const char[], const FontStyle&,
                                        const char*[], int,
                                        uint32_t ch) override {
    return fonts[2]->UnicharToGlyph(ch) ? fonts[2] : nullptr;
  }
};

TEST_P(OneLineShaperFallbackTest, PreselectionRespectsFamilyPriority) {
  auto manager = std::make_shared<OrderedFamilyManager>();
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  FontDescriptor descriptor;
  descriptor.font_family_list_ = {"primary", "secondary"};
  const ShapeStyle style(descriptor, 16.f);
  auto result = shaper.ShapeText(U"A中", 2, &style, GetParam());
  ASSERT_TRUE(result);
  EXPECT_EQ(result->FontByCharId(0), manager->fonts[0]);
  EXPECT_EQ(result->FontByCharId(1), manager->fonts[1]);
}

TEST_P(OneLineShaperFallbackTest, PreselectionRespectsDisabledFallback) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{{U'A'}, {U'中'}});
  FontmgrCollection collection;
  collection.SetAssetFontManager(manager);
  collection.disableFontFallback();
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  auto result = shaper.ShapeText(U"A中", 2, &style, GetParam());
  ASSERT_TRUE(result);
  ASSERT_EQ(result->CharCount(), 2u);
  ASSERT_EQ(result->GlyphCount(), 2u);
  EXPECT_EQ(result->Glyphs(result->CharToGlyph(1)), 0);
  EXPECT_EQ(manager->fonts[1]->glyph_queries, 0u);
}

TEST_P(OneLineShaperFallbackTest, MixedScriptMarksFallbackWithinShapingRun) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{
          {U'এ'}, {U'ۣ', U'ۖ'}, {U'ิ'}, {U'꧔'}});
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  const std::u32string text = U"এۣۖิ꧔";
  auto result = shaper.ShapeText(text.data(), text.size(), &style, GetParam());
  ASSERT_TRUE(result);
  EXPECT_EQ(result->CharCount(), text.size());
  ASSERT_GT(result->GlyphCount(), 0u);
  for (uint32_t i = 0; i < text.size(); ++i) {
    EXPECT_LT(result->CharToGlyph(i), result->GlyphCount());
    EXPECT_NE(result->Glyphs(result->CharToGlyph(i)), 0);
  }
  for (uint32_t i = 0; i < result->GlyphCount(); ++i) {
    EXPECT_LT(result->GlyphToChar(i), text.size());
  }
  EXPECT_LT(manager->fonts.front()->glyph_queries, 1000u);
}

TEST_P(OneLineShaperFallbackTest,
       EmojiSequenceKeepsOneFallbackFontBetweenScripts) {
  auto manager = std::make_shared<CoverageFontManager>(
      std::initializer_list<std::initializer_list<Unichar>>{
          {U'A'}, {U' ', 0x1F3F3, 0xFE0F, 0x200D, 0x1F308}, {U'ا'}});
  FontmgrCollection collection(manager);
  ShaperSkShaper shaper(collection);
  const ShapeStyle style(FontDescriptor(), 16.f);
  const std::u32string text = U"A🏳️‍🌈ا";
  auto result = shaper.ShapeText(text.data(), text.size(), &style, GetParam());
  ASSERT_TRUE(result);
  ASSERT_EQ(result->CharCount(), text.size());
  EXPECT_EQ(result->FontByCharId(0), manager->fonts[0]);
  EXPECT_EQ(result->FontByCharId(1), manager->fonts[1]);
  EXPECT_EQ(result->FontByCharId(4), manager->fonts[1]);
  EXPECT_EQ(result->FontByCharId(5), manager->fonts[2]);
  for (uint32_t i = 0; i < result->GlyphCount(); ++i) {
    EXPECT_LT(result->GlyphToChar(i), text.size());
  }
}

INSTANTIATE_TEST_SUITE_P(Directions, OneLineShaperFallbackTest,
                         testing::Bool());

}  // namespace
}  // namespace tttext
}  // namespace ttoffice

#endif  // ENABLE_SKSHAPER
