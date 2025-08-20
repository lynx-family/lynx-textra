// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_INTERNAL_LINE_RANGE_H_
#define SRC_TEXTLAYOUT_INTERNAL_LINE_RANGE_H_

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "src/textlayout/internal/run_range.h"
namespace ttoffice {
namespace tttext {
enum class CharacterVerticalAlignment : uint8_t;
class BaseRun;
class LayoutMeasurer;
class RunRange;
class TextLineImpl;
class BlockRegion;
class ParagraphImpl;
class LayoutDrawer;
class LineRange {
  friend LayoutMeasurer;
  friend TextLineImpl;
  friend LayoutDrawer;

 public:
  explicit LineRange(float x_min, float x_max)
      : x_min_(x_min), x_current_(x_min), x_max_(x_max) {
    run_range_lst_.clear();
  }
  ~LineRange() = default;

 public:
  float GetAvailableWidth() const { return x_max_ - x_current_; }
  void AddWordRange(std::unique_ptr<RunRange> run_range) {
    x_current_ += run_range->GetWidthWithIndent();
    run_range_lst_.emplace_back(std::move(run_range));
  }
  float GetRangeWidth() const { return x_max_ - x_min_; }
  void SetXMin(float x) { x_min_ = x; }
  float GetXMin() const { return x_min_; }
  void SetXMax(float x) { x_max_ = x; }
  float GetXMax() const { return x_max_; }
  float GetContentLeft() const { return x_min_; }
  float GetContentRight() const { return GetContentLeft() + GetContentWidth(); }
  void Clear() { run_range_lst_.clear(); }
  bool Empty() const { return run_range_lst_.empty(); }

 private:
  float GetContentWidth() const { return x_current_ - x_min_; }

 private:
  float x_min_ = 0;
  float x_current_ = 0;
  float x_max_ = 0;
  std::vector<std::unique_ptr<RunRange>> run_range_lst_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_INTERNAL_LINE_RANGE_H_
