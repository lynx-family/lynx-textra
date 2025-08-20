// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_TT_COLOR_H_
#define PUBLIC_TEXTRA_TT_COLOR_H_
#include <textra/macro.h>

#include <string>
namespace ttoffice {
namespace tttext {
enum class ColorType : uint8_t;

/**
 * @brief A color in ARGB format.
 */
class L_EXPORT TTColor {
 public:
  static constexpr uint32_t RED() { return 0xFFFF0000; }
  static constexpr uint32_t GREEN() { return 0xFF00FF00; }
  static constexpr uint32_t BLUE() { return 0xFF0000FF; }
  static constexpr uint32_t WHITE() { return 0xFFFFFFFF; }
  static constexpr uint32_t BLACK() { return 0xFF000000; }
  static constexpr TTColor UNDEFINED() { return {}; }

 public:
  constexpr TTColor() : value_(0) {}
  TTColor(const TTColor& color) : value_(color.value_) {}
  explicit constexpr TTColor(uint32_t plain_color) : value_(plain_color) {}
  TTColor(uint8_t theme_color_id, uint8_t alpha) = delete;
  ~TTColor() = default;

 public:
  bool operator==(const TTColor& other) const { return value_ == other.value_; }
  bool operator!=(const TTColor& other) const { return !(*this == other); }
  void operator=(const uint32_t& color) { SetColor(color); }
  operator uint32_t() const { return value_; }

 public:
  void SetARGB(uint32_t color) { value_ = color; }
  void SetThemeIdAndAlpha(uint8_t theme_id, uint8_t alpha) = delete;

 public:
  bool IsThemeColor() const { return false; }
  bool IsUndefined() const { return false; }
  uint32_t GetPlainColor() const { return value_; }
  uint8_t GetAlpha() const { return value_ >> 24; }
  uint8_t GetRed() const { return (value_ >> 16) & 0xff; }
  uint8_t GetGreen() const { return (value_ >> 8) & 0xff; }
  uint8_t GetBlue() const { return (value_) & 0xff; }
  uint8_t GetThemeColorId() const = delete;
  std::string ToHexString() const;

 public:
  static TTColor ReadHexString(const char* argb_string);
  void SetColor(uint32_t color) { value_ = color; }

 public:
  uint32_t value_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_TT_COLOR_H_
