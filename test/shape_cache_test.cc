// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/font_info.h>

#include <thread>

#include "src/textlayout/shaper_cache.h"
#include "src/textlayout/tt_shaper.h"
#include "test_utils.h"

using namespace ttoffice::tttext;

namespace {

class LigatureShapeResultReader final : public PlatformShapingResultReader {
 public:
  uint32_t GlyphCount() const override { return 2; }
  uint32_t TextCount() const override { return 3; }
  GlyphID ReadGlyphID(uint32_t idx) const override { return glyphs_.at(idx); }
  float ReadAdvanceX(uint32_t idx) const override {
    return advances_.at(idx)[0];
  }
  float ReadAdvanceY(uint32_t idx) const override {
    return advances_.at(idx)[1];
  }
  float ReadPositionX(uint32_t idx) const override { return 0.f; }
  float ReadPositionY(uint32_t idx) const override { return 0.f; }
  uint32_t ReadIndices(uint32_t idx) const override {
    return char_indices_.at(idx);
  }
  TypefaceRef ReadFontId(uint32_t idx) const override { return nullptr; }

 private:
  std::vector<GlyphID> glyphs_{10, 20};
  std::vector<std::array<float, 2>> advances_{{5.f, 0.f}, {6.f, 0.f}};
  std::vector<uint32_t> char_indices_{0, 2};
};

class CacheTestShaper final : public TTShaper {
 public:
  explicit CacheTestShaper(const FontmgrCollection& fonts = FontmgrCollection())
      : TTShaper(fonts) {}

  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override {
    ++shape_count;
    TestShapingResultReader reader(key.text_.size());
    result->AppendPlatformShapingResult(reader);
  }

  mutable uint32_t shape_count = 0;
};

class ShapeCacheTest : public ::testing::TestWithParam<bool> {};

}  // namespace

TEST_P(ShapeCacheTest, AddToCacheAndFind) {
  FontDescriptor font1;
  FontDescriptor font2;
  font2.font_style_ = FontStyle::Bold();
  ShapeKey key1(U"1", 2, FontDescriptor(font1), 10.f, false);
  ShapeKey key2(U"A", 2, FontDescriptor(font1), 10.f, false);
  ShapeKey key3(U"1", 2, FontDescriptor(font2), 10.f, false);
  ShapeKey key4(U"1", 2, FontDescriptor(font1), 20.f, false);
  ShapeKey key5(U"1", 2, FontDescriptor(font1), 10.f, true);

  const auto create_fake_shape_result = [](std::vector<GlyphID> glyphs) {
    auto result = std::make_shared<ShapeResult>(glyphs.size(), false);
    TestShapingResultReader reader(glyphs.size());
    reader.glyphs_ = glyphs;
    result->AppendPlatformShapingResult(reader);
    return result;
  };
  const ShapeResultRef result1 = create_fake_shape_result({1});
  const ShapeResultRef result2 = create_fake_shape_result({2});
  const ShapeResultRef result3 = create_fake_shape_result({3});
  const ShapeResultRef result4 = create_fake_shape_result({4});
  const ShapeResultRef result5 = create_fake_shape_result({5});

  auto cache_handle = TTTextContext::CreateShaperCache(GetParam());
  ShapeCache& cache = cache_handle->shape_cache_;
  cache.AddToCache(key1, result1);
  cache.AddToCache(key2, result2);
  cache.AddToCache(key3, result3);
  cache.AddToCache(key4, result4);
  cache.AddToCache(key5, result5);

  EXPECT_EQ(cache.Find(key1), result1);
  EXPECT_EQ(cache.Find(key2), result2);
  EXPECT_EQ(cache.Find(key3), result3);
  EXPECT_EQ(cache.Find(key4), result4);
  EXPECT_EQ(cache.Find(key5), result5);
}

TEST_P(ShapeCacheTest, CreatesIndependentCaches) {
  auto cache1 = TTTextContext::CreateShaperCache(GetParam());
  auto cache2 = TTTextContext::CreateShaperCache(GetParam());
  ShapeKey key(U"isolated", 8, FontDescriptor(), 10.f, false);
  auto result = std::make_shared<ShapeResult>(8, false);
  cache1->shape_cache_.AddToCache(key, result);

  EXPECT_NE(cache1, cache2);
  EXPECT_EQ(cache1->shape_cache_.Find(key), result);
  EXPECT_EQ(cache2->shape_cache_.Find(key), nullptr);
}

