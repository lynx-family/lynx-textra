// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/shape_cache.h"

#include <gtest/gtest.h>
#include <textra/font_info.h>
#include <textra/i_typeface_helper.h>
#include <textra/shape_cache_scope.h>
#include <textra/text_layout.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "src/textlayout/shape_cache_scope_internal.h"
#include "src/textlayout/tt_shaper.h"

using namespace ttoffice::tttext;

namespace {

class TestShapingResultReader final : public PlatformShapingResultReader {
 public:
  explicit TestShapingResultReader(uint32_t glyph_count)
      : advances_(glyph_count),
        positions_(glyph_count),
        glyphs_(glyph_count),
        indices_(glyph_count),
        text_count_(glyph_count) {
    for (uint32_t index = 0; index < glyph_count; ++index) {
      indices_[index] = index;
    }
  }

  uint32_t GlyphCount() const override { return glyphs_.size(); }
  uint32_t TextCount() const override { return text_count_; }
  GlyphID ReadGlyphID(uint32_t index) const override {
    return glyphs_.at(index);
  }
  float ReadAdvanceX(uint32_t index) const override {
    return advances_.at(index)[0];
  }
  float ReadAdvanceY(uint32_t index) const override {
    return advances_.at(index)[1];
  }
  float ReadPositionX(uint32_t index) const override {
    return positions_.at(index)[0];
  }
  float ReadPositionY(uint32_t index) const override {
    return positions_.at(index)[1];
  }
  uint32_t ReadIndices(uint32_t index) const override {
    return indices_.at(index);
  }
  TypefaceRef ReadFontId(uint32_t index) const override { return font_; }

  std::vector<std::array<float, 2>> advances_;
  std::vector<std::array<float, 2>> positions_;
  std::vector<GlyphID> glyphs_;
  std::vector<uint32_t> indices_;
  uint32_t text_count_;
  TypefaceRef font_;
};

class FixedMetricsTypefaceHelper final : public ITypefaceHelper {
 public:
  explicit FixedMetricsTypefaceHelper(uint32_t unique_id)
      : ITypefaceHelper(unique_id) {}

  float GetHorizontalAdvance(GlyphID glyph_id, float font_size) const override {
    return font_size;
  }
  void GetHorizontalAdvances(GlyphID glyph_ids[], uint32_t count,
                             float widths[], float font_size) const override {
    for (uint32_t index = 0; index < count; ++index) {
      widths[index] = font_size;
    }
  }
  void GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                     float font_size) const override {
    rect_ltwh[0] = 0.f;
    rect_ltwh[1] = -0.75f * font_size;
    rect_ltwh[2] = font_size;
    rect_ltwh[3] = font_size;
  }
  void GetWidthBounds(float* rect_ltrb, GlyphID glyphs[], uint32_t glyph_count,
                      float font_size) override {
    rect_ltrb[0] = 0.f;
    rect_ltrb[1] = glyph_count == 0 ? 0.f : -0.75f * font_size;
    rect_ltrb[2] = font_size * static_cast<float>(glyph_count);
    rect_ltrb[3] = glyph_count == 0 ? 0.f : 0.25f * font_size;
  }
  const void* GetFontData() const override { return nullptr; }
  size_t GetFontDataSize() const override { return 0; }
  int GetFontIndex() const override { return 0; }
  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variation_selector = 0) const override {
    return static_cast<uint16_t>(codepoint == 0 ? 0 : 1);
  }
  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override {
    for (uint32_t index = 0; index < count; ++index) {
      glyphs[index] = UnicharToGlyph(unichars[index]);
    }
  }
  uint32_t GetUnitsPerEm() const override { return 1000; }

 protected:
  void OnCreateFontInfo(FontInfo* info, float font_size) const override {
    *info = FontInfo(-0.75f * font_size, 0.25f * font_size, font_size);
  }
};

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

class CountingShaper final : public TTShaper {
 public:
  explicit CountingShaper(std::atomic<int>* shape_count,
                          std::function<void()> on_shape = {})
      : TTShaper(FontmgrCollection()),
        shape_count_(shape_count),
        on_shape_(std::move(on_shape)) {}

  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override {
    ++*shape_count_;
    if (on_shape_) {
      on_shape_();
    }
    TestShapingResultReader reader(static_cast<uint32_t>(key.text_.size()));
    for (size_t index = 0; index < key.text_.size(); ++index) {
      reader.glyphs_[index] = static_cast<GlyphID>(index + 1);
      reader.advances_[index] = {key.style_.GetFontSize(), 0.f};
    }
    result->AppendPlatformShapingResult(reader);
  }

