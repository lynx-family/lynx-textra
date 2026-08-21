// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/style/paragraph_style_impl.h"

#include <textra/paragraph.h>
#include <textra/paragraph_style.h>
#include <textra/run_delegate.h>

#include <cstring>
#include <limits>
#include <memory>
#include <utility>

#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
ParagraphStyleImpl::ParagraphStyleImpl()
    : horizontal_alignment_(ParagraphHorizontalAlignment::kLeft),
      vertical_alignment_(ParagraphVerticalAlignment::kCenter),
      dominant_baseline_(DominantBaseline::kAlphabetic),
      default_style_(Style::DefaultStyle()),
      indent_(std::make_unique<Indent>()),
      spacing_(std::make_unique<Spacing>()),

      write_direction_(WriteDirection::kAuto),
      ellipsis_(U""),
      max_lines_(std::numeric_limits<uint32_t>::max()),
      line_height_override_(false),
      half_leading_(false),
      enable_text_bounds_(false),
      inline_vertical_alignment_mode_(InlineVerticalAlignmentMode::kLegacy),
      overflow_wrap_(OverflowWrap::kAnywhere),
      line_break_strategy_(kLineBreakStrategyDefault) {}
ParagraphStyleImpl::ParagraphStyleImpl(
    const ParagraphStyleImpl& paragraph_style)
    : ParagraphStyleImpl() {
  *this = paragraph_style;
}
ParagraphStyleImpl& ParagraphStyleImpl::operator=(
    const ParagraphStyleImpl& paragraph_style) {
  if (this == &paragraph_style) return *this;
  horizontal_alignment_ = paragraph_style.horizontal_alignment_;
  vertical_alignment_ = paragraph_style.vertical_alignment_;
  dominant_baseline_ = paragraph_style.dominant_baseline_;
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
  inline_vertical_alignment_mode_ =
      paragraph_style.inline_vertical_alignment_mode_;
  overflow_wrap_ = paragraph_style.overflow_wrap_;
  line_break_strategy_ = paragraph_style.line_break_strategy_;
  return *this;
}
ParagraphStyleImpl::~ParagraphStyleImpl() = default;

void ParagraphStyleImpl::SetEllipsis(const std::u16string& ellipsis) {
  ellipsis_ = base::U16StringToU32(ellipsis.data(),
                                   static_cast<uint32_t>(ellipsis.size()));
  ellipsis_delegate_ = nullptr;
}
void ParagraphStyleImpl::SetEllipsis(const char* ellipsis) {
  ellipsis_ =
      base::U8StringToU32(ellipsis, static_cast<uint32_t>(strlen(ellipsis)));
  ellipsis_delegate_ = nullptr;
}

ParagraphStyle::ParagraphStyle()
    : owned_impl_(std::make_unique<ParagraphStyleImpl>()),
      impl_(owned_impl_.get()) {}
