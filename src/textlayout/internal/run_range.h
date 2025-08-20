// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_INTERNAL_RUN_RANGE_H_
#define SRC_TEXTLAYOUT_INTERNAL_RUN_RANGE_H_

#include <textra/layout_definition.h>
#include <textra/macro.h>

#include <memory>

#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/run/base_run.h"
#include "src/textlayout/utils/float_comparison.h"

namespace ttoffice {
namespace tttext {
class ParagraphImpl;
class LineRange;
class LayoutMeasurer;

class RunRange {
  friend LineRange;
  friend LayoutMeasurer;
  friend class TextLine;

 public:
  RunRange() = default;
  RunRange(const BaseRun* run, const LineRange* parent, uint32_t start_char,
           uint32_t end_char)
      : run_(run),
        parent_(parent),
        start_char_(start_char),
        end_char_(end_char) {}

 public:
  CharPos GetStartCharPosInParagraph() const {
    return start_char_ + run_->GetStartCharPos();
  }
  CharPos GetEndCharPosInParagraph() const {
    return end_char_ + run_->GetStartCharPos();
  }
  uint32_t GetCharCount() const {
    return GetEndCharPosInParagraph() - GetStartCharPosInParagraph();
  }
  float GetWidth() const {
    // When calculating the total width, the width of the hyphen must be
    // included; otherwise, in justified or right alignment, the hyphen may
    // overflow.
    return run_->GetWidth(start_char_, end_char_ - start_char_);
    //    return width + (ghost_content_ == nullptr
    //                        ? 0
    //                        : (ghost_content_->GetWidth(0, nullptr)));
  }
  float GetWidthWithIndent() const { return GetWidth(); }
  float GetWidthInRange(uint32_t char_start_in_range,
                        uint32_t char_count) const {
    TTASSERT(start_char_ + char_start_in_range + char_count <= end_char_);
    return run_->GetWidth(start_char_ + char_start_in_range, char_count);
  }
  float GetLeft() const { return GetXOffset(); }
  float GetRight() const { return GetXOffset() + GetWidth(); }
  float GetYOffsetInLine() const { return y_offset_in_line_; }
  void SetYOffsetInLine(float y_offset_in_line) {
    y_offset_in_line_ = y_offset_in_line;
  }
  /*
   * Get the character position relative to the paragraph.
   * @param x: The x offset relative to the drawer piece.
   * @return pos: The character position in the paragraph at the given x
   * position.
   */
  CharPos FindCharPosInParagraphByX(float x) const {
    if (FloatsLargerOrEqual(0, x)) return GetStartCharPosInParagraph();
    uint32_t k = 1;
    for (; k < GetCharCount(); k++) {
      if (FloatsLarger(GetWidthInRange(0, k), x)) break;
    }
    return k + GetStartCharPosInParagraph() - 1;
  }
  float GetXOffset() const { return x_offset_; }
  void SetXOffset(float x_offset) { x_offset_ = x_offset; }
  // float GetStartIndent() const { return start_indent_; }
  // void SetStartIndent(float start_indent) { start_indent_ = start_indent; };
  // float GetEndIndent() const { return end_indent_; };
  // void SetEndIndent(float end_indent) { end_indent_ = end_indent; };
  const BaseRun* GetRun() const { return run_; }
  //  bool Visible() const { return run_->Visible(); }
  uint32_t GetVisualOrderIndex() const { return run_->GetVisualOrderIndex(); }
  RunType GetType() const { return run_->GetType(); }
  const LayoutMetrics& GetLayoutMetrics() const { return metrics_; }
  void AddHyphnateRun(std::unique_ptr<BaseRun> hyphnate_run) {
    //    ghost_content_ = std::move(hyphnate_run);
  }
  const BaseRun* GetHyphnateRun() const {
    TTASSERT(false);
    return nullptr;
    //    return ghost_content_.get();
  }
  const LineRange* GetParent() const { return parent_; }

 private:
  const BaseRun* run_{};
  const LineRange* parent_{nullptr};
  uint32_t start_char_{};  // offset relative BaseRun
  uint32_t end_char_{};
  float x_offset_{};
  float y_offset_in_line_{};
  // float start_indent_{};
  // float end_indent_{};
  LayoutMetrics metrics_;
  //  std::unique_ptr<BaseRun> ghost_content_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_INTERNAL_RUN_RANGE_H_