 private:
  std::atomic<int>* shape_count_;
  std::function<void()> on_shape_;
};

ShapeStyle MakeShapeStyle(std::vector<std::string> families,
                          uint64_t platform_font = 0) {
  FontDescriptor descriptor;
  descriptor.font_family_list_ = std::move(families);
  descriptor.platform_font_ = platform_font;
  return ShapeStyle(descriptor, 16.f, false, false);
}

class TestPlatformCacheState final : public ShapeCachePlatformState {
 public:
  void Clear() override { ++clear_count; }
  void RemoveFamily(const std::string& family) override {
    ++remove_count;
    last_removed_family = family;
  }

  int clear_count = 0;
  int remove_count = 0;
  std::string last_removed_family;
};

}  // namespace

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

TEST(ShapeCache, InstancesAreIsolated) {
  ShapeCache first_cache;
  ShapeCache second_cache;
  ShapeKey key(U"instance", 8, FontDescriptor(), 10.f, false, false, false);
  auto result = std::make_shared<ShapeResult>(1, false);

  first_cache.AddToCache(key, result);

  EXPECT_EQ(first_cache.Find(key), result);
  EXPECT_EQ(second_cache.Find(key), nullptr);
}

TEST(ShapeCache, DuplicateInsertionIsIdempotent) {
  ShapeCache cache;
  ShapeKey key(U"duplicate", 9, FontDescriptor(), 10.f, false, false, false);
  auto first_result = std::make_shared<ShapeResult>(1, false);
  auto second_result = std::make_shared<ShapeResult>(1, false);

  cache.AddToCache(key, first_result);
  cache.AddToCache(key, second_result);

  EXPECT_EQ(cache.Find(key), first_result);
}

