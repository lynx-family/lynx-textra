// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PARAGRAPH_STYLE_H_
#define PUBLIC_TEXTRA_PARAGRAPH_STYLE_H_

#include <textra/layout_definition.h>
#include <textra/macro.h>
#include <textra/style.h>

#include <cstdint>
#include <limits>
#include <memory>

namespace ttoffice {
namespace tttext {
class RunDelegate;
struct Indent;
struct Spacing;
/**
 * @brief Defines formatting and layout properties for a paragraph of text.
 *
 * ParagraphStyle encapsulates paragraph-level styling attributes that control
 * how text is laid out and rendered in the text layout system. It manages:
 * - Horizontal and vertical alignment
 * - Indentation (first-line, hanging, start/end)
 * - Line and paragraph spacing
 * - Text direction and writing mode
 * - Line breaking and hyphenation
 * - Truncation (ellipsis)
 * - Maximum number of lines
 *
 * It works with the Style class, which handles character-level formatting.
 * The default_style_ defines the base character style for the paragraph.
 *
 * Used by the Paragraph class to control layout behavior in the text engine.
 */
class L_EXPORT ParagraphStyle {
 public:
  ParagraphStyle();
  ParagraphStyle(const ParagraphStyle& paragraph_style);
  ~ParagraphStyle();
  ParagraphStyle& operator=(const ParagraphStyle& paragraph_style);

 public:
  /**
   * @brief Default character-level style for the paragraph.
   *
   * Provides the base Style that applies to all text in the paragraph unless
   * overridden by specific text runs. This includes font, size, color, and
   * other character-level formatting.
   *
   * @see Style class for complete character-level formatting options
   */
  const Style& GetDefaultStyle() const { return default_style_; }
  void SetDefaultStyle(const Style& default_style) {
    default_style_ = default_style;
  }

  /**
   * @brief Horizontal text alignment within the paragraph.
   *
   * Controls how text lines are positioned horizontally within the available
   * paragraph width. This affects text distribution and spacing.
   *
   * Value: See ParagraphHorizontalAlignment enum, default kLeft.
   *
   * @warning kDistributed alignment is defined but not implemented.
   *
   */
  ParagraphHorizontalAlignment GetHorizontalAlign() const {
    return horizontal_alignment_;
  }
  void SetHorizontalAlign(ParagraphHorizontalAlignment align) {
    horizontal_alignment_ = align;
  }

  /**
   * @brief Vertical alignment of the paragraph within its container.
   *
   * Controls how the entire paragraph is positioned vertically within the
   * available layout region height.
   *
   * Value: ParagraphVerticalAlignment enum. kTop aligns to top of container,
   * kCenter centers vertically in container, kBottom aligns to bottom of
   * container. Default is kCenter. Only affects multi-line paragraphs with
   * extra vertical space.
   *
   * @see ParagraphVerticalAlignment enum for available values
   *
   * @note Different from Style::VerticalAlignment which affects character
   * positioning within a line (e.g. for superscript/subscript)
   */
  ParagraphVerticalAlignment GetVerticalAlign() const {
    return vertical_alignment_;
  }
  void SetVerticalAlign(ParagraphVerticalAlignment vertical_align) {
    vertical_alignment_ = vertical_align;
  }

  /**
   * @brief Start indentation in pixels.
   *
   * Sets or gets the indentation from the start edge of the paragraph
   * (left edge in LTR text, right edge in RTL text) measured in pixels.
   *
   * Value: Any float value in pixels. Positive values indent inward from the
   * edge, negative values extend beyond the normal margin. Default is 0.0f
   * pixels (no indentation).
   *
   * This reduces available text width and is applied to all lines except the
   * first line. The first line uses GetFirstLineIndentInPx() instead. For
   * subsequent lines, this combines with GetHangingIndentInPx() to determine
   * the total indentation.
   *
   * @note Setting pixel indentation resets character count indentation to 0
   */
  float GetStartIndentInPx() const;
  void SetStartIndentInPx(float px);

