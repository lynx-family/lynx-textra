// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/tt_shaper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <textra/font_info.h>

#include <array>
#include <initializer_list>
#include <numeric>
#include <unordered_set>
#include <utility>

#if defined(ENABLE_CTSHAPER)
#include "src/ports/shaper/coretext/shaper_core_text_self_rendering.h"
#endif
#include "mocks.h"
#include "src/textlayout/shape_cache.h"
#include "test_utils.h"

using namespace ttoffice::tttext;
using namespace ::testing;

namespace {

class SelectiveTypefaceHelper final : public FixedMetricsTypefaceHelper {
 public:
  SelectiveTypefaceHelper(uint32_t unique_id, TypefaceRef backing_typeface,
                          std::initializer_list<Unichar> supported_characters)
      : FixedMetricsTypefaceHelper(unique_id),
        backing_typeface_(std::move(backing_typeface)),
        supported_characters_(supported_characters) {}

  uint16_t UnicharToGlyph(Unichar codepoint, uint32_t = 0) const override {
    return supported_characters_.count(codepoint) == 0 ? 0 : 1;
  }

  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override {
    for (uint32_t i = 0; i < count; ++i) {
      glyphs[i] = UnicharToGlyph(unichars[i]);
    }
  }

  const void* GetFontData() const override {
    return backing_typeface_->GetFontData();
  }

  size_t GetFontDataSize() const override {
    return backing_typeface_->GetFontDataSize();
  }

  int GetFontIndex() const override {
    return backing_typeface_->GetFontIndex();
  }

 private:
  TypefaceRef backing_typeface_;
  std::unordered_set<Unichar> supported_characters_;
};

class AdjacentFlagFontManager final : public IFontManager {
 public:
  AdjacentFlagFontManager()
      : primary_typeface_(std::make_shared<SelectiveTypefaceHelper>(
            1, TFH_DEFAULT, std::initializer_list<Unichar>{})),
        first_flag_typeface_(std::make_shared<SelectiveTypefaceHelper>(
            2, TFH_DEFAULT,
            std::initializer_list<Unichar>{0x1F3F3, 0xFE0F, 0x200D, 0x1F308})),
        second_flag_typeface_(std::make_shared<SelectiveTypefaceHelper>(
            3, TFH_DEFAULT, std::initializer_list<Unichar>{0x1F1E8, 0x1F1F3})) {
  }

  int countFamilies() const override { return 0; }

  TypefaceRef matchFamilyStyle(const char[], const FontStyle&) override {
    return primary_typeface_;
  }

  TypefaceRef matchFamilyStyleCharacter(const char[], const FontStyle&,
                                        const char*[], int,
                                        uint32_t character) override {
    if (first_flag_typeface_->UnicharToGlyph(character) != 0) {
      return first_flag_typeface_;
    }
    if (second_flag_typeface_->UnicharToGlyph(character) != 0) {
      return second_flag_typeface_;
    }
    return nullptr;
  }

  TypefaceRef makeFromFile(const char[], int) override { return nullptr; }

  TypefaceRef legacyMakeTypeface(const char[], FontStyle) const override {
    return primary_typeface_;
  }

  const TypefaceRef& first_flag_typeface() const {
    return first_flag_typeface_;
  }

  const TypefaceRef& second_flag_typeface() const {
    return second_flag_typeface_;
  }

 private:
  TypefaceRef primary_typeface_;
  TypefaceRef first_flag_typeface_;
  TypefaceRef second_flag_typeface_;
};

void AppendTestShapeResult(const ShapeKey& key, ShapeResult* result) {
  TestShapingResultReader reader(static_cast<uint32_t>(key.text_.size()));
  result->AppendPlatformShapingResult(reader);
}

}  // namespace

