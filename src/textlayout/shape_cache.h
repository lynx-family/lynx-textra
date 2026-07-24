// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_SHAPE_CACHE_H_
#define SRC_TEXTLAYOUT_SHAPE_CACHE_H_

#include <textra/font_info.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include "src/textlayout/tt_shaper.h"
namespace ttoffice {
namespace tttext {
class ShapeResultPiece {
 public:
  ShapeResultPiece() = default;

 public:
  void InitWithShapeResult(const std::shared_ptr<ShapeResult>& result,
                           const uint32_t& start_char_pos,
                           const uint32_t& end_char_pos) {
    result_ = result;
    start_char_pos_ = start_char_pos;
    end_char_pos_ = end_char_pos;
  }

  bool Valid() const { return CharCount() > 0; }
  uint32_t CharCount() const { return end_char_pos_ - start_char_pos_; }
  uint32_t GlyphCount() const {
    if (CharCount() == 0 || result_ == nullptr) {
      return 0;
    }
    // Account for non-1:1 char/glyph mappings. The two end-position
    // calculations can each be biased for one-to-many or many-to-one
    // mappings, so use the larger value to cover the full glyph range.
    auto end_glyph_pos = std::max(result_->CharToGlyph(end_char_pos_ - 1) + 1,
                                  result_->CharToGlyph(end_char_pos_));
    return end_glyph_pos - result_->CharToGlyph(start_char_pos_);
  }
  const GlyphID& Glyphs(const uint32_t& glyph_idx) const {
    return result_->Glyphs(result_->CharToGlyph(start_char_pos_) + glyph_idx);
  }
  const float* Advances(const uint32_t& glyph_idx) const {
    return result_->Advances(result_->CharToGlyph(start_char_pos_) + glyph_idx);
  }
  const float* Positions(const uint32_t& glyph_idx) const {
    return result_->Positions(result_->CharToGlyph(start_char_pos_) +
                              glyph_idx);
  }
  const TypefaceRef& Font(const uint32_t& glyph_idx) const {
    return result_->Font(result_->CharToGlyph(start_char_pos_) + glyph_idx);
  }
  const TypefaceRef& FontByCharId(const uint32_t& char_idx) const {
    return result_->FontByCharId(char_idx + start_char_pos_);
  }
  uint32_t CharToGlyph(const uint32_t& char_idx) const {
    return result_->CharToGlyph(char_idx + start_char_pos_) -
           result_->CharToGlyph(start_char_pos_);
  }
  uint32_t GlyphToChar(const uint32_t& glyph_idx) const {
    auto idx = glyph_idx + result_->CharToGlyph(start_char_pos_);
    return result_->GlyphToChar(idx) - start_char_pos_;
  }
  bool IsRTL() const { return result_->IsRTL(); }

  float MeasureWidth(uint32_t start_char, uint32_t char_count,
                     float letter_spacing) const {
    return result_->MeasureWidth(start_char_pos_ + start_char, char_count,
                                 letter_spacing);
  }

 private:
  std::shared_ptr<const ShapeResult> result_;
  uint32_t start_char_pos_{};
  uint32_t end_char_pos_{};
};
}  // namespace tttext
}  // namespace ttoffice
using ShapeResultRef = std::shared_ptr<tttext::ShapeResult>;
#ifdef USE_LRU_CACHE
#include "src/textlayout/utils/lru_cache.h"
namespace android {
template <>
inline uint32_t hash_type<tttext::ShapeKey>(const tttext::ShapeKey& key) {
  return hash_type(static_cast<uint64_t>(key.hash_));
}
}  // namespace android

#endif

namespace std {
template <>
struct hash<const tttext::ShapeKey> {
  size_t operator()(const tttext::ShapeKey& record) const noexcept {
    return record.hash_;
  }
};

template <>
struct hash<tttext::ShapeKey> {
  size_t operator()(const tttext::ShapeKey& record) const noexcept {
    return record.hash_;
  }
};
}  // namespace std

namespace ttoffice {
namespace tttext {
class ShapeCache final {
 public:
  using Epoch = uint64_t;

  static ShapeCache& GetInstance() {
    static ShapeCache* instance = new ShapeCache();
    return *instance;
  }

  ShapeCache(const ShapeCache& other) = delete;
  void operator=(const ShapeCache& other) = delete;

 private:
  ShapeCache() = default;
  ~ShapeCache() = default;

 public:
  void AddToCache(const ShapeKey& key, const ShapeResultRef& result) {
    std::unique_lock lock(mutex_);
    AddToCacheLocked(key, result);
  }

  void AddToCache(const ShapeKey& key, const ShapeResultRef& result,
                  Epoch epoch) {
    std::unique_lock lock(mutex_);
    // A font-manager update may clear the cache while shaping is in flight.
    // Do not let a result made with the old font set repopulate the cache.
    if (epoch != epoch_) {
      return;
    }
    AddToCacheLocked(key, result);
  }

  std::shared_ptr<ShapeResult> Find(const ShapeKey& key,
                                    Epoch* epoch = nullptr) {
    std::shared_lock lock(mutex_);
    if (epoch != nullptr) {
      *epoch = epoch_;
    }
#ifdef USE_LRU_CACHE
    auto iter = shape_cache_.get(key);
    return iter == nullptr ? nullptr : *iter;
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
    TTASSERT(shape_cache_.get(key) == nullptr);
    shape_cache_.put(key, result);
#else
    TTASSERT(shape_cache_.find(key) == shape_cache_.end());
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
  mutable std::shared_mutex mutex_{};
  Epoch epoch_ = 0;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_SHAPE_CACHE_H_
