// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_STYLE_PARAGRAPH_STYLE_IMPL_H_
#define SRC_TEXTLAYOUT_STYLE_PARAGRAPH_STYLE_IMPL_H_

#include <textra/layout_definition.h>
#include <textra/style.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>

#include "src/textlayout/style_attributes.h"

namespace ttoffice {
namespace tttext {
class RunDelegate;

class ParagraphStyleImpl {
 public:
  ParagraphStyleImpl();
  ParagraphStyleImpl(const ParagraphStyleImpl& paragraph_style);
  ParagraphStyleImpl& operator=(const ParagraphStyleImpl& paragraph_style);
  ~ParagraphStyleImpl();

  const Style& GetDefaultStyle() const { return default_style_; }
  const StyleImpl& GetDefaultStyleImpl() const {
    return default_style_.GetImpl();
  }
  void SetDefaultStyle(const Style& default_style) {
    default_style_ = default_style;
  }
  ParagraphHorizontalAlignment GetHorizontalAlign() const {
    return horizontal_alignment_;
  }
  void SetHorizontalAlign(ParagraphHorizontalAlignment align) {
    horizontal_alignment_ = align;
  }
  ParagraphVerticalAlignment GetVerticalAlign() const {
    return vertical_alignment_;
  }
  void SetVerticalAlign(ParagraphVerticalAlignment vertical_align) {
    vertical_alignment_ = vertical_align;
  }
  DominantBaseline GetDominantBaseline() const { return dominant_baseline_; }
  void SetDominantBaseline(DominantBaseline dominant_baseline) {
    dominant_baseline_ = dominant_baseline;
  }
  float GetStartIndentInPx() const { return indent_->start_; }
  void SetStartIndentInPx(float px) {
    indent_->start_ = px;
    indent_->start_chars_ = 0;
  }
  int32_t GetStartIndentInCharCnt() const { return indent_->start_chars_; }
  void SetStartIndentInCharCnt(int32_t char_count) {
    indent_->start_chars_ = char_count;
    indent_->start_ = 0.0f;
  }
  float GetEndIndentInPx() const { return indent_->end_; }
  void SetEndIndentInPx(float px) {
    indent_->end_ = px;
    indent_->end_chars_ = 0;
  }
  int32_t GetEndIndentInCharCnt() const { return indent_->end_chars_; }
  void SetEndIndentInCharCnt(int32_t char_count) {
    indent_->end_chars_ = char_count;
    indent_->end_ = 0.0f;
  }
  float GetFirstLineIndentInPx() const { return indent_->first_line_; }
  void SetFirstLineIndentInPx(float px) {
    indent_->first_line_ = px;
    indent_->first_line_chars_ = 0;
  }
  int32_t GetFirstLineIndentInCharCnt() const {
    return indent_->first_line_chars_;
  }
  void SetFirstLineIndentInCharCnt(int32_t char_count) {
    indent_->first_line_chars_ = char_count;
    indent_->first_line_ = 0.0f;
  }
  float GetHangingIndentInPx() const { return indent_->hanging_; }
  void SetHangingIndentInPx(float px) {
    indent_->hanging_ = px;
    indent_->hanging_chars_ = 0;
  }
  int32_t GetHangingIndentInCharCnt() const { return indent_->hanging_chars_; }
  void SetHangingIndentInCharCnt(int32_t char_count) {
    indent_->hanging_chars_ = char_count;
    indent_->hanging_ = 0.0f;
  }
  void SetWriteDirection(WriteDirection dir) { write_direction_ = dir; }
  WriteDirection GetWriteDirection() const { return write_direction_; }
  float GetSpacingAfterInPx() const { return spacing_->after_px_; }
  void SetSpacingAfterInPx(float px) { spacing_->after_px_ = px; }
  float GetSpacingBeforeInPx() const { return spacing_->before_px_; }
  void SetSpacingBeforeInPx(float px) { spacing_->before_px_ = px; }
  float GetLineHeightInPx() const { return spacing_->line_px_; }
  void SetLineHeightInPx(float line_height, RulerType rt) {
    spacing_->line_px_ = line_height;
    spacing_->line_rule_ = rt;
  }
  void SetLineHeightInPxExact(float px) {
    spacing_->line_px_ = px;
    spacing_->line_rule_ = RulerType::kExact;
  }
  void SetLineHeightInPxAtLeast(float px) {
    spacing_->line_px_ = px;
    spacing_->line_rule_ = RulerType::kAtLeast;
  }
  float GetLineHeightInPercent() const { return spacing_->line_percent_; }
  void SetLineHeightInPercent(float percent) {
    spacing_->line_percent_ = percent;
  }
  RulerType GetLineHeightRule() const { return spacing_->line_rule_; }
  void SetInlineVerticalAlignmentMode(InlineVerticalAlignmentMode mode) {
    inline_vertical_alignment_mode_ = mode;
  }
  InlineVerticalAlignmentMode GetInlineVerticalAlignmentMode() const {
    return inline_vertical_alignment_mode_;
  }
  void SetLineSpaceBeforePx(float px) { spacing_->line_space_before_px_ = px; }
  float GetLineSpaceBeforePx() const { return spacing_->line_space_before_px_; }
  void SetLineSpaceAfterPx(float px) { spacing_->line_space_after_px_ = px; }
  float GetLineSpaceAfterPx() const { return spacing_->line_space_after_px_; }
  const Spacing& GetSpacing() const { return *spacing_; }
  void SetSpacing(const Spacing& spacing) { *spacing_ = spacing; }
  void SetEllipsis(const std::u16string& ellipsis);
  void SetEllipsis(const char* ellipsis);
  void SetEllipsis(std::shared_ptr<RunDelegate> ellipsis) {
    ellipsis_delegate_ = std::move(ellipsis);
    ellipsis_.clear();
  }
  const std::u32string& GetEllipsis() const { return ellipsis_; }
  const std::shared_ptr<RunDelegate>& GetEllipsisDelegate() const {
    return ellipsis_delegate_;
  }
  void SetEllipsisUsesDefaultForeground(bool enabled) {
    ellipsis_uses_default_foreground_ = enabled;
  }
  bool EllipsisUsesDefaultForeground() const {
    return ellipsis_uses_default_foreground_;
  }
  uint32_t GetMaxLines() const { return max_lines_; }
  void SetMaxLines(uint32_t max_line) {
    max_lines_ =
        max_line == 0 ? std::numeric_limits<uint32_t>::max() : max_line;
  }
  bool LineHeightOverride() const { return line_height_override_; }
  void SetLineHeightOverride(bool line_height_override) {
    line_height_override_ = line_height_override;
  }
  bool HalfLeading() const { return half_leading_; }
  void SetHalfLeading(bool half) { half_leading_ = half; }
  void EnableTextBounds(bool align_with_bbox) {
    enable_text_bounds_ = align_with_bbox;
  }
  bool EnableTextBounds() const { return enable_text_bounds_; }
  OverflowWrap GetOverflowWrap() const { return overflow_wrap_; }
  void SetOverflowWrap(OverflowWrap overflow_wrap) {
    overflow_wrap_ = overflow_wrap;
  }
  void AllowBreakAroundPunctuation(bool allow) {
    if (!allow) {
      line_break_strategy_ = static_cast<LineBreakStrategy>(
          line_break_strategy_ | kAvoidBreakAroundPunctuation);
    } else {
      line_break_strategy_ = static_cast<LineBreakStrategy>(
          line_break_strategy_ & (0xff ^ kAvoidBreakAroundPunctuation));
    }
  }

 public:
  ParagraphHorizontalAlignment horizontal_alignment_;
  ParagraphVerticalAlignment vertical_alignment_;
  DominantBaseline dominant_baseline_;
  Style default_style_;
  std::unique_ptr<Indent> indent_;
  std::unique_ptr<Spacing> spacing_;

  WriteDirection write_direction_;
  std::u32string ellipsis_;
  std::shared_ptr<RunDelegate> ellipsis_delegate_;
  bool ellipsis_uses_default_foreground_{false};
  uint32_t max_lines_;
  bool line_height_override_;
  bool half_leading_;
  bool enable_text_bounds_;
  InlineVerticalAlignmentMode inline_vertical_alignment_mode_;
  OverflowWrap overflow_wrap_;
  LineBreakStrategy line_break_strategy_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_STYLE_PARAGRAPH_STYLE_IMPL_H_
