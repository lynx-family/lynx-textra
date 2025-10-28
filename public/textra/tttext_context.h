// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_
#define PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_

#include <textra/layout_definition.h>
#include <textra/macro.h>

#include <memory>
#include <utility>

namespace ttoffice {
namespace tttext {
class LayoutRegion;
class LayoutPosition;
class TextLayoutImpl;
/**
 * @brief A class manages the text layout configurations and intermediate layout
 * states.
 *
 * The configurations should be specified by clients before the layout
 * process. The intermediate layout states are set by TextLayout during the
 * layout process.
 */
class L_EXPORT TTTextContext {
 public:
  TTTextContext();
  ~TTTextContext();

 public:
  // Layout Configurations getters/setters

  /**
   * @brief Controls whether the last line of text can overflow page height
   * boundaries.
   *
   * Value: Boolean. When enabled, the last line may overflow beyond the page
   * height if the line's top position is within the page bounds; when disabled,
   * the last line is always constrained to fit within the page height.
   * Default is true.
   */
  bool IsLastLineCanOverflow() const { return last_line_can_overflow_; }
  void SetLastLineCanOverflow(bool lastLineCanOverflow) {
    last_line_can_overflow_ = lastLineCanOverflow;
  }
  /**
   * @brief Controls whether paragraph spacing is applied before the first line.
   *
   * Value: Boolean. When enabled, the first line starts at the top of the
   * layout area without leading space. Default is false.
   */
  bool IsSkipSpacingBeforeFirstLine() const {
    return skip_spacing_before_first_line_;
  }
  void SetSkipSpacingBeforeFirstLine(bool skipSpacingBeforeFirstLine) {
    skip_spacing_before_first_line_ = skipSpacingBeforeFirstLine;
  }

  /**
   * @brief Force harmony system shaper only use low Level API from Harmony OS
   */
  bool IsHarmonyShaperForceLowAPI() const {
    return harmony_shaper_force_low_api_;
  }
  void SetHarmonyShaperForceLowAPI(bool harmonyShaperForceLowAPI) {
    harmony_shaper_force_low_api_ = harmonyShaperForceLowAPI;
  }

  // Layout state getters/setters
 public:
  void Reset();
  void SetLayoutPosition(uint32_t run_idx, uint32_t char_idx_in_run) const;
  std::pair<uint32_t, uint32_t> GetLayoutPosition() const;
  float GetLayoutBottom() const { return layout_bottom_; }
  void SetLayoutBottom(float layout_bottom) { layout_bottom_ = layout_bottom; }

 private:
  friend class TextLayoutImpl;
  friend class TextLayoutTest;
  friend class TTTextContextTest;
  L_HIDDEN LayoutPosition& GetPositionRef() { return *position_; }

 private:
  // Layout Configurations
  bool last_line_can_overflow_{true};
  bool skip_spacing_before_first_line_{false};
  bool harmony_shaper_force_low_api_{false};
  // Layout States
  std::unique_ptr<LayoutPosition> position_;
  float layout_bottom_{0};
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_
