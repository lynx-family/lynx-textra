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
class TTTextContextImpl;
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
  bool IsLastLineCanOverflow() const;
  void SetLastLineCanOverflow(bool lastLineCanOverflow);
  /**
   * @brief Controls whether paragraph spacing is applied before the first line.
   *
   * Value: Boolean. When enabled, the first line starts at the top of the
   * layout area without leading space. Default is false.
   */
  bool IsSkipSpacingBeforeFirstLine() const;
  void SetSkipSpacingBeforeFirstLine(bool skipSpacingBeforeFirstLine);

  /**
   * @brief Force harmony system shaper only use low Level API from Harmony OS
   */
  bool IsHarmonyShaperForceLowAPI() const;
  void SetHarmonyShaperForceLowAPI(bool harmonyShaperForceLowAPI);

  /**
   *@brief Enable font style adjustments based on different systems.
   */
  bool IsEnableSystemFontAdjust() const;
  void SetEnableSystemFontAdjust(bool enable_system_font_adjust);

  void EnableFeature(FeatureOption feature_option, bool value);

  // Layout state getters/setters
 public:
  void Reset();
  void SetLayoutPosition(uint32_t run_idx, uint32_t char_idx_in_run) const;
  std::pair<uint32_t, uint32_t> GetLayoutPosition() const;
  float GetLayoutBottom() const;
  void SetLayoutBottom(float layout_bottom);
  const TTTextContextImpl& GetImpl();

 private:
  friend class TextLayoutImpl;
  friend class TextLayoutTest;
  friend class TTTextContextTest;
  L_HIDDEN LayoutPosition& GetPositionRef();

 private:
  std::unique_ptr<TTTextContextImpl> impl_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_
