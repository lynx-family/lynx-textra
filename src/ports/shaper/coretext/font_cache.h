// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_FONT_CACHE_H_
#define SRC_PORTS_SHAPER_CORETEXT_FONT_CACHE_H_

#include <CoreText/CoreText.h>

#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {

class CoreTextFontCache final {
 public:
  using Epoch = uint64_t;

  explicit CoreTextFontCache(bool thread_safe)
      : mutex_(thread_safe ? std::make_unique<std::mutex>() : nullptr) {}
  ~CoreTextFontCache() { ClearEntries(); }

  // Returns a caller-owned font reference.
  CTFontRef Find(const ShapeStyle& key, Epoch* epoch) {
    auto lock = Lock();
    *epoch = epoch_;
    auto iter = cache_.find(key);
    if (iter == cache_.end()) {
      return nullptr;
    }
    lru_.splice(lru_.begin(), lru_, iter->second.lru_it_);
    return static_cast<CTFontRef>(CFRetain(iter->second.font_));
  }

  // Consumes font's reference and returns a caller-owned reference, possibly
  // to an existing font produced after a concurrent miss.
  CTFontRef AddToCache(const ShapeStyle& key, CTFontRef font, Epoch epoch) {
    auto lock = Lock();
    if (epoch != epoch_ || max_entries_ == 0) {
      return font;
    }
    auto iter = cache_.find(key);
    if (iter != cache_.end()) {
      CFRelease(font);
      lru_.splice(lru_.begin(), lru_, iter->second.lru_it_);
      return static_cast<CTFontRef>(CFRetain(iter->second.font_));
    }
    lru_.push_front(key);
    cache_.emplace(key,
                   Entry{static_cast<CTFontRef>(CFRetain(font)), lru_.begin()});
    Trim();
    return font;
  }

  void SetMaxEntries(size_t max_entries) {
    auto lock = Lock();
    max_entries_ = max_entries;
    Trim();
  }

  size_t GetMaxEntries() {
    auto lock = Lock();
    return max_entries_;
  }

  void Clear() {
    auto lock = Lock();
    ClearEntries();
    ++epoch_;
  }

 private:
  struct Entry {
    CTFontRef font_;
    std::list<ShapeStyle>::iterator lru_it_;
  };

  std::unique_lock<std::mutex> Lock() {
    return mutex_ ? std::unique_lock<std::mutex>(*mutex_)
                  : std::unique_lock<std::mutex>();
  }

  void ClearEntries() {
    for (auto& entry : cache_) {
      CFRelease(entry.second.font_);
    }
    cache_.clear();
    lru_.clear();
  }

  void Trim() {
    while (cache_.size() > max_entries_) {
      auto iter = cache_.find(lru_.back());
      CFRelease(iter->second.font_);
      cache_.erase(iter);
      lru_.pop_back();
    }
  }

  std::unordered_map<ShapeStyle, Entry> cache_;
  std::list<ShapeStyle> lru_;
  std::unique_ptr<std::mutex> mutex_;
  size_t max_entries_ = 256;
  Epoch epoch_ = 0;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_CORETEXT_FONT_CACHE_H_