ParagraphStyle::ParagraphStyle(ParagraphStyleImpl* impl) : impl_(impl) {}
ParagraphStyle::ParagraphStyle(const ParagraphStyle& paragraph_style)
    : ParagraphStyle() {
  *this = paragraph_style;
}
ParagraphStyle::~ParagraphStyle() = default;
ParagraphStyle& ParagraphStyle::operator=(
    const ParagraphStyle& paragraph_style) {
  if (this == &paragraph_style) return *this;
  *impl_ = paragraph_style.GetImpl();
  return *this;
}
const Style& ParagraphStyle::GetDefaultStyle() const {
  return impl_->GetDefaultStyle();
}
void ParagraphStyle::SetDefaultStyle(const Style& default_style) {
  impl_->SetDefaultStyle(default_style);
}
ParagraphHorizontalAlignment ParagraphStyle::GetHorizontalAlign() const {
  return impl_->GetHorizontalAlign();
}
void ParagraphStyle::SetHorizontalAlign(ParagraphHorizontalAlignment align) {
  impl_->SetHorizontalAlign(align);
}
ParagraphVerticalAlignment ParagraphStyle::GetVerticalAlign() const {
  return impl_->GetVerticalAlign();
}
void ParagraphStyle::SetVerticalAlign(
    ParagraphVerticalAlignment vertical_align) {
  impl_->SetVerticalAlign(vertical_align);
}
DominantBaseline ParagraphStyle::GetDominantBaseline() const {
  return impl_->GetDominantBaseline();
}
void ParagraphStyle::SetDominantBaseline(DominantBaseline dominant_baseline) {
  impl_->SetDominantBaseline(dominant_baseline);
}
float ParagraphStyle::GetStartIndentInPx() const {
  return impl_->GetStartIndentInPx();
}
void ParagraphStyle::SetStartIndentInPx(float px) {
  impl_->SetStartIndentInPx(px);
}
int32_t ParagraphStyle::GetStartIndentInCharCnt() const {
  return impl_->GetStartIndentInCharCnt();
}
void ParagraphStyle::SetStartIndentInCharCnt(int32_t char_count) {
  impl_->SetStartIndentInCharCnt(char_count);
}
float ParagraphStyle::GetEndIndentInPx() const {
  return impl_->GetEndIndentInPx();
}
void ParagraphStyle::SetEndIndentInPx(float px) { impl_->SetEndIndentInPx(px); }
int32_t ParagraphStyle::GetEndIndentInCharCnt() const {
  return impl_->GetEndIndentInCharCnt();
}
void ParagraphStyle::SetEndIndentInCharCnt(int32_t char_count) {
  impl_->SetEndIndentInCharCnt(char_count);
}
float ParagraphStyle::GetFirstLineIndentInPx() const {
  return impl_->GetFirstLineIndentInPx();
}
void ParagraphStyle::SetFirstLineIndentInPx(float px) {
  impl_->SetFirstLineIndentInPx(px);
}
int32_t ParagraphStyle::GetFirstLineIndentInCharCnt() const {
  return impl_->GetFirstLineIndentInCharCnt();
}
void ParagraphStyle::SetFirstLineIndentInCharCnt(int32_t char_count) {
  impl_->SetFirstLineIndentInCharCnt(char_count);
}
float ParagraphStyle::GetHangingIndentInPx() const {
  return impl_->GetHangingIndentInPx();
}
void ParagraphStyle::SetHangingIndentInPx(float px) {
  impl_->SetHangingIndentInPx(px);
}
int32_t ParagraphStyle::GetHangingIndentInCharCnt() const {
  return impl_->GetHangingIndentInCharCnt();
}
void ParagraphStyle::SetHangingIndentInCharCnt(int32_t char_count) {
  impl_->SetHangingIndentInCharCnt(char_count);
}
void ParagraphStyle::SetWriteDirection(WriteDirection dir) {
  impl_->SetWriteDirection(dir);
}
WriteDirection ParagraphStyle::GetWriteDirection() const {
  return impl_->GetWriteDirection();
}
float ParagraphStyle::GetSpacingAfterInPx() const {
  return impl_->GetSpacingAfterInPx();
}
void ParagraphStyle::SetSpacingAfterInPx(float px) {
  impl_->SetSpacingAfterInPx(px);
}
float ParagraphStyle::GetSpacingBeforeInPx() const {
  return impl_->GetSpacingBeforeInPx();
}
void ParagraphStyle::SetSpacingBeforeInPx(float px) {
  impl_->SetSpacingBeforeInPx(px);
}
float ParagraphStyle::GetLineHeightInPx() const {
  return impl_->GetLineHeightInPx();
}
void ParagraphStyle::SetLineHeightInPx(float line_height, RulerType rt) {
  impl_->SetLineHeightInPx(line_height, rt);
}
void ParagraphStyle::SetLineHeightInPxExact(float px) {
  impl_->SetLineHeightInPxExact(px);
}
void ParagraphStyle::SetLineHeightInPxAtLeast(float px) {
  impl_->SetLineHeightInPxAtLeast(px);
}
float ParagraphStyle::GetLineHeightInPercent() const {
  return impl_->GetLineHeightInPercent();
}
void ParagraphStyle::SetLineHeightInPercent(float percent) {
  impl_->SetLineHeightInPercent(percent);
}
RulerType ParagraphStyle::GetLineHeightRule() const {
  return impl_->GetLineHeightRule();
}
void ParagraphStyle::SetInlineVerticalAlignmentMode(
    InlineVerticalAlignmentMode mode) {
  impl_->SetInlineVerticalAlignmentMode(mode);
}
InlineVerticalAlignmentMode ParagraphStyle::GetInlineVerticalAlignmentMode()
    const {
  return impl_->GetInlineVerticalAlignmentMode();
}
void ParagraphStyle::SetLineSpaceBeforePx(float px) {
  impl_->SetLineSpaceBeforePx(px);
}
float ParagraphStyle::GetLineSpaceBeforePx() const {
  return impl_->GetLineSpaceBeforePx();
}
void ParagraphStyle::SetLineSpaceAfterPx(float px) {
  impl_->SetLineSpaceAfterPx(px);
}
float ParagraphStyle::GetLineSpaceAfterPx() const {
  return impl_->GetLineSpaceAfterPx();
}
const Spacing& ParagraphStyle::GetSpacing() const {
  return impl_->GetSpacing();
}
void ParagraphStyle::SetSpacing(const Spacing& spacing) {
  impl_->SetSpacing(spacing);
}
void ParagraphStyle::SetEllipsis(const std::u16string& ellipsis) {
  impl_->SetEllipsis(ellipsis);
}
void ParagraphStyle::SetEllipsis(const char* ellipsis) {
  impl_->SetEllipsis(ellipsis);
}
void ParagraphStyle::SetEllipsis(std::shared_ptr<RunDelegate> ellipsis) {
  impl_->SetEllipsis(std::move(ellipsis));
}
const std::u32string& ParagraphStyle::GetEllipsis() const {
  return impl_->GetEllipsis();
}
const std::shared_ptr<RunDelegate>& ParagraphStyle::GetEllipsisDelegate()
    const {
  return impl_->GetEllipsisDelegate();
}
uint32_t ParagraphStyle::GetMaxLines() const { return impl_->GetMaxLines(); }
void ParagraphStyle::SetMaxLines(uint32_t max_line) {
  impl_->SetMaxLines(max_line);
}
bool ParagraphStyle::LineHeightOverride() const {
  return impl_->LineHeightOverride();
}
void ParagraphStyle::SetLineHeightOverride(bool line_height_override) {
  impl_->SetLineHeightOverride(line_height_override);
}
bool ParagraphStyle::HalfLeading() const { return impl_->HalfLeading(); }
void ParagraphStyle::SetHalfLeading(bool half) { impl_->SetHalfLeading(half); }
void ParagraphStyle::EnableTextBounds(bool align_with_bbox) {
  impl_->EnableTextBounds(align_with_bbox);
}
bool ParagraphStyle::EnableTextBounds() const {
  return impl_->EnableTextBounds();
}
OverflowWrap ParagraphStyle::GetOverflowWrap() const {
  return impl_->GetOverflowWrap();
}
void ParagraphStyle::SetOverflowWrap(OverflowWrap overflow_wrap) {
  impl_->SetOverflowWrap(overflow_wrap);
}
void ParagraphStyle::AllowBreakAroundPunctuation(bool allow) {
  impl_->AllowBreakAroundPunctuation(allow);
}
ParagraphStyleImpl& ParagraphStyle::GetImpl() { return *impl_; }
const ParagraphStyleImpl& ParagraphStyle::GetImpl() const { return *impl_; }

}  // namespace tttext
}  // namespace ttoffice
