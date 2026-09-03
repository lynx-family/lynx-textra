// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/tttext_context_impl.h"

#include <textra/tttext_context.h>

#include <memory>
#include <utility>

#include "src/textlayout/layout_position.h"
namespace ttoffice {
namespace tttext {

TTTextContextImpl::TTTextContextImpl()
    : position_(std::make_unique<LayoutPosition>(0, 0)),
      last_line_can_overflow_(true),
      skip_spacing_before_first_line_(false),
      trim_line_tail_space_(true),
      harmony_shaper_force_low_api_(false),
      enable_system_font_adjust_(false),
      disable_shape_cache_(false),
      layout_bottom_(0) {}

TTTextContext::TTTextContext() : impl_(std::make_unique<TTTextContextImpl>()) {}

TTTextContext::~TTTextContext() = default;

bool TTTextContext::IsLastLineCanOverflow() const {
  return impl_->last_line_can_overflow_;
}

void TTTextContext::SetLastLineCanOverflow(bool lastLineCanOverflow) {
  impl_->last_line_can_overflow_ = lastLineCanOverflow;
}

bool TTTextContext::IsSkipSpacingBeforeFirstLine() const {
  return impl_->skip_spacing_before_first_line_;
}

void TTTextContext::SetSkipSpacingBeforeFirstLine(
    bool skipSpacingBeforeFirstLine) {
  impl_->skip_spacing_before_first_line_ = skipSpacingBeforeFirstLine;
}

bool TTTextContext::IsHarmonyShaperForceLowAPI() const {
  return impl_->harmony_shaper_force_low_api_;
}

void TTTextContext::SetHarmonyShaperForceLowAPI(bool harmonyShaperForceLowAPI) {
  impl_->harmony_shaper_force_low_api_ = harmonyShaperForceLowAPI;
}

bool TTTextContext::IsEnableSystemFontAdjust() const {
  return impl_->enable_system_font_adjust_;
}

void TTTextContext::SetEnableSystemFontAdjust(bool enable_system_font_adjust) {
  impl_->enable_system_font_adjust_ = enable_system_font_adjust;
}

void TTTextContext::SetShapeCacheScope(std::shared_ptr<ShapeCacheScope> scope) {
  impl_->shape_cache_scope_ = std::move(scope);
}

void TTTextContext::EnableFeature(FeatureOption feature_option, bool value) {
  switch (feature_option) {
    case kLastLineCanOverflow:
      impl_->last_line_can_overflow_ = value;
      break;
    case kSkipSpaceBeforeFirstLine:
      impl_->skip_spacing_before_first_line_ = value;
      break;
    case kTrimLineTailSpace:
      impl_->trim_line_tail_space_ = value;
      break;
    case kHarmonyShaperForceUseLowAPI:
      impl_->harmony_shaper_force_low_api_ = value;
      break;
    case kSystemFontAdjust:
      impl_->enable_system_font_adjust_ = value;
      break;
    case kDisableShapeCache:
      impl_->disable_shape_cache_ = value;
      break;
    default:
      break;
  }
}

void TTTextContext::Reset() {
  *impl_->position_ = LayoutPosition{0, 0};
  impl_->layout_bottom_ = 0;
}

void TTTextContext::SetLayoutPosition(uint32_t run_idx,
                                      uint32_t char_idx_in_run) const {
  *impl_->position_ = LayoutPosition{run_idx, char_idx_in_run};
}
std::pair<uint32_t, uint32_t> TTTextContext::GetLayoutPosition() const {
  return {impl_->position_->GetRunIdx(), impl_->position_->GetCharIdx()};
}

float TTTextContext::GetLayoutBottom() const { return impl_->layout_bottom_; }

void TTTextContext::SetLayoutBottom(float layout_bottom) {
  impl_->layout_bottom_ = layout_bottom;
}

LayoutPosition& TTTextContext::GetPositionRef() { return *impl_->position_; }
const TTTextContextImpl& TTTextContext::GetImpl() { return *impl_; }

}  // namespace tttext
}  // namespace ttoffice
