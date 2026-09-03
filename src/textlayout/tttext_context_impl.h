// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_TTTEXT_CONTEXT_IMPL_H_
#define SRC_TEXTLAYOUT_TTTEXT_CONTEXT_IMPL_H_
#include <memory>

#include "src/textlayout/layout_position.h"
namespace ttoffice {
namespace tttext {
class ShapeCacheScope;
class TTTextContextImpl {
 public:
  TTTextContextImpl();
  ~TTTextContextImpl() = default;
  bool IsLastLineCanOverflow() const { return last_line_can_overflow_; }
  bool IsSkipSpacingBeforeFirstLine() const {
    return skip_spacing_before_first_line_;
  }
  bool IsHarmonyShaperForceLowAPI() const {
    return harmony_shaper_force_low_api_;
  }
  bool IsEnableSystemFontAdjust() const { return enable_system_font_adjust_; }
  bool IsShapeCacheDisabled() const { return disable_shape_cache_; }
  const std::shared_ptr<ShapeCacheScope>& GetShapeCacheScope() const {
    return shape_cache_scope_;
  }
  bool IsTrimLineTailSpace() const { return trim_line_tail_space_; }

 private:
  friend class TTTextContext;
  std::unique_ptr<LayoutPosition> position_;
  std::shared_ptr<ShapeCacheScope> shape_cache_scope_;
  bool last_line_can_overflow_;
  bool skip_spacing_before_first_line_;
  bool trim_line_tail_space_;
  bool harmony_shaper_force_low_api_;
  bool enable_system_font_adjust_;
  bool disable_shape_cache_;
  float layout_bottom_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_TTTEXT_CONTEXT_IMPL_H_
