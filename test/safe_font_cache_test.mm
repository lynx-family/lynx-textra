// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/coretext/safe_font_cache.h"

#include <gtest/gtest.h>
#include <textra/font_info.h>
#include <textra/shape_cache_scope.h>

#include <memory>
#include <string>
#include <utility>

#include "src/textlayout/shape_cache_scope_internal.h"

using namespace ttoffice::tttext;

namespace {

struct TestSafeFontKey {
  FontDescriptor font_descriptor_;
  int id = 0;

  bool operator==(const TestSafeFontKey& other) const {
    return font_descriptor_ == other.font_descriptor_ && id == other.id;
  }

  struct Hasher {
    size_t operator()(const TestSafeFontKey& key) const {
      size_t seed = FontDescriptor::Hasher()(key.font_descriptor_);
      seed ^= std::hash<int>()(key.id) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };
};

using TestSafeFontCache =
    SafeFontCache<TestSafeFontKey, TestSafeFontKey::Hasher>;

FontDescriptor MakeDescriptor(std::string family) {
  FontDescriptor descriptor;
  descriptor.font_family_list_.push_back(std::move(family));
  return descriptor;
}

CTFontRef CreateTestFont() {
  return CTFontCreateWithName(CFSTR("Helvetica"), 16.0, nullptr);
}

void AddRetainedFont(TestSafeFontCache* cache, const TestSafeFontKey& key,
                     CTFontRef font) {
  TestSafeFontCache::Epoch epoch = 0;
  EXPECT_EQ(cache->Find(key, &epoch), nullptr);
  CTFontRef result =
      cache->Add(key, static_cast<CTFontRef>(CFRetain(font)), epoch);
  ASSERT_NE(result, nullptr);
  CFRelease(result);
}

TEST(SafeFontCache, ScopeDestructionReleasesCustomFontOnly) {
  TestSafeFontCache global_cache;
  const TestSafeFontKey global_key{MakeDescriptor("Helvetica"), 1};
  const TestSafeFontKey scoped_key{MakeDescriptor("PageFont"), 2};
  CTFontRef global_font = CreateTestFont();
  CTFontRef scoped_font = CreateTestFont();
  ASSERT_NE(global_font, nullptr);
  ASSERT_NE(scoped_font, nullptr);
  AddRetainedFont(&global_cache, global_key, global_font);

  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("PageFont");
  auto route =
      ShapeCacheScopeInternal::CaptureRoute(scope, scoped_key.font_descriptor_);
  static const int kPlatformStateIdentity = 0;
  auto scoped_cache = std::static_pointer_cast<TestSafeFontCache>(
      ShapeCacheScopeInternal::GetOrCreatePlatformState(
          route, &kPlatformStateIdentity,
          []() { return std::make_shared<TestSafeFontCache>(); }));
  ASSERT_NE(scoped_cache, nullptr);
  AddRetainedFont(scoped_cache.get(), scoped_key, scoped_font);
  route.scope.reset();

  const CFIndex global_retain_count = CFGetRetainCount(global_font);
  const CFIndex scoped_retain_count = CFGetRetainCount(scoped_font);
  scoped_cache.reset();
  scope.reset();

  EXPECT_EQ(CFGetRetainCount(scoped_font), scoped_retain_count - 1);
  EXPECT_EQ(CFGetRetainCount(global_font), global_retain_count);
  CTFontRef cached_global_font = global_cache.Find(global_key, nullptr);
  ASSERT_NE(cached_global_font, nullptr);
  CFRelease(cached_global_font);

  global_cache.Clear();
  CFRelease(scoped_font);
  CFRelease(global_font);
}

TEST(SafeFontCache, FamilyUpdateInvalidatesMatchingEntryAndEpoch) {
  auto scope = std::make_shared<ShapeCacheScope>();
  scope->NotifyCustomFontChanged("PageFont");
  const TestSafeFontKey page_key{MakeDescriptor("PAGEFONT"), 1};
  const TestSafeFontKey other_key{MakeDescriptor("OtherFont"), 2};
  const auto route =
      ShapeCacheScopeInternal::CaptureRoute(scope, page_key.font_descriptor_);
  static const int kPlatformStateIdentity = 0;
  auto cache = std::static_pointer_cast<TestSafeFontCache>(
      ShapeCacheScopeInternal::GetOrCreatePlatformState(
          route, &kPlatformStateIdentity,
          []() { return std::make_shared<TestSafeFontCache>(); }));
  ASSERT_NE(cache, nullptr);

  CTFontRef page_font = CreateTestFont();
  CTFontRef other_font = CreateTestFont();
  ASSERT_NE(page_font, nullptr);
  ASSERT_NE(other_font, nullptr);
  AddRetainedFont(cache.get(), page_key, page_font);
  AddRetainedFont(cache.get(), other_key, other_font);

  TestSafeFontCache::Epoch stale_epoch = 0;
  CTFontRef cached_page_font = cache->Find(page_key, &stale_epoch);
  ASSERT_NE(cached_page_font, nullptr);
  CFRelease(cached_page_font);
  scope->NotifyCustomFontChanged("pagefont");

  EXPECT_EQ(cache->Find(page_key, nullptr), nullptr);
  CTFontRef cached_other_font = cache->Find(other_key, nullptr);
  ASSERT_NE(cached_other_font, nullptr);
  CFRelease(cached_other_font);

  CTFontRef stale_font = CreateTestFont();
  ASSERT_NE(stale_font, nullptr);
  CTFontRef stale_result = cache->Add(page_key, stale_font, stale_epoch);
  EXPECT_EQ(stale_result, stale_font);
  CFRelease(stale_result);
  EXPECT_EQ(cache->Find(page_key, nullptr), nullptr);

  cache.reset();
  scope.reset();
  CFRelease(other_font);
  CFRelease(page_font);
}

}  // namespace
