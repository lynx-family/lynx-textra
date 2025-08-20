// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_TT_POINT_H_
#define SRC_TEXTLAYOUT_UTILS_TT_POINT_H_
#include <cmath>

namespace ttoffice {
namespace tttext {
class PointF {
  // public:
  //  TTPoint() : x_(0), y_(0){};
  //  explicit TTPoint(float x, float y) : x_(x), y_(y){};
  //  ~TTPoint() = default;

 public:
  inline void SetX(float x) { x_ = x; }
  inline float GetX() const { return x_; }
  inline void SetY(float y) { y_ = y; }
  inline float GetY() const { return y_; }
  inline void Translate(float x, float y) {
    x_ += x;
    y_ += y;
  }
  inline void Translate(const PointF& point) {
    x_ += point.x_;
    y_ += point.y_;
  }

 public:
  PointF operator+(const PointF& other) const {
    return {x_ + other.x_, y_ + other.y_};
  }
  void operator+=(const PointF& other) {
    x_ += other.x_;
    y_ += other.y_;
  }
  PointF operator-(const PointF& other) const {
    return {x_ - other.x_, y_ - other.y_};
  }
  void operator-=(const PointF& other) {
    x_ -= other.x_;
    y_ -= other.y_;
  }
  PointF operator*(float n) const { return {x_ * n, y_ * n}; }
  void operator*=(float n) {
    x_ *= n;
    y_ *= n;
  }
  PointF operator/(float n) const { return {x_ / n, y_ / n}; }
  void operator/=(float n) {
    x_ /= n;
    y_ /= n;
  }
  float LengthToZero() const { return std::sqrt(x_ * x_ + y_ * y_); }

 public:
  bool operator==(const PointF& point) const;
  bool operator!=(const PointF& point) { return !operator==(point); }

 public:
  float x_{0};
  float y_{0};
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_UTILS_TT_POINT_H_