TEST(ShapeCache, ConcurrentDuplicateInsertionIsIdempotent) {
  ShapeCache cache;
  ShapeKey key(U"concurrent", 10, FontDescriptor(), 10.f, false, false, false);
  std::vector<std::thread> threads;
  for (int index = 0; index < 8; ++index) {
    threads.emplace_back([&cache, &key]() {
      cache.AddToCache(key, std::make_shared<ShapeResult>(1, false));
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_NE(cache.Find(key), nullptr);
}

TEST(ShapeCache, ClearRemovesAllEntries) {
  ShapeKey key(U"clear", 5, FontDescriptor(), 10.f, false, false, false);
  auto result = std::make_shared<ShapeResult>(1, false);
  ShapeCache& cache = ShapeCache::GetInstance();
  cache.Clear();
  cache.AddToCache(key, result);

  EXPECT_EQ(cache.Find(key), result);
  cache.Clear();
  EXPECT_EQ(cache.Find(key), nullptr);
}

TEST(ShapeCache, ClearRejectsAnInFlightResultFromThePreviousEpoch) {
  ShapeKey key(U"stale", 5, FontDescriptor(), 10.f, false, false, false);
  auto result = std::make_shared<ShapeResult>(1, false);
  ShapeCache& cache = ShapeCache::GetInstance();
  cache.Clear();

  ShapeCache::Epoch epoch = 0;
  EXPECT_EQ(cache.Find(key, &epoch), nullptr);
  cache.Clear();
  cache.AddToCache(key, result, epoch);

  EXPECT_EQ(cache.Find(key), nullptr);
}

TEST(ShapeCacheScope, TemporaryContextsShareScopedResults) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("SharedCustom");
  auto style = MakeShapeStyle({"sharedcustom"});
  TTTextContext first_context;
  first_context.SetShapeCacheScope(scope);
  TTTextContext second_context;
  second_context.SetShapeCacheScope(scope);
  std::atomic<int> first_count = 0;
  std::atomic<int> second_count = 0;
  CountingShaper first_shaper(&first_count);
  CountingShaper second_shaper(&second_count);
  first_shaper.SetContext(first_context);
  second_shaper.SetContext(second_context);

  first_shaper.ShapeText(U"scope", 5, &style, false);
  second_shaper.ShapeText(U"scope", 5, &style, false);

  EXPECT_EQ(first_count, 1);
  EXPECT_EQ(second_count, 0);
}

TEST(ShapeCacheScope, SeparateScopesDoNotShareCustomResults) {
  auto first_scope = std::make_shared<ShapeCacheScope>();
  auto second_scope = std::make_shared<ShapeCacheScope>();
  first_scope->NotifyCustomFontChanged("isolated");
  second_scope->NotifyCustomFontChanged("isolated");
  auto style = MakeShapeStyle({"isolated"});
  TTTextContext first_context;
  first_context.SetShapeCacheScope(first_scope);
  TTTextContext second_context;
  second_context.SetShapeCacheScope(second_scope);
  std::atomic<int> first_count = 0;
  std::atomic<int> second_count = 0;
  CountingShaper first_shaper(&first_count);
  CountingShaper second_shaper(&second_count);
  first_shaper.SetContext(first_context);
  second_shaper.SetContext(second_context);

  first_shaper.ShapeText(U"isolate", 7, &style, false);
  second_shaper.ShapeText(U"isolate", 7, &style, false);

  EXPECT_EQ(first_count, 1);
  EXPECT_EQ(second_count, 1);
}

TEST(ShapeCacheScope, LegacyContextsShareGlobalResults) {
  ShapeCache::GetInstance().Clear();
  auto style = MakeShapeStyle({"system"});
  TTTextContext first_context;
  TTTextContext second_context;
  std::atomic<int> first_count = 0;
  std::atomic<int> second_count = 0;
  CountingShaper first_shaper(&first_count);
  CountingShaper second_shaper(&second_count);
  first_shaper.SetContext(first_context);
  second_shaper.SetContext(second_context);

  first_shaper.ShapeText(U"global", 6, &style, false);
  second_shaper.ShapeText(U"global", 6, &style, false);

  EXPECT_EQ(first_count, 1);
  EXPECT_EQ(second_count, 0);
}

TEST(ShapeCacheScope, LoadedFamilyLeavesExistingGlobalFallbackUsable) {
  ShapeCache::GetInstance().Clear();
  auto current_scope = std::make_shared<ShapeCacheScope>();
  auto other_scope = std::make_shared<ShapeCacheScope>();
  auto style = MakeShapeStyle({"loadable"});
  TTTextContext current_context;
  current_context.SetShapeCacheScope(current_scope);
  TTTextContext other_context;
  other_context.SetShapeCacheScope(other_scope);
  std::atomic<int> current_count = 0;
  std::atomic<int> other_count = 0;
  CountingShaper current_shaper(&current_count);
  CountingShaper other_shaper(&other_count);
  current_shaper.SetContext(current_context);
  other_shaper.SetContext(other_context);

  current_shaper.ShapeText(U"fallback", 8, &style, false);
  current_scope->NotifyCustomFontChanged("loadable");
  current_shaper.ShapeText(U"fallback", 8, &style, false);
  other_shaper.ShapeText(U"fallback", 8, &style, false);

  EXPECT_EQ(current_count, 2);
  EXPECT_EQ(other_count, 0);
}

TEST(ShapeCacheScope, DisableShapeCacheBypassesScopedResults) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("disabled");
  auto style = MakeShapeStyle({"disabled"});
  TTTextContext context;
  context.SetShapeCacheScope(scope);
  context.EnableFeature(kDisableShapeCache, true);
  std::atomic<int> shape_count = 0;
  CountingShaper shaper(&shape_count);
  shaper.SetContext(context);

  shaper.ShapeText(U"disabled", 8, &style, false);
  shaper.ShapeText(U"disabled", 8, &style, false);

  EXPECT_EQ(shape_count, 2);
}

TEST(ShapeCacheScope, AnyCustomFamilyAndPlatformFontsUseScope) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("CUSTOM");
  auto mixed_style = MakeShapeStyle({"system", "custom"});
  auto platform_style = MakeShapeStyle({"system"}, 1);
  auto mixed_route = ShapeCacheScopeInternal::CaptureRoute(
      scope, mixed_style.GetFontDescriptor());
  auto platform_route = ShapeCacheScopeInternal::CaptureRoute(
      scope, platform_style.GetFontDescriptor());

  EXPECT_TRUE(mixed_route.use_scoped_cache);
  EXPECT_TRUE(platform_route.use_scoped_cache);
}

