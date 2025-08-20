// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_RUN_LAYOUT_METRICS_H_
#define SRC_TEXTLAYOUT_RUN_LAYOUT_METRICS_H_

#include <algorithm>

#include "src/textlayout/utils/float_comparison.h"
namespace ttoffice {
namespace tttext {
class BaseRun;
class LayoutMeasurer;
class LayoutMetrics {
  friend BaseRun;
  friend LayoutMeasurer;

 public:
  LayoutMetrics() = default;
  explicit LayoutMetrics(float ascent, float descent)
      : max_ascent_(ascent), max_descent_(descent) {}

 public:
  float GetMaxAscent() const { return max_ascent_; }
  float GetMaxDescent() const { return max_descent_; }
  void UpdateMaxAscent(float ascent) {
    max_ascent_ = std::min(max_ascent_, ascent);
  }
  void UpdateMaxDescent(float descent) {
    max_descent_ = std::max(max_descent_, descent);
  }
  void UpdateMax(const LayoutMetrics& metrics) {
    UpdateMaxAscent(metrics.GetMaxAscent());
    UpdateMaxDescent(metrics.GetMaxDescent());
  }
  void UpdateMax(const FontInfo& font_info) {
    UpdateMaxAscent(font_info.GetAscent());
    UpdateMaxDescent(font_info.GetDescent());
  }
  float GetHeight() const { return GetMaxDescent() - GetMaxAscent(); }
  void Reset() {
    max_ascent_ = 0;
    max_descent_ = 0;
  }
  void ApplyBaselineOffset(float baseline_offset) {
    if (FloatsLarger(baseline_offset, 0)) {
      max_descent_ += baseline_offset;
    } else {
      max_ascent_ += baseline_offset;
    }
  }
  void ApplyDesiredHeight(float desired_height) {
    if (FloatsEqual(GetHeight(), 0)) return;
    // // in office word, if the height of a line is larger than text, word
    // layout align the text top with line top if (FloatsLarger(desired_height,
    // GetHeight())) {
    //   auto delta = (desired_height - GetHeight()) / 2;
    //   max_ascent_ -= delta;
    //   max_descent_ += delta;
    // } else {
    auto ratio = desired_height / GetHeight();
    max_ascent_ *= ratio;
    max_descent_ *= ratio;
    // }
  }

 private:
  float max_ascent_ = 0;
  float max_descent_ = 0;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_RUN_LAYOUT_METRICS_H_