TEST_P(ShapeCacheTest, ClearRemovesAllEntries) {
  ShapeKey key(U"clear", 5, FontDescriptor(), 10.f, false);
  auto result = std::make_shared<ShapeResult>(1, false);
  auto cache_handle = TTTextContext::CreateShaperCache(GetParam());
  ShapeCache& cache = cache_handle->shape_cache_;
  cache.Clear();
  cache.AddToCache(key, result);

  EXPECT_EQ(cache.Find(key), result);
  cache.Clear();
  EXPECT_EQ(cache.Find(key), nullptr);
}

TEST_P(ShapeCacheTest, ClearRejectsAnInFlightResultFromThePreviousEpoch) {
  ShapeKey key(U"stale", 5, FontDescriptor(), 10.f, false);
  auto result = std::make_shared<ShapeResult>(1, false);
  auto cache_handle = TTTextContext::CreateShaperCache(GetParam());
  ShapeCache& cache = cache_handle->shape_cache_;
  cache.Clear();

  ShapeCache::Epoch epoch = 0;
  EXPECT_EQ(cache.Find(key, &epoch), nullptr);
  cache.Clear();
  cache.AddToCache(key, result, epoch);

  EXPECT_EQ(cache.Find(key), nullptr);
}

TEST(ShapeResultPiece, GlyphCountIncludesLigatureGlyphs) {
  auto result = std::make_shared<ShapeResult>(3, false);
  LigatureShapeResultReader reader;
  result->AppendPlatformShapingResult(reader);

  ShapeResultPiece empty_piece;
  EXPECT_EQ(empty_piece.GlyphCount(), 0u);

  ShapeResultPiece first_char;
  first_char.InitWithShapeResult(result, 0, 1);
  EXPECT_EQ(first_char.GlyphCount(), 1u);

  ShapeResultPiece second_char;
  second_char.InitWithShapeResult(result, 1, 2);
  EXPECT_EQ(second_char.GlyphCount(), 1u);

  ShapeResultPiece ligature_chars;
  ligature_chars.InitWithShapeResult(result, 0, 2);
  EXPECT_EQ(ligature_chars.GlyphCount(), 1u);

  ShapeResultPiece full_range;
  full_range.InitWithShapeResult(result, 0, 3);
  EXPECT_EQ(full_range.GlyphCount(), 2u);
}

#ifdef USE_LRU_CACHE
TEST_P(ShapeCacheTest, LruCacheCapacity) {
  auto int_to_shape_key = [](int i) {
    std::u32string u32str;
    for (auto c : std::to_string(i)) {
      u32str.push_back(static_cast<char32_t>(c));
    }
    return ShapeKey(u32str.c_str(), u32str.length(), FontDescriptor(), 10.f,
                    false);
  };
  constexpr int kDefaultCapacity =
      android::LruCache<const ShapeKey,
                        ShapeResultRef>::Capacity::kDefaultCapacity;
  auto cache_handle = TTTextContext::CreateShaperCache(GetParam());
  ShapeCache& cache = cache_handle->shape_cache_;
  const auto dummy_shape_result = std::make_shared<ShapeResult>(1, false);
  // Fill the cache with kDefaultCapacity entries
  for (int i = 0; i < kDefaultCapacity; i++) {
    cache.AddToCache(int_to_shape_key(i), dummy_shape_result);
  }
  // Check all entries are still in the cache
  for (int i = 0; i < kDefaultCapacity; i++) {
    ShapeKey shape_key = int_to_shape_key(i);
    EXPECT_NE(cache.Find(shape_key), nullptr);
  }
  // Add one more entry, which should evict the least recently used entry
  cache.AddToCache(int_to_shape_key(kDefaultCapacity), dummy_shape_result);
  EXPECT_EQ(cache.Find(int_to_shape_key(0)), nullptr);
}
#endif

