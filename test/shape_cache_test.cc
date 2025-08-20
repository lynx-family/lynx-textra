// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "shape_cache.h"

#include <gtest/gtest.h>
#include <textra/font_info.h>

#include "src/textlayout/tt_shaper.h"
#include "test_utils.h"

using namespace ttoffice::tttext;

TEST(ShapeCache, AddToCacheAndFind) {
  FontDescriptor font1;
  FontDescriptor font2;
  font2.font_style_ = FontStyle::Bold();
  ShapeKey key1(U"1", 2, FontDescriptor(font1), 10.f, false, false, false);
  ShapeKey key2(U"A", 2, FontDescriptor(font1), 10.f, false, false, false);
  ShapeKey key3(U"1", 2, FontDescriptor(font2), 10.f, false, false, false);
  ShapeKey key4(U"1", 2, FontDescriptor(font1), 20.f, false, false, false);
  ShapeKey key5(U"1", 2, FontDescriptor(font1), 10.f, true, false, false);
  ShapeKey key6(U"1", 2, FontDescriptor(font1), 10.f, false, true, false);
  ShapeKey key7(U"1", 2, FontDescriptor(font1), 10.f, false, false, true);

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
  const ShapeResultRef result6 = create_fake_shape_result({6});
  const ShapeResultRef result7 = create_fake_shape_result({7});

  ShapeCache& cache = ShapeCache::GetInstance();
  cache.AddToCache(key1, result1);
  cache.AddToCache(key2, result2);
  cache.AddToCache(key3, result3);
  cache.AddToCache(key4, result4);
  cache.AddToCache(key5, result5);
  cache.AddToCache(key6, result6);
  cache.AddToCache(key7, result7);

  EXPECT_EQ(cache.Find(key1), result1);
  EXPECT_EQ(cache.Find(key2), result2);
  EXPECT_EQ(cache.Find(key3), result3);
  EXPECT_EQ(cache.Find(key4), result4);
  EXPECT_EQ(cache.Find(key5), result5);
  EXPECT_EQ(cache.Find(key6), result6);
  EXPECT_EQ(cache.Find(key7), result7);
}

TEST(ShapeCache, Singleton) {
  ShapeCache& cache1 = ShapeCache::GetInstance();
  ShapeCache& cache2 = ShapeCache::GetInstance();
  EXPECT_EQ(&cache1, &cache2);
}

#ifdef USE_LRU_CACHE
TEST(ShapeCache, LruCacheCapacity) {
  auto int_to_shape_key = [](int i) {
    std::u32string u32str;
    for (auto c : std::to_string(i)) {
      u32str.push_back(static_cast<char32_t>(c));
    }
    return ShapeKey(u32str.c_str(), u32str.length(), FontDescriptor(), 10.f,
                    false, false, false);
  };
  constexpr int kDefaultCapacity =
      android::LruCache<const ShapeKey,
                        ShapeResultRef>::Capacity::kDefaultCapacity;
  ShapeCache& cache = ShapeCache::GetInstance();
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
