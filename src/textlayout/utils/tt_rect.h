// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_TT_RECT_H_
#define SRC_TEXTLAYOUT_UTILS_TT_RECT_H_

#include <textra/macro.h>

#include "src/textlayout/utils/tt_point.h"

namespace ttoffice {
namespace tttext {
class Rect {
  using RectType = uint32_t;

 public:
  Rect() : Rect(0, 0, 0, 0) {}
  explicit Rect(RectType left, RectType top, RectType width, RectType height)
      : x_(left), y_(top), width_(width), height_(height) {}
  ~Rect() = default;

 public:
  static Rect MakeEmpty() { return {}; }
  static Rect MakeLTRB(RectType left, RectType top, RectType right,
                       RectType bottom) {
    return Rect(left, top, right - left, bottom - top);
  }
  static Rect MakeLTWH(RectType left, RectType top, RectType width,
                       RectType height) {
    return Rect(left, top, width, height);
  }
  static Rect MakeWH(RectType width, RectType height) {
    return Rect(0, 0, width, height);
  }

 public:
  void SetLeft(RectType left) { x_ = left; }
  void SetTop(RectType top) { y_ = top; }
  void SetRight(RectType right) { width_ = right - x_; }
  void SetBottom(RectType bottom) { height_ = bottom - y_; }
  void SetWidth(RectType width) { width_ = width; }
  void SetHeight(RectType height) { height_ = height; }
  RectType GetLeft() const { return x_; }
  RectType GetTop() const { return y_; }
  RectType GetRight() const { return x_ + width_; }
  RectType GetBottom() const { return y_ + height_; }
  RectType GetWidth() const { return width_; }
  RectType GetHeight() const { return height_; }
  void Offset(RectType x, RectType y) {
    x_ += x;
    y_ += y;
  }
  void OffsetTo(RectType abs_x, RectType abs_y) {
    x_ = abs_x;
    y_ = abs_y;
  }
  bool Contains(RectType x, RectType y) const {
    return x >= GetLeft() && GetRight() > x && y >= GetTop() && GetBottom() > y;
  }
  bool InterSects(RectType left, RectType top, RectType right,
                  RectType bottom) const {
    return !(GetLeft() > right || GetTop() > bottom || left > GetRight() ||
             top > GetBottom());
  }
  bool InterSects(const Rect& rect) const {
    return InterSects(rect.GetLeft(), rect.GetTop(), rect.GetRight(),
                      rect.GetBottom());
  }
  bool IsEmpty() const { return GetWidth() == 0 || GetHeight() == 0; }

 public:
  bool operator==(const Rect& rect) {
    return x_ == rect.x_ && y_ == rect.y_ && width_ == rect.width_ &&
           height_ == rect.height_;
  }
  bool operator!=(const Rect& rect) { return !operator==(rect); }
  void operator=(const Rect& rect) {
    x_ = rect.x_;
    y_ = rect.y_;
    width_ = rect.width_;
    height_ = rect.height_;
  }

 private:
  RectType x_;
  RectType y_;
  RectType width_;
  RectType height_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_UTILS_TT_RECT_H_
