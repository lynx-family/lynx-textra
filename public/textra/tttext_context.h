// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_
#define PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_

#include <textra/layout_definition.h>
#include <textra/macro.h>
#include <textra/text_line.h>

#include <memory>

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

  // Layout state getters/setters
 public:
  void Reset();
  void ResetLayoutPosition(const LayoutPosition& position);

 private:
  friend class TextLayoutImpl;
  friend class TextLayoutTest;
  friend class TTTextContextTest;
  L_HIDDEN LayoutPosition& GetPositionRef() { return *position_; }
  float GetLayoutBottom() const { return layout_bottom_; }
  float GetParagraphGap() const { return paragraph_space_; }
  float GetLineSpace() const { return line_space_; }
  float GetBottomMargin() const { return bottom_margin_; }
  float GetMinBottomHeight() const { return min_bottom_height_; }

  void SetLayoutBottom(float bottom) { layout_bottom_ = bottom; }
  void SetParagraphSpace(float paragraphSpace) {
    paragraph_space_ = paragraphSpace;
  }
  void SetLineSpace(float line_space) { line_space_ = line_space; }
  void SetBottomMargin(float bottomMargin) { bottom_margin_ = bottomMargin; }
  void SetMinBottomHeight(float minBottomHeight) {
    min_bottom_height_ = minBottomHeight;
  }

  L_HIDDEN void UpdateContextSpace(TextLine* line);

 private:
  // Layout Configurations
  bool last_line_can_overflow_{true};
  bool skip_spacing_before_first_line_{false};
  // Layout States
  std::unique_ptr<LayoutPosition> position_;
  float layout_bottom_{0};
  float paragraph_space_{0};
  float line_space_{0};
  float bottom_margin_{0};
  float min_bottom_height_{0};
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_
