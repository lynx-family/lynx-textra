// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_SAFE_FONT_CACHE_H_
#define SRC_PORTS_SHAPER_CORETEXT_SAFE_FONT_CACHE_H_

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>

#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

#include "src/textlayout/shape_cache_scope_internal.h"
#include "src/textlayout/shape_cache_utils.h"

namespace ttoffice {
namespace tttext {

template <typename Key, typename Hasher>
class SafeFontCache final : public ShapeCachePlatformState {
 public:
  using Epoch = uint64_t;

  explicit SafeFontCache(size_t max_entries = 256)
      : max_entries_(max_entries) {}
  ~SafeFontCache() override { Clear(); }

  SafeFontCache(const SafeFontCache&) = delete;
  SafeFontCache& operator=(const SafeFontCache&) = delete;

  CTFontRef Find(const Key& key, Epoch* epoch) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (epoch != nullptr) {
      *epoch = epoch_;
    }
    auto iterator = cache_.find(key);
    if (iterator == cache_.end()) {
      return nullptr;
    }
    lru_.splice(lru_.begin(), lru_, iterator->second.lru_iterator);
    iterator->second.lru_iterator = lru_.begin();
    return static_cast<CTFontRef>(CFRetain(iterator->second.safe_font));
  }

  // Takes ownership of |safe_font|. The caller owns the returned reference.
  CTFontRef Add(const Key& key, CTFontRef safe_font, Epoch epoch) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (epoch != epoch_ || max_entries_ == 0) {
      return safe_font;
    }

    auto existing = cache_.find(key);
    if (existing != cache_.end()) {
      CFRelease(safe_font);
      lru_.splice(lru_.begin(), lru_, existing->second.lru_iterator);
      existing->second.lru_iterator = lru_.begin();
      return static_cast<CTFontRef>(CFRetain(existing->second.safe_font));
    }

    lru_.push_front(key);
    cache_.emplace(key, Entry{safe_font, lru_.begin()});
    TrimLocked();
    return static_cast<CTFontRef>(CFRetain(safe_font));
  }

  void SetMaxEntries(size_t max_entries) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_entries_ = max_entries;
    TrimLocked();
  }

  size_t GetMaxEntries() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return max_entries_;
  }

  void Clear() override {
    std::lock_guard<std::mutex> lock(mutex_);
    ClearLocked();
    ++epoch_;
  }

  void RemoveFamily(const std::string& canonical_family) override {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iterator = cache_.begin(); iterator != cache_.end();) {
      if (FontDescriptorContainsShapeCacheFamily(
              iterator->first.font_descriptor_, canonical_family)) {
        CFRelease(iterator->second.safe_font);
        lru_.erase(iterator->second.lru_iterator);
        iterator = cache_.erase(iterator);
      } else {
        ++iterator;
      }
    }
    ++epoch_;
  }

 private:
  struct Entry {
    CTFontRef safe_font = nullptr;
    typename std::list<Key>::iterator lru_iterator;
  };

  void TrimLocked() {
    while (cache_.size() > max_entries_) {
      auto iterator = cache_.find(lru_.back());
      if (iterator != cache_.end()) {
        CFRelease(iterator->second.safe_font);
        cache_.erase(iterator);
      }
      lru_.pop_back();
    }
  }

  void ClearLocked() {
    for (const auto& entry : cache_) {
      CFRelease(entry.second.safe_font);
    }
    cache_.clear();
    lru_.clear();
  }

  std::unordered_map<Key, Entry, Hasher> cache_;
  std::list<Key> lru_;
  mutable std::mutex mutex_;
  size_t max_entries_;
  Epoch epoch_ = 0;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_CORETEXT_SAFE_FONT_CACHE_H_
