// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_TT_SIZE_H_
#define SRC_TEXTLAYOUT_UTILS_TT_SIZE_H_

namespace ttoffice {
namespace tttext {
class TTSize {
 public:
  TTSize() : width_(0), height_(0) {}
  TTSize(float w, float h) : width_(w), height_(h) {}
  ~TTSize() = default;

 public:
  void SetWidth(float w) { width_ = w; }
  float Width() const { return width_; }
  void SetHeight(float h) { height_ = h; }
  float Height() const { return height_; }

 public:
  bool operator==(const TTSize& other) {
    return width_ == other.width_ && height_ == other.height_;
  }
  bool operator!=(const TTSize& other) { return !operator==(other); }
  void operator=(const TTSize& other) {
    width_ = other.width_;
    height_ = other.height_;
  }

 private:
  float width_;
  float height_;
};
}  // namespace tttext
}  // namespace ttoffice
using TTSize = ::ttoffice::tttext::TTSize;
#endif  // SRC_TEXTLAYOUT_UTILS_TT_SIZE_H_
