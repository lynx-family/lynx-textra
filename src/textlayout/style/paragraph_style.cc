// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/paragraph.h>
#include <textra/paragraph_style.h>
#include <textra/run_delegate.h>

#include <memory>

#include "src/textlayout/style_attributes.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
ParagraphStyle::ParagraphStyle()
    : horizontal_alignment_(ParagraphHorizontalAlignment::kLeft),
      vertical_alignment_(ParagraphVerticalAlignment::kCenter),
      default_style_(Style::DefaultStyle()),
      indent_(std::make_unique<Indent>()),
      spacing_(std::make_unique<Spacing>()),

      write_direction_(WriteDirection::kAuto),
      ellipsis_(U""),
      max_lines_(std::numeric_limits<uint32_t>::max()),
      line_height_override_(false),
      half_leading_(false),
      enable_text_bounds_(false),
      overflow_wrap_(OverflowWrap::kAnywhere),
      line_break_strategy_(kLineBreakStrategyDefault) {}
ParagraphStyle::ParagraphStyle(const ParagraphStyle& paragraph_style)
    : ParagraphStyle() {
  *this = paragraph_style;
}
ParagraphStyle::~ParagraphStyle() = default;
ParagraphStyle& ParagraphStyle::operator=(
    const ParagraphStyle& paragraph_style) {
  if (this == &paragraph_style) return *this;
  horizontal_alignment_ = paragraph_style.horizontal_alignment_;
  vertical_alignment_ = paragraph_style.vertical_alignment_;
  default_style_ = paragraph_style.default_style_;
  *indent_ = *paragraph_style.indent_;
  *spacing_ = *paragraph_style.spacing_;

  write_direction_ = paragraph_style.write_direction_;
  ellipsis_ = paragraph_style.ellipsis_;
  ellipsis_delegate_ = paragraph_style.ellipsis_delegate_;
  max_lines_ = paragraph_style.max_lines_;
  line_height_override_ = paragraph_style.line_height_override_;
  half_leading_ = paragraph_style.half_leading_;
  enable_text_bounds_ = paragraph_style.enable_text_bounds_;
  overflow_wrap_ = paragraph_style.overflow_wrap_;
  line_break_strategy_ = paragraph_style.line_break_strategy_;
  return *this;
}
float ParagraphStyle::GetStartIndentInPx() const { return indent_->start_; }
void ParagraphStyle::SetStartIndentInPx(float px) {
  indent_->start_ = px;
  indent_->start_chars_ = 0;
}
int32_t ParagraphStyle::GetStartIndentInCharCnt() const {
  return indent_->start_chars_;
}
void ParagraphStyle::SetStartIndentInCharCnt(int32_t char_count) {
  indent_->start_chars_ = char_count;
  indent_->start_ = 0.0f;
}
float ParagraphStyle::GetEndIndentInPx() const { return indent_->end_; }
void ParagraphStyle::SetEndIndentInPx(float px) {
  indent_->end_ = px;
  indent_->end_chars_ = 0;
}
int32_t ParagraphStyle::GetEndIndentInCharCnt() const {
  return indent_->end_chars_;
}
void ParagraphStyle::SetEndIndentInCharCnt(int32_t char_count) {
  indent_->end_chars_ = char_count;
  indent_->end_ = 0.0f;
}
float ParagraphStyle::GetFirstLineIndentInPx() const {
  return indent_->first_line_;
}
void ParagraphStyle::SetFirstLineIndentInPx(float px) {
  indent_->first_line_ = px;
  indent_->first_line_chars_ = 0;
}
int32_t ParagraphStyle::GetFirstLineIndentInCharCnt() const {
  return indent_->first_line_chars_;
}
void ParagraphStyle::SetFirstLineIndentInCharCnt(int32_t char_count) {
  indent_->first_line_chars_ = char_count;
  indent_->first_line_ = 0.0f;
}
float ParagraphStyle::GetHangingIndentInPx() const { return indent_->hanging_; }
void ParagraphStyle::SetHangingIndentInPx(float px) {
  indent_->hanging_ = px;
  indent_->hanging_chars_ = 0;
}
int32_t ParagraphStyle::GetHangingIndentInCharCnt() const {
  return indent_->hanging_chars_;
}
void ParagraphStyle::SetHangingIndentInCharCnt(int32_t char_count) {
  indent_->hanging_chars_ = char_count;
  indent_->hanging_ = 0.0f;
}
float ParagraphStyle::GetSpacingAfterInPx() const {
  return spacing_->after_px_;
}
void ParagraphStyle::SetSpacingAfterInPx(float px) { spacing_->after_px_ = px; }
float ParagraphStyle::GetSpacingBeforeInPx() const {
  return spacing_->before_px_;
}
void ParagraphStyle::SetSpacingBeforeInPx(float px) {
  spacing_->before_px_ = px;
}
float ParagraphStyle::GetLineHeightInPx() const { return spacing_->line_px_; }
void ParagraphStyle::SetLineHeightInPx(float line_height, RulerType rt) {
  spacing_->line_px_ = line_height;
  spacing_->line_rule_ = rt;
}
void ParagraphStyle::SetLineHeightInPxExact(float px) {
  spacing_->line_px_ = px;
  spacing_->line_rule_ = RulerType::kExact;
}
void ParagraphStyle::SetLineHeightInPxAtLeast(float px) {
  spacing_->line_px_ = px;
  spacing_->line_rule_ = RulerType::kAtLeast;
}
float ParagraphStyle::GetLineHeightInPercent() const {
  return spacing_->line_percent_;
}
void ParagraphStyle::SetLineHeightInPercent(float percent) {
  spacing_->line_percent_ = percent;
}
RulerType ParagraphStyle::GetLineHeightRule() const {
  return spacing_->line_rule_;
}
void ParagraphStyle::SetLineSpaceBeforePx(float px) {
  spacing_->line_space_before_px_ = px;
}
float ParagraphStyle::GetLineSpaceBeforePx() const {
  return spacing_->line_space_before_px_;
}
void ParagraphStyle::SetLineSpaceAfterPx(float px) {
  spacing_->line_space_after_px_ = px;
}
float ParagraphStyle::GetLineSpaceAfterPx() const {
  return spacing_->line_space_after_px_;
}
const Spacing& ParagraphStyle::GetSpacing() const { return *spacing_; }
void ParagraphStyle::SetSpacing(const Spacing& spacing) { *spacing_ = spacing; }
void ParagraphStyle::SetEllipsis(const std::u16string& ellipsis) {
  ellipsis_ = base::U16StringToU32(ellipsis.data(),
                                   static_cast<uint32_t>(ellipsis.size()));
  ellipsis_delegate_ = nullptr;
}
void ParagraphStyle::SetEllipsis(const char* ellipsis) {
  ellipsis_ =
      base::U8StringToU32(ellipsis, static_cast<uint32_t>(strlen(ellipsis)));
  ellipsis_delegate_ = nullptr;
}
void ParagraphStyle::SetEllipsis(std::shared_ptr<RunDelegate> ellipsis) {
  ellipsis_delegate_ = std::move(ellipsis);
  ellipsis_.clear();
}
void ParagraphStyle::AllowBreakAroundPunctuation(bool allow) {
  if (!allow) {
    line_break_strategy_ = static_cast<LineBreakStrategy>(
        line_break_strategy_ | kAvoidBreakAroundPunctuation);
  } else {
    line_break_strategy_ = static_cast<LineBreakStrategy>(
        line_break_strategy_ & (0xff ^ kAvoidBreakAroundPunctuation));
  }
}

}  // namespace tttext
}  // namespace ttoffice
