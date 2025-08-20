// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_LAYOUT_POSITION_H_
#define SRC_TEXTLAYOUT_LAYOUT_POSITION_H_

#include <textra/macro.h>

#include <limits>
#include <numeric>

namespace ttoffice {
namespace tttext {
/**
 * @brief Represents a layout position, identified by a run index and a
 * character index.
 *
 * Provides utility functions for updating and comparing positions.
 *
 * Used in various scenarios, including:
 * - Tracking the current position during layout
 * - Representing the start and end positions of text lines
 * - Comparing positions for text range operations
 */
class L_EXPORT LayoutPosition {
 public:
  LayoutPosition(uint32_t run_idx, uint32_t char_idx)
      : run_idx_(run_idx), char_idx_(char_idx) {}

 public:
  uint32_t GetRunIdx() const { return run_idx_; }
  void SetRunIdx(uint32_t run_idx) { run_idx_ = run_idx; }
  uint32_t GetCharIdx() const { return char_idx_; }
  void SetCharIdx(uint32_t char_idx) { char_idx_ = char_idx; }
  const LayoutPosition& PrevRun(uint32_t run_count = 1) {
    if (run_idx_ >= 0 && run_idx_ - run_count >= 0) {
      run_idx_ -= run_count;
      char_idx_ = 0;
    }
    return *this;
  }
  const LayoutPosition& NextRun(uint32_t run_count = 1) {
    if (std::numeric_limits<uint32_t>::max() - run_idx_ >= run_count) {
      run_idx_ += run_count;
      char_idx_ = 0;
    }
    return *this;
  }
  const LayoutPosition& NextChar(uint32_t char_count = 1) {
    char_idx_ += char_count;
    return *this;
  }
  void Update(const LayoutPosition& pos) { *this = pos; }

 public:
  bool operator<(const LayoutPosition& rhs) const {
    return run_idx_ < rhs.run_idx_ ||
           (run_idx_ == rhs.run_idx_ && char_idx_ < rhs.char_idx_);
  }
  bool operator>(const LayoutPosition& rhs) const {
    return run_idx_ > rhs.run_idx_ ||
           (run_idx_ == rhs.run_idx_ && char_idx_ > rhs.char_idx_);
  }
  bool operator==(const LayoutPosition& rhs) const {
    return run_idx_ == rhs.run_idx_ && char_idx_ == rhs.char_idx_;
  }
  bool operator<=(const LayoutPosition& rhs) const { return !operator>(rhs); }
  bool operator>=(const LayoutPosition& rhs) const { return !operator<(rhs); }
  bool operator!=(const LayoutPosition& rhs) const { return !operator==(rhs); }

 private:
  uint32_t run_idx_;
  uint32_t char_idx_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_LAYOUT_POSITION_H_