  /**
   * @brief Start indentation in character count.
   *
   * Similar to GetStartIndentInPx(), but measured in character count. Actual
   * indent in pixels is proportional to font size.
   *
   * Value: Any int32_t value representing character count. Default is 0
   * characters (no indentation).
   *
   * @see GetStartIndentInPx() for detailed behavior description
   * @note Setting character indentation resets pixel indentation to 0
   */
  int32_t GetStartIndentInCharCnt() const;
  void SetStartIndentInCharCnt(int32_t char_count);

  /**
   * @brief End indentation in pixels.
   *
   * Sets or gets the indentation from the end edge of the paragraph
   * (right edge in LTR text, left edge in RTL text) measured in pixels.
   *
   * Value: Any float value in pixels. Positive values indent inward from the
   * edge, negative values extend beyond the normal margin. Default is 0.0f
   * pixels (no indentation).
   *
   * This reduces available text width and is applied consistently to all
   * lines in the paragraph. It works with start indentation to define the
   * text area.
   *
   * @note Setting pixel indentation resets character count indentation to 0
   */
  float GetEndIndentInPx() const;
  void SetEndIndentInPx(float px);

  /**
   * @brief End indentation in character count.
   *
   * Similar to GetEndIndentInPx(), but measured in character count. Actual
   * indent in pixels is proportional to font size.
   *
   * Value: Any int32_t value representing character count. Default is 0
   * characters (no indentation).
   *
   * @see GetEndIndentInPx() for detailed behavior description
   * @note Setting character indentation resets pixel indentation to 0
   */
  int32_t GetEndIndentInCharCnt() const;
  void SetEndIndentInCharCnt(int32_t char_count);

  /**
   * @brief Additional indentation for the first line only.
   *
   * Sets or gets extra indentation applied only to the first line of the
   * paragraph, in addition to any start indentation. Measured in pixels.
   *
   * Value: Any float value in pixels. Positive values create additional
   * first-line indentation, negative values create hanging indent (first line
   * extends left). Default is 0.0f pixels (no additional indentation).
   *
   * This is used exclusively for the first line and overrides the start
   * indentation for that line. Positive values create traditional paragraph
   * indentation, while negative values create hanging indent effect. It is
   * commonly used for paragraph formatting in documents.
   *
   * @note Setting pixel indentation resets character count indentation to 0
   */
  float GetFirstLineIndentInPx() const;
  void SetFirstLineIndentInPx(float px);

  /**
   * @brief Additional indentation for the first line in character count.
   *
   * Similar to GetFirstLineIndentInPx(), but measured in character count.
   * The actual indent in pixels is proportional to font size.
   *
   * Value: Any int32_t value representing character count. Default is 0
   * characters (no additional indentation).
   *
   * @see GetFirstLineIndentInPx() for detailed behavior description
   * @note Setting character indentation resets pixel indentation to 0
   */
  int32_t GetFirstLineIndentInCharCnt() const;
  void SetFirstLineIndentInCharCnt(int32_t char_count);

  /**
   * @brief Additional indentation for lines after the first.
   *
   * Sets or gets additional indentation applied to all lines except the
   * first line. Creates a hanging indent effect. Measured in pixels.
   *
   * Value: Any float value in pixels. Positive values indent subsequent
   * lines inward, negative values extend subsequent lines outward. Default is
   * 0.0f pixels (no hanging indentation).
   *
   * This is applied to all lines except the first and is combined with start
   * indentation for subsequent lines.
   *
   * @note Setting pixel indentation resets character count indentation to 0
   */
  float GetHangingIndentInPx() const;
  void SetHangingIndentInPx(float px);

  /**
   * @brief Hanging indentation in character count.
   *
   * Similar to GetHangingIndentInPx(), but measured in character count.
   * The actual indent in pixels is proportional to font size.
   *
   * Value: Any int32_t value representing character count. Default is 0
   * characters (no hanging indentation).
   *
   * @see GetHangingIndentInPx() for detailed behavior description
   * @note Setting character indentation resets pixel indentation to 0
   */
  int32_t GetHangingIndentInCharCnt() const;
  void SetHangingIndentInCharCnt(int32_t char_count);

