// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_TT_RECTF_H_
#define SRC_TEXTLAYOUT_UTILS_TT_RECTF_H_

#include <array>

#include "src/textlayout/utils/float_comparison.h"
#include "src/textlayout/utils/tt_rect.h"

namespace ttoffice {
namespace tttext {
class L_EXPORT RectF {
 public:
  RectF() : RectF(0, 0, 0, 0) {}
  RectF(float left, float top, float width, float height)
      : x_(left), y_(top), width_(width), height_(height) {}
  ~RectF() = default;

 public:
  static RectF MakeEmpty() { return {}; }
  static RectF MakeLTRB(float left, float top, float right, float bottom) {
    return RectF(left, top, right - left, bottom - top);
  }
  static RectF MakeLTWH(float left, float top, float width, float height) {
    return RectF(left, top, width, height);
  }
  static RectF MakeWH(float width, float height) {
    return RectF(0, 0, width, height);
  }

 public:
  void SetLeft(float left) { x_ = left; }
  void SetTop(float top) { y_ = top; }
  void SetRight(float right) { width_ = right - x_; }
  void SetBottom(float bottom) { height_ = bottom - y_; }
  void SetWidth(float width) { width_ = width; }
  void SetHeight(float height) { height_ = height; }
  float GetLeft() const { return x_; }
  float GetTop() const { return y_; }
  float GetRight() const { return x_ + width_; }
  float GetBottom() const { return y_ + height_; }
  float GetWidth() const { return width_; }
  float GetHeight() const { return height_; }
  void Offset(float x, float y) {
    x_ += x;
    y_ += y;
  }
  void OffsetTo(float abs_x, float abs_y) {
    x_ = abs_x;
    y_ = abs_y;
  }
  bool Contains(float x, float y) const;
  bool InterSects(float left, float top, float right, float bottom) const;
  bool InterSects(const RectF& rect) const;
  bool IsEmpty() const;
  bool operator==(const RectF& rect) const;
  bool operator!=(const RectF& rect) const { return !operator==(rect); }
  void operator=(const RectF& rect) {
    x_ = rect.x_;
    y_ = rect.y_;
    width_ = rect.width_;
    height_ = rect.height_;
  }
  std::array<float, 4> ToArrayLTWH() const {
    return {GetLeft(), GetTop(), GetWidth(), GetHeight()};
  }

 private:
  float x_;
  float y_;
  float width_;
  float height_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_UTILS_TT_RECTF_H_
