// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/shape_cache.h"

#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#ifdef USE_LRU_CACHE
#include "src/textlayout/utils/lru_cache.h"

namespace android {
template <>
inline uint32_t hash_type<tttext::ShapeKey>(const tttext::ShapeKey& key) {
  return hash_type(static_cast<uint64_t>(key.hash_));
}
}  // namespace android
#endif

namespace ttoffice {
namespace tttext {

class ShapeCache::Impl {
 public:
  void AddToCache(const ShapeKey& key, const ShapeResultRef& result) {
    std::unique_lock lock(mutex_);
    AddToCacheLocked(key, result);
  }

  void AddToCache(const ShapeKey& key, const ShapeResultRef& result,
                  Epoch epoch) {
    std::unique_lock lock(mutex_);
    if (epoch != epoch_) {
      return;
    }
    AddToCacheLocked(key, result);
  }

  ShapeResultRef Find(const ShapeKey& key, Epoch* epoch) {
#ifdef USE_LRU_CACHE
    std::unique_lock lock(mutex_);
#else
    std::shared_lock lock(mutex_);
#endif
    if (epoch != nullptr) {
      *epoch = epoch_;
    }
#ifdef USE_LRU_CACHE
    auto* result = shape_cache_.get(key);
    return result == nullptr ? nullptr : *result;
#else
    auto iter = shape_cache_.find(key);
    return iter == shape_cache_.end() ? nullptr : iter->second;
#endif
  }

  void Clear() {
    std::unique_lock lock(mutex_);
    shape_cache_.clear();
    ++epoch_;
  }

 private:
  void AddToCacheLocked(const ShapeKey& key, const ShapeResultRef& result) {
#ifdef USE_LRU_CACHE
    if (shape_cache_.get(key) == nullptr) {
      shape_cache_.put(key, result);
    }
#else
    shape_cache_.insert({key, result});
#endif
  }

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
  impl_->AddToCache(key, result);
}

void ShapeCache::AddToCache(const ShapeKey& key, const ShapeResultRef& result,
                            Epoch epoch) {
  impl_->AddToCache(key, result, epoch);
}

ShapeResultRef ShapeCache::Find(const ShapeKey& key, Epoch* epoch) {
  return impl_->Find(key, epoch);
}

void ShapeCache::Clear() { impl_->Clear(); }

}  // namespace tttext
}  // namespace ttoffice