TEST(ShapeCacheScope, FamilyUpdateInvalidatesOnlyThatFamily) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("first");
  scope->NotifyCustomFontChanged("second");
  auto first_style = MakeShapeStyle({"first"});
  auto second_style = MakeShapeStyle({"second"});
  TTTextContext context;
  context.SetShapeCacheScope(scope);
  std::atomic<int> shape_count = 0;
  CountingShaper shaper(&shape_count);
  shaper.SetContext(context);
  shaper.ShapeText(U"family", 6, &first_style, false);
  shaper.ShapeText(U"family", 6, &second_style, false);

  scope->NotifyCustomFontChanged("FIRST");
  shaper.ShapeText(U"family", 6, &first_style, false);
  shaper.ShapeText(U"family", 6, &second_style, false);

  EXPECT_EQ(shape_count, 3);
}

TEST(ShapeCacheScope, ClearPreservesCustomFamilyRouting) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("persistent");
  auto style = MakeShapeStyle({"PERSISTENT"});

  scope->Clear();
  auto route =
      ShapeCacheScopeInternal::CaptureRoute(scope, style.GetFontDescriptor());

  EXPECT_TRUE(route.use_scoped_cache);
}

TEST(ShapeCacheScope, LegacyGlobalClearDoesNotClearScopedResults) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("scoped");
  auto style = MakeShapeStyle({"scoped"});
  TTTextContext context;
  context.SetShapeCacheScope(scope);
  std::atomic<int> shape_count = 0;
  CountingShaper shaper(&shape_count);
  shaper.SetContext(context);
  shaper.ShapeText(U"legacy-clear", 12, &style, false);

  TextLayout::ClearShapeCache();
  shaper.ShapeText(U"legacy-clear", 12, &style, false);

  EXPECT_EQ(shape_count, 1);
}

TEST(ShapeCacheScope, ClearRejectsPreviouslyCapturedGeneration) {
  auto scope = std::make_shared<ShapeCacheScope>();
  auto style = MakeShapeStyle({"system"});
  auto route =
      ShapeCacheScopeInternal::CaptureRoute(scope, style.GetFontDescriptor());
  bool executed = false;

  scope->Clear();

  EXPECT_FALSE(
      ShapeCacheScopeInternal::RunIfCurrent(route, [&]() { executed = true; }));
  EXPECT_FALSE(executed);
}

TEST(ShapeCacheScope, EagerTwoPhaseNotificationDropsWindowFallback) {
  auto scope = std::make_shared<ShapeCacheScope>();
  auto style = MakeShapeStyle({"eager"});
  TTTextContext context;
  context.SetShapeCacheScope(scope);
  std::atomic<int> shape_count = 0;
  CountingShaper shaper(&shape_count);
  shaper.SetContext(context);

  scope->NotifyCustomFontChanged("eager");
  shaper.ShapeText(U"window", 6, &style, false);
  scope->NotifyCustomFontChanged("eager");
  shaper.ShapeText(U"window", 6, &style, false);

  EXPECT_EQ(shape_count, 2);
}

TEST(ShapeCacheScope, LazyNotificationPreventsGlobalWriteBack) {
  ShapeCache::GetInstance().Clear();
  auto scope = std::make_shared<ShapeCacheScope>();
  auto style = MakeShapeStyle({"lazy"});
  TTTextContext context;
  context.SetShapeCacheScope(scope);
  std::atomic<int> shape_count = 0;
  std::atomic<bool> notified = false;
  CountingShaper shaper(&shape_count, [&]() {
    if (!notified.exchange(true)) {
      scope->NotifyCustomFontChanged("lazy");
    }
  });
  shaper.SetContext(context);

  shaper.ShapeText(U"lazy", 4, &style, false);
  shaper.ShapeText(U"lazy", 4, &style, false);
  shaper.ShapeText(U"lazy", 4, &style, false);

  EXPECT_EQ(shape_count, 2);
}