  /**
   * @brief Text writing direction and script orientation.
   *
   * Controls the base direction for text layout, affecting both character
   * order and paragraph-level alignment behavior.
   *
   * Value: WriteDirection enum. kAuto automatically detects from text content,
   * kLTR is left-to-right (Latin, Greek, Cyrillic scripts), kRTL is
   * right-to-left (Arabic, Hebrew scripts), kTTB is top-to-bottom (some Asian
   * scripts), kBTT is bottom-to-top (rare, specialized use). Default is kAuto.
   *
   * kAuto determines direction by the first strong directional character. kLTR
   * makes characters flow left-to-right where "start" means left edge. kRTL
   * makes characters flow right-to-left where "start" means right edge.
   * kTTB/kBTT are vertical writing modes with limited support. This affects the
   * meaning of start/end indentation and alignment.
   *
   * Auto-detection works well for most content. Manual setting is useful for
   * empty paragraphs or mixed-script documents.
   */
  void SetWriteDirection(WriteDirection dir) { write_direction_ = dir; }
  WriteDirection GetWriteDirection() const { return write_direction_; }

  /**
   * @brief Spacing after the paragraph in pixels.
   *
   * Controls the vertical space added after the paragraph, affecting
   * the gap between this paragraph and the following content.
   *
   * Value: Any float value in pixels. Positive values add space after
   * the paragraph, zero means no additional spacing. Default is 0.0f pixels (no
   * additional spacing).
   *
   * This creates vertical separation between paragraphs and does not affect
   * line spacing within the paragraph. It is useful for paragraph separation in
   * documents.
   *
   * @warning Currently not used by the layout engine.
   */
  float GetSpacingAfterInPx() const;
  void SetSpacingAfterInPx(float px);

  /**
   * @brief Spacing before the paragraph in pixels.
   *
   * Controls the vertical space added before the paragraph, affecting
   * the gap between the preceding content and this paragraph.
   *
   * Value: Any float value in pixels. Positive values add space before
   * the paragraph, zero means no additional spacing. Default is 0.0f pixels (no
   * additional spacing).
   *
   * This creates vertical separation from previous content and does not affect
   * line spacing within the paragraph. It is useful for paragraph separation
   * and section breaks.
   */
  float GetSpacingBeforeInPx() const;
  void SetSpacingBeforeInPx(float px);

  /**
   * @brief Line height in pixels with specified rule type.
   *
   * Controls the vertical spacing between lines within the paragraph.
   * The rule type determines how the specified height is interpreted.
   *
   * @param line_height Height value in pixels
   * @param rt Rule type controlling height interpretation
   *
   * Combined with SetLineHeightInPx() for flexible line height control.
   */
  void SetLineHeightInPx(float line_height, RulerType rt);
  float GetLineHeightInPx() const;

  /**
   * @brief Sets exact line height in pixels.
   *
   * Forces all lines to have exactly the specified height, regardless
   * of font size or content. May clip tall characters if too small.
   *
   * Value: Any positive float value in pixels. Should be larger than
   * font size to avoid clipping.
   *
   * This uses RulerType::kExact to make lines have exactly this height.
   * It may clip tall characters or accents if height is insufficient,
   * but provides precise vertical rhythm control.
   */
  void SetLineHeightInPxExact(float px);

  /**
   * @brief Sets minimum line height in pixels.
   *
   * Ensures lines are at least the specified height, but allows
   * taller lines if needed to accommodate large fonts or characters.
   *
   * Value: Any positive float value in pixels. Lines will be this
   * tall or taller as needed.
   *
   * This uses RulerType::kAtLeast to ensure lines are at least this height.
   * It prevents lines from being too cramped and adapts to content requirements
   * while maintaining minimum spacing.
   */
  void SetLineHeightInPxAtLeast(float px);