TEST(ShapeStyle, Constructor) {
  FontDescriptor font_descriptor;
  ShapeStyle style(font_descriptor, 10.f, true, true);
  EXPECT_EQ(style.GetFontDescriptor(), font_descriptor);
  EXPECT_FLOAT_EQ(style.GetFontSize(), 10.f);
  EXPECT_TRUE(style.FakeBold());
  EXPECT_TRUE(style.FakeItalic());
}

TEST(ShapeStyle, CopyAndAssignment) {
  ShapeStyle source(FontDescriptor(), 10.f, true, true);
  ShapeStyle copy_by_constructor(source);  // NOLINT(performance-*)
  EXPECT_EQ(copy_by_constructor, source);
  ShapeStyle copy_by_assignment;
  copy_by_assignment = source;
  EXPECT_EQ(copy_by_assignment, source);
}

TEST(ShapeStyle, SetFontDescriptor) {
  FontDescriptor old_font;
  ShapeStyle old_style(old_font, 10.f, false, false);
  EXPECT_EQ(old_style.GetFontDescriptor(), old_font);
  ShapeStyle new_style = old_style;

  FontDescriptor new_font;
  new_font.font_style_ = FontStyle::Bold();
  new_style.SetFontDescriptor(new_font);
  EXPECT_EQ(new_style.GetFontDescriptor(), new_font);

  EXPECT_FALSE(old_style == new_style);
}

TEST(ShapeStyle, FamilyOrderAndAllFieldsParticipateInIdentity) {
  FontDescriptor descriptor_ab{{"A", "B"}, FontStyle::Normal(), 0};
  FontDescriptor descriptor_ba{{"B", "A"}, FontStyle::Normal(), 0};

  ShapeStyle style_ab(descriptor_ab, 10.f, false, false);
  ShapeStyle style_ba(descriptor_ba, 10.f, false, false);
  ShapeStyle different_size(descriptor_ab, 11.f, false, false);
  ShapeStyle fake_bold(descriptor_ab, 10.f, true, false);
  ShapeStyle fake_italic(descriptor_ab, 10.f, false, true);

  EXPECT_FALSE(style_ab == style_ba);
  EXPECT_NE(std::hash<ShapeStyle>()(style_ab),
            std::hash<ShapeStyle>()(style_ba));
  EXPECT_FALSE(style_ab == different_size);
  EXPECT_FALSE(style_ab == fake_bold);
  EXPECT_FALSE(style_ab == fake_italic);
}

TEST(ShapeKey, Constructor) {
  const std::u32string text = U"Hello world";
  const float font_size = 10.f;
  {
    ShapeStyle style(FontDescriptor(), font_size, false, false);
    ShapeKey key(text.c_str(), text.size(), &style, true);
    EXPECT_EQ(key.style_, style);
    EXPECT_EQ(key.text_, text);
    EXPECT_TRUE(key.rtl_);
  }
  {
    ShapeKey key(text.c_str(), text.size(), FontDescriptor(), font_size, true,
                 true, true);
    EXPECT_EQ(key.text_, text);
    EXPECT_EQ(key.style_, ShapeStyle(FontDescriptor(), font_size, true, true));
    EXPECT_TRUE(key.rtl_);
  }
}

TEST(ShapeKey, UsedAsMapKey) {
  const std::u32string text1 = U"Hello world";
  const std::u32string text2 = U"Different text";
  const ShapeStyle style1(FontDescriptor(), 10.f, false, false);
  const ShapeStyle style2(FontDescriptor(), 10.f, true, true);

  ShapeKey key1(text1.c_str(), text1.size(), &style1, false);
  ShapeKey key2(text1.c_str(), text1.size(), &style1, false);  // Same as key1
  ShapeKey key3(text2.c_str(), text2.size(), &style1, false);  // Different text
  ShapeKey key4(text1.c_str(), text1.size(), &style2,
                false);                                       // Different style
  ShapeKey key5(text1.c_str(), text1.size(), &style1, true);  // Different RTL

  std::unordered_map<ShapeKey, int> shape_map;
  shape_map[key1] = 1;
  shape_map[key2] = 2;  // overrides shape_map[key1]
  shape_map[key3] = 3;
  shape_map[key4] = 4;
  shape_map[key5] = 5;

  EXPECT_EQ(shape_map.size(), 4u);
  EXPECT_EQ(shape_map[key1], 2);
  EXPECT_EQ(shape_map[key2], 2);  // key2 is identical to key1
  EXPECT_EQ(shape_map[key3], 3);
  EXPECT_EQ(shape_map[key4], 4);
  EXPECT_EQ(shape_map[key5], 5);
}