TEST_P(ShapeCacheTest, KeepsFirstResultAfterDuplicateMisses) {
  auto cache = TTTextContext::CreateShaperCache(GetParam());
  ShapeKey key(U"duplicate", 9, FontDescriptor(), 10.f, false);
  ShapeCache::Epoch first_epoch = 0;
  ShapeCache::Epoch second_epoch = 0;
  EXPECT_EQ(cache->shape_cache_.Find(key, &first_epoch), nullptr);
  EXPECT_EQ(cache->shape_cache_.Find(key, &second_epoch), nullptr);

  auto first = std::make_shared<ShapeResult>(9, false);
  auto second = std::make_shared<ShapeResult>(9, false);
  cache->shape_cache_.AddToCache(key, first, first_epoch);
  cache->shape_cache_.AddToCache(key, second, second_epoch);

  EXPECT_EQ(cache->shape_cache_.Find(key), first);
}

TEST_P(ShapeCacheTest, ShaperFollowsTheCurrentContext) {
  TTTextContext first_context;
  TTTextContext second_context;
  first_context.SetShaperCache(TTTextContext::CreateShaperCache(GetParam()));
  second_context.SetShaperCache(TTTextContext::CreateShaperCache(GetParam()));
  CacheTestShaper shaper;
  ShapeStyle style(FontDescriptor(), 12.f);
  const auto shape = [&] {
    return shaper.ShapeText(U"cache", 5, &style, false);
  };

  shaper.SetContext(first_context);
  auto first = shape();
  EXPECT_EQ(shape(), first);
  shaper.SetContext(second_context);
  auto second = shape();
  EXPECT_NE(second, first);
  EXPECT_EQ(shape(), second);
  shaper.SetContext(first_context);
  EXPECT_EQ(shape(), first);
  EXPECT_EQ(shaper.shape_count, 2u);
}

TEST_P(ShapeCacheTest, SharedCacheOutlivesContextAndCanBeCleared) {
  FontmgrCollection fonts;
  std::shared_ptr<ShaperCache> cache;
  ShapeResultRef first;
  ShapeStyle style(FontDescriptor(), 12.f);
  {
    TTTextContext context;
    context.SetShaperCache(TTTextContext::CreateShaperCache(GetParam()));
    CacheTestShaper shaper(fonts);
    shaper.SetContext(context);
    first = shaper.ShapeText(U"shared", 6, &style, false);
    cache = context.GetShaperCache();
  }

  TTTextContext context;
  context.SetShaperCache(cache);
  CacheTestShaper shaper(fonts);
  shaper.SetContext(context);
  EXPECT_EQ(shaper.ShapeText(U"shared", 6, &style, false), first);
  EXPECT_EQ(shaper.shape_count, 0u);

  TextLayout::ClearShapeCache(context);
  EXPECT_NE(shaper.ShapeText(U"shared", 6, &style, false), first);
  EXPECT_EQ(shaper.shape_count, 1u);
}

TEST_P(ShapeCacheTest, DisableFeatureBypassesContextCache) {
  TTTextContext context;
  context.SetShaperCache(TTTextContext::CreateShaperCache(GetParam()));
  CacheTestShaper shaper;
  shaper.SetContext(context);
  ShapeStyle style(FontDescriptor(), 12.f);
  const auto shape = [&] {
    return shaper.ShapeText(U"bypass", 6, &style, false);
  };
  auto cached = shape();
  context.EnableFeature(kDisableShapeCache, true);
  auto uncached = shape();
  EXPECT_NE(uncached, cached);
  EXPECT_NE(shape(), uncached);
  context.EnableFeature(kDisableShapeCache, false);
  EXPECT_EQ(shape(), cached);
  EXPECT_EQ(shaper.shape_count, 3u);
}