  /**
   * @brief Line height as percentage of font size.
   *
   * Sets line height proportional to the font size. Common values
   * range from 100% (no extra spacing) to 150% (1.5x font size).
   *
   * Value: Any positive float value as percentage.
   * 100.0 means line height equals font size, 150.0 means line height is 1.5x
   * font size (common for readability). Default is 100.0% (line height equals
   * font size).
   *
   * This scales proportionally with font size changes and maintains consistent
   * visual rhythm. It follows standard typography practice for readable text.
   */
  float GetLineHeightInPercent() const;
  void SetLineHeightInPercent(float percent);

  /**
   * @brief Current line height rule type.
   *
   * Returns how the line height is being interpreted and applied.
   *
   * Value: RulerType enum.
   * - kAuto provides automatic line height based on font metrics.
   * - kExact provides fixed line height (may clip content).
   * - kAtLeast provides minimum line height (expands as needed).
   * Default is kAuto.
   */
  RulerType GetLineHeightRule() const;

  /**
   * @brief Additional spacing before each line.
   *
   * Adds extra vertical space above each line, in addition to the
   * base line height. Applied to every line in the paragraph.
   *
   * Value: Any float value in pixels. Positive values increase line
   * separation, negative values decrease line separation (may cause overlap).
   * Default is 0.0f pixels (no additional spacing).
   *
   * This increases visual space between lines and is applied uniformly to
   * all lines. It combines with line height and after spacing.
   */
  void SetLineSpaceBeforePx(float px);
  float GetLineSpaceBeforePx() const;

  /**
   * @brief Additional spacing after each line.
   *
   * Similar to SetLineSpaceBeforePx(), but for the after spacing.
   */
  void SetLineSpaceAfterPx(float px);
  float GetLineSpaceAfterPx() const;

  /**
   * @brief Complete spacing configuration structure.
   *
   * Access to the full Spacing object containing all paragraph and
   * line spacing settings in a single structure.
   *
   * The Spacing structure contains paragraph spacing (before/after), line
   * height settings (pixels, percentage, rule type), line spacing (before/after
   * each line), and auto spacing flags for smart spacing.
   *
   * @note Spacing is an opaque type and can only be copied as a whole when
   * copying its owners (like ParagraphStyle).
   */
  const Spacing& GetSpacing() const;
  void SetSpacing(const Spacing& spacing);

  /**
   * @brief Sets text-based ellipsis for text truncation.
   *
   * Configures the text string to display when content is truncated due to
   * space constraints or maximum line limits. This replaces truncated text at
   * the end of visible content and uses the paragraph's default style for
   * rendering. It clears any custom ellipsis delegate.
   *
   * @param ellipsis UTF-16 string to display as ellipsis (e.g., "...", "…",
   * " [more]").
   */
  void SetEllipsis(const std::u16string& ellipsis);

  /**
   * @brief Sets text-based ellipsis using C-style string.
   *
   * @param ellipsis UTF-8 C-string for ellipsis display
   */
  void SetEllipsis(const char* ellipsis);

  /**
   * @brief Sets custom delegate-based ellipsis.
   *
   * Uses a RunDelegate object for custom ellipsis rendering, enabling complex
   * ellipsis graphics, icons, or interactive elements. The RunDelegate controls
   * size, appearance, and behavior.
   *
   * @param ellipsis RunDelegate for custom ellipsis rendering
   */
  void SetEllipsis(std::shared_ptr<RunDelegate> ellipsis);

  /**
   * @brief Current ellipsis text string.
   *
   * @return UTF-32 string currently used for text truncation
   */
  const std::u32string& GetEllipsis() const { return ellipsis_; }

  /**
   * @brief Current custom ellipsis delegate.
   *
   * @return RunDelegate object for custom ellipsis, or nullptr if using text
   */
  const std::shared_ptr<RunDelegate>& GetEllipsisDelegate() const {
    return ellipsis_delegate_;
  }

