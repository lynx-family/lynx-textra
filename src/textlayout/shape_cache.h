// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_SHAPE_CACHE_H_
#define SRC_TEXTLAYOUT_SHAPE_CACHE_H_

#include <textra/font_info.h>

#include <memory>
#include <mutex>
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
    return result_->CharToGlyph(end_char_pos_) -
           result_->CharToGlyph(start_char_pos_);
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
inline uint32_t hash_type<ttoffice::tttext::ShapeKey>(
    const ttoffice::tttext::ShapeKey& key) {
  return hash_type((uint64_t)key.hash_);
}
}  // namespace android

#endif

namespace std {
// template <>
// struct hash<ttoffice::textlayout::ShapeStyle> {
//   auto operator()(const ttoffice::textlayout::ShapeStyle& record) const
//       -> size_t {
//     return record.hash_;
//   }
// };

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
  static ShapeCache& GetInstance() {
    static ShapeCache instance;
    return instance;
  }

  ShapeCache(const ShapeCache& other) = delete;
  void operator=(const ShapeCache& other) = delete;

 private:
  ShapeCache() = default;
  ~ShapeCache() = default;

 public:
  void AddToCache(const ShapeKey& key, const ShapeResultRef& result) {
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef USE_LRU_CACHE
    TTASSERT(shape_cache_.get(key) == nullptr);
    shape_cache_.put(key, result);
#else
    TTASSERT(shape_cache_.find(key) == shape_cache_.end());
    shape_cache_.insert({key, result});
#endif
  }
  std::shared_ptr<ShapeResult> Find(const ShapeKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef USE_LRU_CACHE
    auto iter = shape_cache_.get(key);
    return iter == nullptr ? nullptr : *iter;
#else
    auto iter = shape_cache_.find(key);
    return iter == shape_cache_.end() ? nullptr : iter->second;
#endif
  }

 private:
#ifdef USE_LRU_CACHE
  android::LruCache<const ShapeKey, ShapeResultRef> shape_cache_{
      android::LruCache<const ShapeKey,
                        ShapeResultRef>::Capacity::kDefaultCapacity};
#else
  std::unordered_map<const ShapeKey, ShapeResultRef> shape_cache_;
#endif
  std::mutex mutex_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_SHAPE_CACHE_H_