TEST(ShapeKey, FamilyOrderAlwaysParticipatesInIdentity) {
  const std::u32string text = U"same text";
  const ShapeStyle style_ab(FontDescriptor{{"A", "B"}, FontStyle::Normal(), 0},
                            10.f, false, false);
  const ShapeStyle style_ba(FontDescriptor{{"B", "A"}, FontStyle::Normal(), 0},
                            10.f, false, false);

  ShapeKey system_ab(text.c_str(), text.size(), &style_ab, false);
  ShapeKey system_ba(text.c_str(), text.size(), &style_ba, false);
  EXPECT_FALSE(system_ab == system_ba);
  EXPECT_NE(std::hash<ShapeKey>()(system_ab), std::hash<ShapeKey>()(system_ba));
}

#if defined(ENABLE_CTSHAPER)
namespace ttoffice {
namespace tttext {
class ShaperCoreTextSelfRenderingTestAccess {
 public:
  static CTFontDescriptorRef CopyFontDescriptorWithoutCascade(
      CTFontDescriptorRef descriptor) {
    return ShaperCoreTextSelfRendering::CopyFontDescriptorWithoutCascade(
        descriptor);
  }

  static CFArrayRef CreateOrderedCascadeDescriptors(
      const std::vector<CTFontDescriptorRef>& descriptors) {
    return ShaperCoreTextSelfRendering::CreateOrderedCascadeDescriptors(
        descriptors);
  }

  static bool FontCoversStringRangeExactly(CTFontRef font, CFStringRef string,
                                           CFRange range) {
    return ShaperCoreTextSelfRendering::FontCoversStringRangeExactly(
        font, string, range);
  }

  static size_t CacheKeyHash(const FontDescriptor& descriptor) {
    ShaperCoreTextSelfRendering::CachedSafeFontKey key{descriptor, 10.f, false,
                                                       false};
    return ShaperCoreTextSelfRendering::CachedSafeFontKey::Hasher()(key);
  }

  static bool CacheKeysEqual(const FontDescriptor& lhs_descriptor,
                             const FontDescriptor& rhs_descriptor) {
    ShaperCoreTextSelfRendering::CachedSafeFontKey lhs{lhs_descriptor, 10.f,
                                                       false, false};
    ShaperCoreTextSelfRendering::CachedSafeFontKey rhs{rhs_descriptor, 10.f,
                                                       false, false};
    return lhs == rhs;
  }
};
}  // namespace tttext
}  // namespace ttoffice

TEST(ShaperCoreTextSelfRendering, CacheKeyUsesFamilyOrder) {
  const FontDescriptor descriptor_ab{{"A", "B"}, FontStyle::Normal(), 0};
  const FontDescriptor descriptor_ba{{"B", "A"}, FontStyle::Normal(), 0};

  EXPECT_FALSE(ShaperCoreTextSelfRenderingTestAccess::CacheKeysEqual(
      descriptor_ab, descriptor_ba));
  EXPECT_NE(ShaperCoreTextSelfRenderingTestAccess::CacheKeyHash(descriptor_ab),
            ShaperCoreTextSelfRenderingTestAccess::CacheKeyHash(descriptor_ba));
}

