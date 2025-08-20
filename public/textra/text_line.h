// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_TEXT_LINE_H_
#define PUBLIC_TEXTRA_TEXT_LINE_H_
#include <textra/macro.h>
#include <textra/paragraph.h>

#include <limits>

namespace ttoffice {
namespace tttext {
enum class LineType : uint8_t;
using CharPos = uint32_t;

/**
 * @brief Represents a single line of text produced by the text layout system
 * after line breaking.
 *
 * Each TextLine is defined by a start and end position within a paragraph and
 * includes line metrics.
 */
class L_EXPORT TextLine {
 public:
  TextLine() = default;
  virtual ~TextLine() = default;

 public:
  void Initialize(const float top) { line_top_ = top; }
  float GetLineTop() const { return line_top_; }
  float GetLineBaseLine() const { return line_top_ + max_ascent_ + top_extra_; }
  float GetLineBottom() const { return line_top_ + GetLineHeight(); }
  float GetLineHeight() const {
    return GetContentHeight() + top_extra_ + bottom_extra_;
  }
  float GetContentTop() const { return top_extra_; }
  float GetContentBaseline() const { return GetContentTop() + max_ascent_; }
  float GetContentBottom() const {
    return GetContentTop() + GetContentHeight();
  }
  float GetContentHeight() const { return max_ascent_ + max_descent_; }
  float GetLineWidth() const { return GetLineRight() - GetLineLeft(); }
  float GetMaxAscent() const { return max_ascent_; }
  float GetMaxDescent() const { return max_descent_; }
  void SetNoEmpty() { empty_ = false; }
  virtual bool IsEmpty() const = 0;
  void SetLayouted() { layouted_ = true; }
  bool IsLayouted() const { return layouted_; }
  virtual float GetLineLeft() const = 0;
  virtual float GetLineRight() const = 0;

  virtual CharPos GetStartCharPos() const = 0;
  virtual CharPos GetEndCharPos() const = 0;
  CharPos GetCharCount() const { return GetEndCharPos() - GetStartCharPos(); }

  virtual bool IsLastLineOfParagraph() const = 0;

  virtual Paragraph* GetParagraph() const = 0;
  void UpdateLineTop(const float top) { line_top_ = top; }
  virtual void UpdateXMax(float width) = 0;
  virtual void ApplyAlignment() = 0;
  virtual void StripByEllipsis(const char32_t* ellipsis) = 0;

 public:
  /**
   *
   * @param char_pos paragraph text offset index
   * @param bounding_rect return value, [left, top, width, height]
   */
  void GetCharBoundingRect(float bounding_rect[4],
                           const uint32_t char_pos) const {
    return GetBoundingRectByCharRange(bounding_rect, char_pos, char_pos + 1);
  }
  /**
   * Get the x-axis region of the entire line text
   * @param bounding_rect return value, [left, top, width, height]
   */
  void GetBoundingRectForLine(float bounding_rect[4]) const {
    GetBoundingRectByCharRange(bounding_rect, 0,
                               std::numeric_limits<CharPos>::max());
  }
  /**
   *
   * @param start_char_pos paragraph text offset index
   * @param end_char_pos paragraph text offset index
   * @param bounding_rect return value, [left, top, width, height]
   */
  virtual void GetBoundingRectByCharRange(float bounding_rect[4],
                                          CharPos start_char_pos,
                                          CharPos end_char_pos) const = 0;
  /**
   * Get CharPos based on x coordinate, character offset relative to
   * LineStartPos, first character ID at line start is 0
   * @param x x coordinate
   * @return character offset
   */
  virtual CharPos GetCharPosByCoordinateX(float x) const = 0;
  float GetStartIndent() const { return start_indent_; }
  float GetEndIndent() const { return end_indent_; }

 protected:
  float line_top_ = 0;
  float max_ascent_ = 0;   // distance from linetop to baseline
  float max_descent_ = 0;  // distance from baseline to linebottom
  float top_extra_ = 0;
  float bottom_extra_ = 0;
  float start_indent_ = 0;
  float end_indent_ = 0;
  bool empty_ = true;
  bool layouted_ = false;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_TEXT_LINE_H_