TEST(ShapeCacheScope, InFlightGlobalResultIsRejectedAfterFamilyUpdate) {
  ShapeCache::GetInstance().Clear();
  auto scope = std::make_shared<ShapeCacheScope>();
  auto style = MakeShapeStyle({"racing"});
  TTTextContext context;
  context.SetShapeCacheScope(scope);
  std::atomic<int> shape_count = 0;
  std::mutex mutex;
  std::condition_variable condition;
  bool shaping_started = false;
  bool may_finish = false;
  std::atomic<bool> first_shape = true;
  CountingShaper shaper(&shape_count, [&]() {
    if (!first_shape.exchange(false)) {
      return;
    }
    std::unique_lock lock(mutex);
    shaping_started = true;
    condition.notify_all();
    condition.wait(lock, [&]() { return may_finish; });
  });
  shaper.SetContext(context);

  std::thread shaping_thread(
      [&]() { shaper.ShapeText(U"racing", 6, &style, false); });
  {
    std::unique_lock lock(mutex);
    condition.wait(lock, [&]() { return shaping_started; });
  }
  scope->NotifyCustomFontChanged("racing");
  {
    std::lock_guard lock(mutex);
    may_finish = true;
  }
  condition.notify_all();
  shaping_thread.join();
  shaper.ShapeText(U"racing", 6, &style, false);

  EXPECT_EQ(shape_count, 2);
}

TEST(ShapeCacheScope, InFlightScopedResultIsRejectedAfterClear) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("cleared");
  auto style = MakeShapeStyle({"cleared"});
  TTTextContext context;
  context.SetShapeCacheScope(scope);
  std::atomic<int> shape_count = 0;
  std::mutex mutex;
  std::condition_variable condition;
  bool shaping_started = false;
  bool may_finish = false;
  std::atomic<bool> first_shape = true;
  CountingShaper shaper(&shape_count, [&]() {
    if (!first_shape.exchange(false)) {
      return;
    }
    std::unique_lock lock(mutex);
    shaping_started = true;
    condition.notify_all();
    condition.wait(lock, [&]() { return may_finish; });
  });
  shaper.SetContext(context);

  std::thread shaping_thread(
      [&]() { shaper.ShapeText(U"cleared", 7, &style, false); });
  {
    std::unique_lock lock(mutex);
    condition.wait(lock, [&]() { return shaping_started; });
  }
  scope->Clear();
  {
    std::lock_guard lock(mutex);
    may_finish = true;
  }
  condition.notify_all();
  shaping_thread.join();
  shaper.ShapeText(U"cleared", 7, &style, false);

  EXPECT_EQ(shape_count, 2);
}

TEST(ShapeCacheScope, ScopeOwnsAndInvalidatesPlatformCacheState) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("platform");
  auto style = MakeShapeStyle({"platform"});
  auto route =
      ShapeCacheScopeInternal::CaptureRoute(scope, style.GetFontDescriptor());
  static const int kIdentity = 0;
  auto state = std::static_pointer_cast<TestPlatformCacheState>(
      ShapeCacheScopeInternal::GetOrCreatePlatformState(
          route, &kIdentity,
          []() { return std::make_shared<TestPlatformCacheState>(); }));

  scope->NotifyCustomFontChanged("PLATFORM");
  scope->Clear();

  ASSERT_NE(state, nullptr);
  EXPECT_EQ(state->remove_count, 1);
  EXPECT_EQ(state->last_removed_family, "platform");
  EXPECT_EQ(state->clear_count, 1);
}

TEST(ShapeCacheScope, DestructionReleasesCachedTypeface) {
  std::weak_ptr<ITypefaceHelper> weak_typeface;
  {
    auto scope = std::make_shared<ShapeCacheScope>();
    scope->NotifyCustomFontChanged("lifetime");
    auto typeface = std::make_shared<FixedMetricsTypefaceHelper>(42);
    weak_typeface = typeface;
    auto result = std::make_shared<ShapeResult>(1, false);
    TestShapingResultReader reader(1);
    reader.glyphs_[0] = 1;
    reader.font_ = typeface;
    result->AppendPlatformShapingResult(reader);
    auto style = MakeShapeStyle({"lifetime"});
    ShapeKey key(U"lifetime", 8, FontDescriptor(style.GetFontDescriptor()),
                 16.f, false, false, false);
    auto route =
        ShapeCacheScopeInternal::CaptureRoute(scope, style.GetFontDescriptor());
    ShapeCacheScopeInternal::GetShapeCache(route).AddToCache(key, result);
    reader.font_.reset();
    typeface.reset();
    result.reset();
    EXPECT_FALSE(weak_typeface.expired());
  }

  EXPECT_TRUE(weak_typeface.expired());
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
