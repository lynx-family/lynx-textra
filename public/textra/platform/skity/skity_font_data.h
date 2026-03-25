// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_DATA_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_DATA_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>

#include <skity/text/font.hpp>
#include <skity/text/typeface.hpp>

namespace skity {

namespace textlayout {

class SkityFontData {
 public:
  explicit SkityFontData(std::shared_ptr<Typeface> typeface)
      : typeface_(std::move(typeface)) {}

  std::shared_ptr<Typeface> GetTypeface() const { return typeface_; }

  std::pair<float, float> GetAscentAndDescent(float text_size) const {
    EnsureEmHeightCached();
    uint32_t units_per_em = typeface_ ? typeface_->GetUnitsPerEm() : 0u;
    if (!has_em_height_ || units_per_em == 0) {
      return GetFallbackAscentAndDescent(text_size);
    }

    float scale = text_size / static_cast<float>(units_per_em);
    return std::make_pair(static_cast<float>(em_height_ascender_) * scale,
                          static_cast<float>(em_height_descender_) * scale);
  }

  float GetAscent(float text_size) const {
    return GetAscentAndDescent(text_size).first;
  }

  float GetDescent(float text_size) const {
    return GetAscentAndDescent(text_size).second;
  }

  bool HasEmHeight() const {
    EnsureEmHeightCached();
    return has_em_height_;
  }

 private:
  static constexpr FontTableTag SetFourByteTag(char a, char b, char c,
                                               char d) noexcept {
    return (static_cast<FontTableTag>(static_cast<uint8_t>(a)) << 24) |
           (static_cast<FontTableTag>(static_cast<uint8_t>(b)) << 16) |
           (static_cast<FontTableTag>(static_cast<uint8_t>(c)) << 8) |
           static_cast<FontTableTag>(static_cast<uint8_t>(d));
  }

  static constexpr int16_t ByteSwap(int16_t value) noexcept {
    uint16_t u = static_cast<uint16_t>(value);
    u = static_cast<uint16_t>((u << 8) | (u >> 8));
    return static_cast<int16_t>(u);
  }

  void EnsureEmHeightCached() const {
    std::call_once(em_height_once_, [this]() {
      if (!typeface_) {
        return;
      }

      int16_t buffer[2] = {0, 0};
      size_t size = typeface_->GetTableData(SetFourByteTag('O', 'S', '/', '2'),
                                            68, sizeof(buffer), buffer);
      if (size != sizeof(buffer)) {
        return;
      }

      em_height_ascender_ = ByteSwap(buffer[0]);
      em_height_descender_ = -ByteSwap(buffer[1]);
      has_em_height_ = true;
    });
  }

  std::pair<float, float> GetFallbackAscentAndDescent(float text_size) const {
    if (!typeface_) {
      return std::make_pair(0.f, 0.f);
    }

    FontMetrics metrics{};
    Font font(typeface_, text_size);
    font.GetMetrics(&metrics);
    return std::make_pair(-metrics.ascent_, metrics.descent_);
  }

  std::shared_ptr<Typeface> typeface_;
  mutable std::once_flag em_height_once_;
  mutable bool has_em_height_ = false;
  mutable int16_t em_height_ascender_ = 0;
  mutable int16_t em_height_descender_ = 0;
};

}  // namespace textlayout

}  // namespace skity

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_FONT_DATA_H_