TEST(ShaperCoreTextSelfRendering,
     OrderedCascadeStripsNestedCascadesAndPreservesOrder) {
  CTFontDescriptorRef descriptor_a =
      CTFontDescriptorCreateWithNameAndSize(CFSTR("Helvetica"), 0.0);
  CTFontDescriptorRef descriptor_b =
      CTFontDescriptorCreateWithNameAndSize(CFSTR("Courier"), 0.0);
  const void* nested_values[] = {descriptor_b};
  CFArrayRef nested_cascade = CFArrayCreate(kCFAllocatorDefault, nested_values,
                                            1, &kCFTypeArrayCallBacks);
  const void* cascade_keys[] = {kCTFontCascadeListAttribute};
  const void* cascade_values[] = {nested_cascade};
  CFDictionaryRef nested_attributes = CFDictionaryCreate(
      kCFAllocatorDefault, cascade_keys, cascade_values, 1,
      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CTFontDescriptorRef descriptor_a_with_nested_cascade =
      CTFontDescriptorCreateCopyWithAttributes(descriptor_a, nested_attributes);

  CTFontDescriptorRef descriptor_without_cascade =
      ShaperCoreTextSelfRenderingTestAccess::CopyFontDescriptorWithoutCascade(
          descriptor_a_with_nested_cascade);
  CFArrayRef stripped_cascade =
      static_cast<CFArrayRef>(CTFontDescriptorCopyAttribute(
          descriptor_without_cascade, kCTFontCascadeListAttribute));
  ASSERT_NE(stripped_cascade, nullptr);
  EXPECT_EQ(CFArrayGetCount(stripped_cascade), 0);

  CFArrayRef ordered_cascade =
      ShaperCoreTextSelfRenderingTestAccess::CreateOrderedCascadeDescriptors(
          {descriptor_a_with_nested_cascade, descriptor_b});
  ASSERT_EQ(CFArrayGetCount(ordered_cascade), 6);
  CTFontDescriptorRef ordered_a = static_cast<CTFontDescriptorRef>(
      const_cast<void*>(CFArrayGetValueAtIndex(ordered_cascade, 0)));
  CTFontDescriptorRef ordered_b = static_cast<CTFontDescriptorRef>(
      const_cast<void*>(CFArrayGetValueAtIndex(ordered_cascade, 1)));
  CFStringRef ordered_a_name = static_cast<CFStringRef>(
      CTFontDescriptorCopyAttribute(ordered_a, kCTFontNameAttribute));
  CFStringRef ordered_b_name = static_cast<CFStringRef>(
      CTFontDescriptorCopyAttribute(ordered_b, kCTFontNameAttribute));
  ASSERT_NE(ordered_a_name, nullptr);
  ASSERT_NE(ordered_b_name, nullptr);
  EXPECT_TRUE(CFEqual(ordered_a_name, CFSTR("Helvetica")));
  EXPECT_TRUE(CFEqual(ordered_b_name, CFSTR("Courier")));

  CFRelease(ordered_b_name);
  CFRelease(ordered_a_name);
  CFRelease(ordered_cascade);
  CFRelease(stripped_cascade);
  CFRelease(descriptor_without_cascade);
  CFRelease(descriptor_a_with_nested_cascade);
  CFRelease(nested_attributes);
  CFRelease(nested_cascade);
  CFRelease(descriptor_b);
  CFRelease(descriptor_a);
}

TEST(ShaperCoreTextSelfRendering, ExactCoverageRejectsCoreTextSystemFallback) {
  CTFontRef latin_font =
      CTFontCreateWithName(CFSTR("Helvetica"), 16.0, nullptr);
  CTFontRef cjk_font =
      CTFontCreateWithName(CFSTR("PingFang SC"), 16.0, nullptr);
  ASSERT_NE(latin_font, nullptr);
  ASSERT_NE(cjk_font, nullptr);

  CFStringRef text = CFSTR("\u4E2D");
  const CFRange range = CFRangeMake(0, CFStringGetLength(text));
  EXPECT_FALSE(
      ShaperCoreTextSelfRenderingTestAccess::FontCoversStringRangeExactly(
          latin_font, text, range));
  EXPECT_TRUE(
      ShaperCoreTextSelfRenderingTestAccess::FontCoversStringRangeExactly(
          cjk_font, text, range));

  CFRelease(cjk_font);
  CFRelease(latin_font);
}
#endif

TEST(ShapeResult, Constructor) {
  const uint32_t char_count = 10;
  ShapeResult result(char_count, false);
  EXPECT_EQ(result.CharCount(), 0u);
  EXPECT_EQ(result.GlyphCount(), 0u);
  EXPECT_FALSE(result.IsRTL());
}

TEST(ShapeResult, CopyConstructor) {
  const uint32_t char_count = 10;
  ShapeResult source(char_count, true);
  ShapeResult copy(source);
  EXPECT_EQ(copy.CharCount(), source.CharCount());
  EXPECT_EQ(copy.GlyphCount(), source.GlyphCount());
  EXPECT_EQ(copy.IsRTL(), source.IsRTL());
}

TEST(ShapeResult, AppendPlatformShapingResult) {
  TestShapingResultReader shaping_result(4);
  std::vector<std::array<float, 2>> advances = {
      {5.f, 0.f}, {10.f, 0.f}, {15.f, 0.f}, {20.f, 0.f}};
  shaping_result.glyphs_ = {1, 2, 3, 4};
  shaping_result.advances_ = advances;
  shaping_result.positions_ = {
      {0.f, 1.f}, {5.f, 2.f}, {15.f, 3.f}, {30.f, 4.f}};
  TypefaceRef typeface = nullptr;
  shaping_result.font_ = typeface;

  ShapeResult result(shaping_result.glyphs_.size(), false);
  result.AppendPlatformShapingResult(shaping_result);
  EXPECT_EQ(result.CharCount(), shaping_result.TextCount());
  for (uint32_t i = 0; i < result.CharCount(); i++) {
    EXPECT_EQ(result.Glyphs(i), shaping_result.glyphs_[i]);
    EXPECT_FLOAT_EQ(result.Advances(i)[0], shaping_result.advances_[i][0]);
    EXPECT_FLOAT_EQ(result.Advances(i)[1], shaping_result.advances_[i][1]);
    EXPECT_FLOAT_EQ(result.Positions(i)[0], shaping_result.positions_[i][0]);
    EXPECT_FLOAT_EQ(result.Positions(i)[1], shaping_result.positions_[i][1]);
    EXPECT_EQ(result.Font(i), shaping_result.font_);
    EXPECT_EQ(result.FontByCharId(i), typeface);
    EXPECT_EQ(result.CharToGlyph(i), i);
    EXPECT_EQ(result.GlyphToChar(i), i);
  }
}

TEST(ShapeResult, MeasureWidth) {
  ShapeResult result(0, false);
  EXPECT_FLOAT_EQ(result.MeasureWidth(0, 0, 0.0f), 0.0f);

  TestShapingResultReader shaping_result(4);
  shaping_result.glyphs_ = {1, 2, 3, 4};
  std::vector<std::array<float, 2>> advances = {
      {5.f, 0.f}, {10.f, 0.f}, {15.f, 0.f}, {20.f, 0.f}};
  shaping_result.advances_ = advances;
  TypefaceRef typeface = nullptr;
  shaping_result.font_ = typeface;
  result.AppendPlatformShapingResult(shaping_result);

  // Test MeasureWidth without letter spacing
  for (uint32_t idx = 1; idx <= advances.size(); idx++) {
    float expected_width = std::accumulate(
        advances.cbegin(), advances.cbegin() + idx, 0.f,
        [](float sum, auto coordinate) { return sum + coordinate[0]; });
    EXPECT_FLOAT_EQ(result.MeasureWidth(0, idx, 0.f), expected_width);
  }
  // Test MeasureWidth with letter spacing
  const float letter_spacing = 2.f;
  for (uint32_t idx = 1; idx <= advances.size(); idx++) {
    float expected_width =
        std::accumulate(
            advances.cbegin(), advances.cbegin() + idx, 0.f,
            [](float sum, auto coordinate) { return sum + coordinate[0]; }) +
        letter_spacing * idx;
    EXPECT_FLOAT_EQ(result.MeasureWidth(0, idx, letter_spacing),
                    expected_width);
  }
}

TEST(ShapeResult, MeasureWidthIncludesMultiGlyphClusterAdvances) {
  TestShapingResultReader shaping_result(4);
  shaping_result.glyphs_ = {1, 2, 3, 4};
  shaping_result.indices_ = {0, 0, 0, 1};
  shaping_result.text_count_ = 2;
  shaping_result.advances_ = {
      {5.f, 0.f}, {10.f, 0.f}, {15.f, 0.f}, {20.f, 0.f}};
  TypefaceRef typeface = nullptr;
  shaping_result.font_ = typeface;

  ShapeResult result(shaping_result.text_count_, false);
  result.AppendPlatformShapingResult(shaping_result);

  EXPECT_EQ(result.CharToGlyph(0), 0u);
  EXPECT_EQ(result.CharToGlyph(1), 3u);
  EXPECT_EQ(result.CharToGlyph(2), 4u);
  EXPECT_FLOAT_EQ(result.MeasureWidth(0, 1, 0.f), 30.f);
  EXPECT_FLOAT_EQ(result.MeasureWidth(0, 2, 0.f), 50.f);

  const float letter_spacing = 2.f;
  EXPECT_FLOAT_EQ(result.MeasureWidth(0, 1, letter_spacing), 32.f);
  EXPECT_FLOAT_EQ(result.MeasureWidth(0, 2, letter_spacing), 54.f);
}

TEST(TTShaper, Constructor) {
  FontmgrCollection expected = TestUtils::getFontmgrCollection();
  TestShaper shaper(expected);
  const FontmgrCollection& actual = shaper.GetFontCollection();
  EXPECT_EQ(actual.getFontManagersCount(), expected.getFontManagersCount());
  EXPECT_EQ(actual.GetDefaultFontManager(), expected.GetDefaultFontManager());
  EXPECT_EQ(actual.GetAssetFontManager(), expected.GetAssetFontManager());
  EXPECT_EQ(actual.GetDynamicFontManager(), expected.GetDynamicFontManager());
  EXPECT_EQ(actual.GetTestFontManager(), expected.GetTestFontManager());
}

TEST(TTShaper, ShapeTextCallsOnShapeText) {
  ShapeCache::GetInstance().Clear();
  FontmgrCollection font_collection = TestUtils::getFontmgrCollection();
  MockTTShaper mock_shaper(font_collection);
  const std::u32string text = U"test";
  ShapeStyle style(FontDescriptor(), 12.0f, false, false);
  const bool rtl = false;
  ShapeKey key(text.c_str(), text.length(), &style, rtl);

  // Test that ShapeText calls OnShapeText with the correct key
  EXPECT_CALL(mock_shaper, OnShapeText(key, _))
      .WillOnce(Invoke([&text](const ShapeKey& _, ShapeResult* result) {
        TestShapingResultReader shaping_result(text.length());
        result->AppendPlatformShapingResult(shaping_result);
      }));
  auto result = mock_shaper.ShapeText(text.c_str(), text.length(), &style, rtl);
  // Check the returned ShapeResult is set by OnShapeText
  EXPECT_EQ(result->CharCount(), text.length());
}

TEST(TTShaper, GlobalCacheIsSharedAcrossShapers) {
  ShapeCache::GetInstance().Clear();
  FontmgrCollection font_collection = TestUtils::getFontmgrCollection();
  MockTTShaper first_shaper(font_collection);
  MockTTShaper second_shaper(font_collection);
  TTTextContext context;
  context.SetShapeCacheMode(ShapeCacheMode::kGlobal);
  first_shaper.SetContext(context);
  second_shaper.SetContext(context);
  const std::u32string text = U"global cache";
  const ShapeStyle style(FontDescriptor(), 12.f, false, false);
  const ShapeKey key(text.c_str(), text.size(), &style, false);

  EXPECT_CALL(first_shaper, OnShapeText(key, _))
      .WillOnce(Invoke(AppendTestShapeResult));
  EXPECT_CALL(second_shaper, OnShapeText(_, _)).Times(0);

  auto first_result =
      first_shaper.ShapeText(text.c_str(), text.size(), &style, false);
  auto second_result =
      second_shaper.ShapeText(text.c_str(), text.size(), &style, false);

  EXPECT_EQ(first_result, second_result);
}

TEST(TTShaper, InstanceCacheIsIsolatedPerShaper) {
  ShapeCache::GetInstance().Clear();
  FontmgrCollection font_collection = TestUtils::getFontmgrCollection();
  MockTTShaper first_shaper(font_collection);
  MockTTShaper second_shaper(font_collection);
  TTTextContext context;
  context.SetShapeCacheMode(ShapeCacheMode::kInstance);
  first_shaper.SetContext(context);
  second_shaper.SetContext(context);
  const std::u32string text = U"instance cache";
  const ShapeStyle style(FontDescriptor(), 12.f, false, false);
  const ShapeKey key(text.c_str(), text.size(), &style, false);

  EXPECT_CALL(first_shaper, OnShapeText(key, _))
      .WillOnce(Invoke(AppendTestShapeResult));
  EXPECT_CALL(second_shaper, OnShapeText(key, _))
      .WillOnce(Invoke(AppendTestShapeResult));

  auto first_result =
      first_shaper.ShapeText(text.c_str(), text.size(), &style, false);
  auto first_cached_result =
      first_shaper.ShapeText(text.c_str(), text.size(), &style, false);
  auto second_result =
      second_shaper.ShapeText(text.c_str(), text.size(), &style, false);

  EXPECT_EQ(first_result, first_cached_result);
  EXPECT_NE(first_result, second_result);
}

TEST(TTShaper, DisabledCacheAlwaysMissesAndDoesNotPopulateGlobalCache) {
  ShapeCache::GetInstance().Clear();
  FontmgrCollection font_collection = TestUtils::getFontmgrCollection();
  MockTTShaper shaper(font_collection);
  TTTextContext context;
  context.SetShapeCacheMode(ShapeCacheMode::kDisabled);
  shaper.SetContext(context);
  const std::u32string text = U"disabled cache";
  const ShapeStyle style(FontDescriptor(), 12.f, false, false);
  const ShapeKey key(text.c_str(), text.size(), &style, false);

  EXPECT_CALL(shaper, OnShapeText(key, _))
      .Times(3)
      .WillRepeatedly(Invoke(AppendTestShapeResult));

  auto first_disabled_result =
      shaper.ShapeText(text.c_str(), text.size(), &style, false);
  auto second_disabled_result =
      shaper.ShapeText(text.c_str(), text.size(), &style, false);
  EXPECT_NE(first_disabled_result, second_disabled_result);

  context.SetShapeCacheMode(ShapeCacheMode::kGlobal);
  auto global_result =
      shaper.ShapeText(text.c_str(), text.size(), &style, false);
  auto global_cached_result =
      shaper.ShapeText(text.c_str(), text.size(), &style, false);

  EXPECT_NE(global_result, first_disabled_result);
  EXPECT_NE(global_result, second_disabled_result);
  EXPECT_EQ(global_result, global_cached_result);
}

TEST(TTShaper, SwitchingModesPreservesInstanceCache) {
  ShapeCache::GetInstance().Clear();
  FontmgrCollection font_collection = TestUtils::getFontmgrCollection();
  MockTTShaper shaper(font_collection);
  TTTextContext context;
  context.SetShapeCacheMode(ShapeCacheMode::kInstance);
  shaper.SetContext(context);
  const std::u32string text = U"preserved instance";
  const ShapeStyle style(FontDescriptor(), 12.f, false, false);
  const ShapeKey key(text.c_str(), text.size(), &style, false);

  EXPECT_CALL(shaper, OnShapeText(key, _))
      .Times(3)
      .WillRepeatedly(Invoke(AppendTestShapeResult));

  auto instance_result =
      shaper.ShapeText(text.c_str(), text.size(), &style, false);

  context.SetShapeCacheMode(ShapeCacheMode::kGlobal);
  auto global_result =
      shaper.ShapeText(text.c_str(), text.size(), &style, false);

  context.SetShapeCacheMode(ShapeCacheMode::kDisabled);
  auto disabled_result =
      shaper.ShapeText(text.c_str(), text.size(), &style, false);

  context.SetShapeCacheMode(ShapeCacheMode::kInstance);
  auto restored_instance_result =
      shaper.ShapeText(text.c_str(), text.size(), &style, false);

  EXPECT_NE(instance_result, global_result);
  EXPECT_NE(instance_result, disabled_result);
  EXPECT_EQ(instance_result, restored_instance_result);
}

TEST(TTShaper, ClearShapeCacheOnlyClearsThisShaperInstanceCache) {
  ShapeCache::GetInstance().Clear();
  FontmgrCollection font_collection = TestUtils::getFontmgrCollection();
  MockTTShaper first_shaper(font_collection);
  MockTTShaper second_shaper(font_collection);
  TTTextContext context;
  context.SetShapeCacheMode(ShapeCacheMode::kInstance);
  first_shaper.SetContext(context);
  second_shaper.SetContext(context);
  const std::u32string text = U"clear instance";
  const ShapeStyle style(FontDescriptor(), 12.f, false, false);
  const ShapeKey key(text.c_str(), text.size(), &style, false);

  EXPECT_CALL(first_shaper, OnShapeText(key, _))
      .Times(2)
      .WillRepeatedly(Invoke(AppendTestShapeResult));
  EXPECT_CALL(second_shaper, OnShapeText(key, _))
      .WillOnce(Invoke(AppendTestShapeResult));

  auto first_result =
      first_shaper.ShapeText(text.c_str(), text.size(), &style, false);
  auto second_result =
      second_shaper.ShapeText(text.c_str(), text.size(), &style, false);

  first_shaper.ClearInstanceShapeCache();

  auto first_result_after_clear =
      first_shaper.ShapeText(text.c_str(), text.size(), &style, false);
  auto second_result_after_clear =
      second_shaper.ShapeText(text.c_str(), text.size(), &style, false);

  EXPECT_NE(first_result, first_result_after_clear);
  EXPECT_EQ(second_result, second_result_after_clear);
}

TEST(TTShaper, UsesSeparateFallbackFontsForAdjacentFlagClusters) {
  auto font_manager = std::make_shared<AdjacentFlagFontManager>();
  FontmgrCollection font_collection(font_manager);
  ShaperSkShaper shaper(font_collection);
  const std::u32string text = U"🏳️‍🌈🇨🇳";
  const ShapeStyle style(FontDescriptor(), 12.f, false, false);
  const ShapeKey key(text.c_str(), text.size(), &style, false);
  ShapeResult result(text.size(), false);

  shaper.OnShapeText(key, &result);

  ASSERT_EQ(result.CharCount(), text.size());
  EXPECT_EQ(result.FontByCharId(0), font_manager->first_flag_typeface());
  EXPECT_EQ(result.FontByCharId(1), font_manager->first_flag_typeface());
  EXPECT_EQ(result.FontByCharId(2), font_manager->first_flag_typeface());
  EXPECT_EQ(result.FontByCharId(3), font_manager->first_flag_typeface());
  EXPECT_EQ(result.FontByCharId(4), font_manager->second_flag_typeface());
  EXPECT_EQ(result.FontByCharId(5), font_manager->second_flag_typeface());
}
