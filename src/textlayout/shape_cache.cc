// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/shape_cache.h"

#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/textlayout/shape_cache_utils.h"

namespace ttoffice {
namespace tttext {

class ShapeCache::Impl final {
 public:
  void Add(const ShapeKey& key, const ShapeResultRef& result, Epoch* epoch) {
    std::unique_lock lock(mutex_);
    if (epoch != nullptr && *epoch != epoch_) {
      return;
    }
#ifdef USE_LRU_CACHE
    if (shape_cache_.get(key) == nullptr) {
      shape_cache_.put(key, result);
    }
#else
    shape_cache_.emplace(key, result);
#endif
  }

  ShapeResultRef Find(const ShapeKey& key, Epoch* epoch) {
#ifdef USE_LRU_CACHE
    // LruCache::get updates recency ordering. It is a write operation.
    std::unique_lock lock(mutex_);
#else
    std::shared_lock lock(mutex_);
#endif
    if (epoch != nullptr) {
      *epoch = epoch_;
    }
#ifdef USE_LRU_CACHE
    auto* found = shape_cache_.get(key);
    return found == nullptr ? nullptr : *found;
#else
    auto found = shape_cache_.find(key);
    return found == shape_cache_.end() ? nullptr : found->second;
#endif
  }

  void Clear() {
    std::unique_lock lock(mutex_);
    shape_cache_.clear();
    ++epoch_;
  }

  void RemoveFamily(const std::string& canonical_family) {
    std::unique_lock lock(mutex_);
#ifdef USE_LRU_CACHE
    std::vector<ShapeKey> keys_to_remove;
    android::LruCache<const ShapeKey, ShapeResultRef>::Iterator iterator(
        shape_cache_);
    while (iterator.next()) {
      if (FontDescriptorContainsShapeCacheFamily(
              iterator.key().style_.GetFontDescriptor(), canonical_family)) {
        keys_to_remove.push_back(iterator.key());
      }
    }
    for (const auto& key : keys_to_remove) {
      shape_cache_.remove(key);
    }
#else
    for (auto iterator = shape_cache_.begin();
         iterator != shape_cache_.end();) {
      if (FontDescriptorContainsShapeCacheFamily(
              iterator->first.style_.GetFontDescriptor(), canonical_family)) {
        iterator = shape_cache_.erase(iterator);
      } else {
        ++iterator;
      }
    }
#endif
    ++epoch_;
  }

 private:
#ifdef USE_LRU_CACHE
  android::LruCache<const ShapeKey, ShapeResultRef> shape_cache_{
      android::LruCache<const ShapeKey,
                        ShapeResultRef>::Capacity::kDefaultCapacity};
#else
  std::unordered_map<const ShapeKey, ShapeResultRef> shape_cache_;
#endif
  mutable std::shared_mutex mutex_;
  Epoch epoch_ = 0;
};

ShapeCache::ShapeCache() : impl_(std::make_unique<Impl>()) {}

ShapeCache::~ShapeCache() = default;

ShapeCache& ShapeCache::GetInstance() {
  static ShapeCache* instance = new ShapeCache();
  return *instance;
}

void ShapeCache::AddToCache(const ShapeKey& key, const ShapeResultRef& result) {
  impl_->Add(key, result, nullptr);
}

void ShapeCache::AddToCache(const ShapeKey& key, const ShapeResultRef& result,
                            Epoch epoch) {
  impl_->Add(key, result, &epoch);
}

ShapeResultRef ShapeCache::Find(const ShapeKey& key, Epoch* epoch) {
  return impl_->Find(key, epoch);
}

void ShapeCache::Clear() { impl_->Clear(); }

void ShapeCache::RemoveFamily(const std::string& canonical_family) {
  impl_->RemoveFamily(canonical_family);
}

}  // namespace tttext
}  // namespace ttoffice