TEST(ShapeCache, ConcurrentAccessToSharedCache) {
  auto cache = TTTextContext::CreateShaperCache(true);
  ShapeKey key(U"shared", 6, FontDescriptor(), 10.f, false);
  auto result = std::make_shared<ShapeResult>(6, false);
#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
  CTFontRef font = CTFontCreateWithName(CFSTR("Helvetica"), 10.f, nullptr);
  ASSERT_NE(font, nullptr);
#endif
  std::vector<std::thread> threads;
  for (uint32_t i = 0; i < 4; ++i) {
    threads.emplace_back([&] {
      for (uint32_t j = 0; j < 1000; ++j) {
        ShapeCache::Epoch epoch = 0;
        cache->shape_cache_.Find(key, &epoch);
        cache->shape_cache_.AddToCache(key, result, epoch);
#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
        CoreTextFontCache::Epoch font_epoch = 0;
        if (auto cached = cache->font_cache_.Find(key.style_, &font_epoch)) {
          CFRelease(cached);
        }
        auto cached = cache->font_cache_.AddToCache(
            key.style_, static_cast<CTFontRef>(CFRetain(font)), font_epoch);
        CFRelease(cached);
#endif
        if (j % 10 == 0) {
          cache->ClearFontCache();
          cache->ClearShapeCache();
        }
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  cache->ClearFontCache();
  cache->ClearShapeCache();
  cache->shape_cache_.AddToCache(key, result);
  EXPECT_EQ(cache->shape_cache_.Find(key), result);
#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
  CFRelease(font);
#endif
}

TEST(ShapeCache, ContextOnlyOwnsExplicitlyAssignedCache) {
  std::weak_ptr<ShaperCache> cache;
  {
    TTTextContext first_context;
    TTTextContext second_context;
    EXPECT_EQ(first_context.GetShaperCache(), nullptr);
    EXPECT_EQ(second_context.GetShaperCache(), nullptr);
    first_context.Reset();
    EXPECT_EQ(first_context.GetShaperCache(), nullptr);
    first_context.SetShaperCache(TTTextContext::CreateShaperCache());
    cache = first_context.GetShaperCache();
    EXPECT_FALSE(cache.expired());
    EXPECT_NE(first_context.GetShaperCache(), second_context.GetShaperCache());
    first_context.Reset();
    EXPECT_EQ(first_context.GetShaperCache(), cache.lock());
  }
  EXPECT_TRUE(cache.expired());
}

TEST(ShapeCache, NullCacheDisablesCaching) {
  TTTextContext context;
  context.SetShaperCache(TTTextContext::CreateShaperCache());
  CacheTestShaper shaper;
  shaper.SetContext(context);
  ShapeStyle style(FontDescriptor(), 12.f);
  auto cached = shaper.ShapeText(U"uncached", 8, &style, false);
  context.SetShaperCache(nullptr);
  auto first = shaper.ShapeText(U"uncached", 8, &style, false);
  EXPECT_NE(first, cached);
  EXPECT_NE(shaper.ShapeText(U"uncached", 8, &style, false), first);
  EXPECT_EQ(shaper.shape_count, 3u);
  TextLayout::ClearShapeCache(context);
  TextLayout::ClearFontCache(context);
}

#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
TEST_P(ShapeCacheTest, ClearsShapingAndFontCachesIndependently) {
  TTTextContext context;
  context.SetShaperCache(TTTextContext::CreateShaperCache(GetParam()));
  auto cache = context.GetShaperCache();
  ShapeKey key(U"clear", 5, FontDescriptor(), 12.f, false);
  auto result = std::make_shared<ShapeResult>(5, false);
  cache->shape_cache_.AddToCache(key, result);

  for (auto* font_cache : {&cache->system_font_cache_, &cache->font_cache_}) {
    CoreTextFontCache::Epoch epoch = 0;
    EXPECT_EQ(font_cache->Find(key.style_, &epoch), nullptr);
    CTFontRef font = CTFontCreateWithName(CFSTR("Helvetica"), 12.f, nullptr);
    ASSERT_NE(font, nullptr);
    font = font_cache->AddToCache(key.style_, font, epoch);
    CTFontRef cached = font_cache->Find(key.style_, &epoch);
    EXPECT_EQ(cached, font);
    CFRelease(cached);
    CFRelease(font);
  }

  TextLayout::ClearShapeCache(context);
  EXPECT_EQ(cache->shape_cache_.Find(key), nullptr);
  CoreTextFontCache::Epoch epoch = 0;
  for (auto* font_cache : {&cache->system_font_cache_, &cache->font_cache_}) {
    CTFontRef cached = font_cache->Find(key.style_, &epoch);
    ASSERT_NE(cached, nullptr);
    CFRelease(cached);
  }

  cache->shape_cache_.AddToCache(key, result);
  TextLayout::ClearFontCache(context);
  EXPECT_EQ(cache->shape_cache_.Find(key), result);
  EXPECT_EQ(cache->system_font_cache_.Find(key.style_, &epoch), nullptr);
  EXPECT_EQ(cache->font_cache_.Find(key.style_, &epoch), nullptr);
}

TEST_P(ShapeCacheTest, FontCacheEvictsLeastRecentlyUsedFont) {
  auto cache = TTTextContext::CreateShaperCache(GetParam());
  auto& font_cache = cache->font_cache_;
  font_cache.SetMaxEntries(2);
  const ShapeStyle first_key(FontDescriptor(), 12.f);
  const ShapeStyle second_key(FontDescriptor(), 13.f);
  const ShapeStyle third_key(FontDescriptor(), 14.f);
  CoreTextFontCache::Epoch epoch = 0;
  const auto add_font = [&](const ShapeStyle& key) {
    EXPECT_EQ(font_cache.Find(key, &epoch), nullptr);
    CTFontRef font =
        CTFontCreateWithName(CFSTR("Helvetica"), key.GetFontSize(), nullptr);
    EXPECT_NE(font, nullptr);
    return font_cache.AddToCache(key, font, epoch);
  };
  CTFontRef first = add_font(first_key);
  CTFontRef second = add_font(second_key);
  CTFontRef found = font_cache.Find(first_key, &epoch);
  EXPECT_EQ(found, first);
  CFRelease(found);
  CTFontRef third = add_font(third_key);
  EXPECT_EQ(font_cache.Find(second_key, &epoch), nullptr);
  found = font_cache.Find(first_key, &epoch);
  EXPECT_EQ(found, first);
  CFRelease(found);
  found = font_cache.Find(third_key, &epoch);
  EXPECT_EQ(found, third);
  CFRelease(found);
  EXPECT_FLOAT_EQ(CTFontGetSize(second), 13.f);
  font_cache.SetMaxEntries(0);
  EXPECT_EQ(font_cache.Find(first_key, &epoch), nullptr);
  EXPECT_EQ(font_cache.Find(third_key, &epoch), nullptr);
  CFRelease(third);
  CFRelease(second);
  CFRelease(first);
}

TEST_P(ShapeCacheTest, FontCacheRejectsResultsFromBeforeClear) {
  auto cache = TTTextContext::CreateShaperCache(GetParam());
  ShapeStyle key(FontDescriptor(), 12.f);
  CoreTextFontCache::Epoch epoch = 0;
  EXPECT_EQ(cache->font_cache_.Find(key, &epoch), nullptr);
  CTFontRef font = CTFontCreateWithName(CFSTR("Helvetica"), 12.f, nullptr);
  ASSERT_NE(font, nullptr);
  cache->ClearFontCache();
  EXPECT_EQ(cache->font_cache_.AddToCache(key, font, epoch), font);
  EXPECT_EQ(cache->font_cache_.Find(key, &epoch), nullptr);
  EXPECT_FLOAT_EQ(CTFontGetSize(font), 12.f);
  CFRelease(font);
}

TEST_P(ShapeCacheTest, FontCacheKeepsFirstResultAfterDuplicateMisses) {
  auto cache = TTTextContext::CreateShaperCache(GetParam());
  ShapeStyle key(FontDescriptor(), 12.f);
  CoreTextFontCache::Epoch epoch = 0;
  EXPECT_EQ(cache->font_cache_.Find(key, &epoch), nullptr);
  CTFontRef first = CTFontCreateWithName(CFSTR("Helvetica"), 12.f, nullptr);
  CTFontRef second = CTFontCreateWithName(CFSTR("Courier"), 12.f, nullptr);
  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  first = cache->font_cache_.AddToCache(key, first, epoch);
  CTFontRef reused = cache->font_cache_.AddToCache(key, second, epoch);
  EXPECT_EQ(reused, first);
  CFRelease(reused);
  CFRelease(first);
}
#endif

INSTANTIATE_TEST_SUITE_P(LockingModes, ShapeCacheTest, ::testing::Bool());