  /**
   * @brief Maximum number of lines before truncation.
   *
   * Controls text truncation by limiting the number of lines that can
   * be displayed. Excess content is replaced with ellipsis.
   *
   * Value: 1 to max uint32_t for specific line limit, or 0 for unlimited
   * lines (converted to max uint32_t internally). Default is
   * unlimited.
   *
   * Text beyond the limit is truncated and replaced with ellipsis. This is
   * useful for preview text, labels, and space-constrained layouts. It
   * interacts with ellipsis settings for truncation appearance. Line counting
   * includes wrapped lines, not just paragraph breaks.
   */
  uint32_t GetMaxLines() const { return max_lines_; }
  void SetMaxLines(uint32_t max_line) {
    max_lines_ =
        max_line == 0 ? std::numeric_limits<uint32_t>::max() : max_line;
  }

  /**
   * @brief Controls line height calculation override.
   *
   * Line height is first calculated from font metrics. This flag controls
   * whether the font metrics should be overridden to match the text size.
   *
   * Value: Boolean. When false, line height is calculated from font metrics.
   * When true, font metrics is additionally adjusted to match text size.
   * Default is false (native metrics).
   *
   * When enabled, this interacts with HalfLeading() to determine the exact
   * adjustment method.
   */
  bool LineHeightOverride() const { return line_height_override_; }
  void SetLineHeightOverride(bool override) {
    line_height_override_ = override;
  }

  /**
   * @brief Controls how the line height difference (from LineHeightOverride())
   * is distributed around the text.
   *
   * Value: Boolean. When true, the difference between font height and text size
   * (which may be positive or negative) is added entirely to the ascent (above
   * the text baseline). When false, ascent and descent are scaled
   * proportionally to match the text size. Default is false (proportional
   * scaling).
   *
   * This only takes effect when LineHeightOverride() is true.
   */
  bool HalfLeading() const { return half_leading_; }
  void SetHalfLeading(bool half) { half_leading_ = half; }

  /**
   * @brief Controls whether text baseline positioning uses actual glyph
   * bounding boxes.
   *
   * Value: Boolean. When false, baseline positioning uses font metrics.
   * When true, the layout engine calculates the actual bounding box of each
   * glyph and adjusts the baseline offset to better balance the text within the
   * available line height. This is particularly useful for mixed-font text or
   * when precise vertical alignment is required. Default is false.
   */
  void EnableTextBounds(bool align_with_bbox) {
    enable_text_bounds_ = align_with_bbox;
  }
  bool EnableTextBounds() const { return enable_text_bounds_; }

  /**
   * @brief Controls overflow wrapping behavior for line breaking.
   *
   * Determines how text wraps when words are too long to fit on a line,
   * providing different strategies for handling overflow content.
   *
   * Value: OverflowWrap enum.
   * - kNormal breaks only at normal word boundaries
   * - kAnywhere breaks anywhere to prevent overflow
   * - kBreakWord breaks long words if necessary.
   * Default is OverflowWrap::kAnywhere.
   *
   * @warning kAnywhere and kBreakWord currently have identical behavior in the
   * implementation.
   */
  OverflowWrap GetOverflowWrap() const { return overflow_wrap_; }
  void SetOverflowWrap(OverflowWrap overflow_wrap) {
    overflow_wrap_ = overflow_wrap;
  }

  /**
   * @brief Controls whether line breaks are allowed around punctuation marks.
   *
   * When false, avoids breaking after left punctuation (like '(') and before
   * right punctuation (like ')'), keeping punctuation attached to adjacent
   * words. When true, allows breaks around all punctuation marks for more
   * flexible line breaking. Default is false.
   */
  void AllowBreakAroundPunctuation(bool allow);

 protected:
  ParagraphHorizontalAlignment horizontal_alignment_;
  ParagraphVerticalAlignment vertical_alignment_;
  Style default_style_;
  std::unique_ptr<Indent> indent_;
  std::unique_ptr<Spacing> spacing_;

  WriteDirection write_direction_;
  std::u32string ellipsis_;
  std::shared_ptr<RunDelegate> ellipsis_delegate_;
  uint32_t max_lines_;
  bool line_height_override_;
  bool half_leading_;
  bool enable_text_bounds_;
  OverflowWrap overflow_wrap_;
  LineBreakStrategy line_break_strategy_;

  friend class ParagraphImpl;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PARAGRAPH_STYLE_H_
