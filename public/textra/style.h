// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_STYLE_H_
#define PUBLIC_TEXTRA_STYLE_H_

#include <textra/font_info.h>
#include <textra/layout_definition.h>
#include <textra/macro.h>
#include <textra/painter.h>
#include <textra/tt_color.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace ttoffice {
namespace tttext {
class BaseRun;
class GhostRun;
class ShapeStyle;
class StyleImpl;
class StyleManager;
class ParagraphImpl;
class ShaperSkShaper;

/**
 * @brief A data structure encapsulating character-level text formatting
 * attributes.
 *
 * Attributes include:
 * - Font attributes (font descriptors, size, scaling, bold, italic, etc.)
 * - Color attributes (foreground, background, decoration colors)
 * - Decoration attributes (decoration type, style, thickness)
 * - Spacing attributes (letter spacing, word spacing)
 * - Alignment and float attributes
 * - Text effects (text shadows, etc.)
 *
 * Clients can provide a default Style via ParagraphStyle or assign styles
 * to individual runs when constructing a paragraph. Style works with
 * ParagraphStyle to influence text layout and rendering.
 */
class L_EXPORT Style {
  friend BaseRun;
  friend ParagraphImpl;
  friend ShaperSkShaper;
  using FlagType = AttrType;
  using TextShadowList = std::vector<TextShadow>;

 public:
  Style();
  Style(const Style& style);
  Style& operator=(const Style& other);
  L_HIDDEN explicit Style(const StyleImpl& style);
  L_HIDDEN Style& operator=(const StyleImpl& other);
  ~Style();

  L_HIDDEN StyleImpl& GetImpl();
  L_HIDDEN const StyleImpl& GetImpl() const;

 public:
  void Reset();
  void ClearShapeStyle() const;

  /**
   * @brief Specifies the font family, weight, width, and slant for text.
   *
   * The font descriptor determines which font will be used for text rendering.
   * It includes a list of font families for fallback support, font style
   * attributes, and an optional platform-specific font handle.
   *
   * Value: FontDescriptor struct with font_family_list_ (vector of font names),
   * font_style_ (see FontStyle), and platform_font_ (uint64_t platform handle,
   * default: 0).
   *
   * The platform_font_ field stores a direct reference to native platform font
   * objects: CTFontRef on iOS/macOS, Java font handles on Android, or
   * SkTypeface* on Skia. When set, it bypasses font family resolution and uses
   * the native font directly.
   */
 public:
  const FontDescriptor& GetFontDescriptor() const;
  void SetFontDescriptor(const FontDescriptor& val);

  /**
   * @brief Sets the font size in pixels.
   *
   * Specifies the nominal font size used for text rendering. This is combined
   * with TextScale to determine the final rendered size.
   *
   * Value: Positive float in pixels, default 13.33f (equivalent to 10pt at 96
   * DPI).
   *
   * The final rendered size is calculated as TextSize × TextScale.
   */
 public:
  float GetTextSize() const;
  void SetTextSize(const float& val);

  /**
   * @brief Scaling factor applied to the base font size.
   *
   * Multiplier applied to TextSize to achieve the final rendered font size.
   * Useful for proportional scaling without changing the base font size.
   *
   * Value: Positive float (typically 0.1f to 10.0f), default 1.0f.
   */
 public:
  float GetTextScale() const;
  void SetTextScale(const float& val);

  /**
   * @brief Color used for text rendering.
   *
   * Specifies the color of the text glyphs. Can be overridden by custom
   * ForegroundPainter for advanced rendering effects.
   *
   * Value: TTColor in ARGB format, default TTColor::BLACK() (0xFF000000).
   *
   * Ignored when ForegroundPainter is set.
   */
 public:
  TTColor GetForegroundColor() const;
  void SetForegroundColor(const TTColor& val);

  /**
   * @brief Background color behind text.
   *
   * Draws a rectangular background behind the text using the specified color.
   * Can be overridden by custom BackgroundPainter for advanced effects.
   *
   * Value: TTColor in ARGB format, default TTColor::UNDEFINED() (no
   * background).
   *
   * Ignored when BackgroundPainter is set.
   */
 public:
  TTColor GetBackgroundColor() const;
  void SetBackgroundColor(const TTColor& val);

  /**
   * @brief Color for text decorations (underline, strikethrough, overline).
   *
   * Specifies the color used for drawing text decorations.
   *
   * Value: TTColor in ARGB format, default TTColor::UNDEFINED()
   * (transparent).
   */
 public:
  TTColor GetDecorationColor() const;
  void SetDecorationColor(const TTColor& val);

  /**
   * @brief Type of text decoration to apply.
   *
   * Specifies which text decorations should be drawn. The enum values are
   * designed for bitwise operations.
   *
   * Value: See DecorationType enum, default DecorationType::kNone.
   *
   * @note Bitwise combining is not implemented in the current rendering code -
   * only single decoration types are supported.
   */
 public:
  DecorationType GetDecorationType() const;
  void SetDecorationType(const DecorationType& val);

  /**
   * @brief Controls the visual appearance of text decoration lines (solid,
   * dashed, dotted, etc.).
   *
   * Value: See LineType enum, default LineType::kSolid.
   *
   * @note Currently only kSolid and kDashed are implemented for text
   * decorations. Other styles are ignored and no decoration is drawn.
   */
 public:
  LineType GetDecorationStyle() const;
  void SetDecorationStyle(const LineType& val);

  /**
   * @brief Multiplier for decoration line thickness.
   *
   * Scales the default thickness of decoration lines. Useful for creating
   * thicker or thinner decorations relative to the font size.
   *
   * Value: Positive float (typically 0.1f to 5.0f), default 1.0f.
   */
 public:
  float GetDecorationThicknessMultiplier() const;
  void SetDecorationThicknessMultiplier(const float& val);

  /**
   * @brief Length of each dashed decoration element.
   *
   * Controls the dash length used for dashed text decorations.
   *
   * Value: Positive float in pixels, default 2.5f.
   */
 public:
  float GetDecorationElementLength() const;
  void SetDecorationElementLength(const float& val);

  /**
   * @brief Gap length between dashed decoration elements.
   *
   * Controls the spacing between dash elements used for dashed text
   * decorations.
   *
   * Value: Non-negative float in pixels, default 1.5f.
   */
 public:
  float GetDecorationGapLength() const;
  void SetDecorationGapLength(const float& val);

  /**
   * @brief Side margin for dashed decoration lines.
   *
   * Limits the leading and trailing inset used when drawing dashed text
   * decorations.
   *
   * Value: Non-negative float in pixels, default 2.5f.
   */
 public:
  float GetDecorationSideMargin() const;
  void SetDecorationSideMargin(const float& val);

  /**
   * @brief Text Stroke Style.
   *
   * Useful for setting text stroke style
   *
   * Value: color default TTColor::Undefined, thickness is positive float
   * (typically 0.1f to 5.0f), default 1.0f.
   */
 public:
  void SetTextStrokeStyle(const TTColor& color, float thickness);
  TTColor GetTextStrokeColor() const;
  float GetTextStrokeWidth() const;
  uint64_t GetTextStrokeValue() const;
  void SetTextStrokeValue(uint64_t val);

  /**
   * @brief Enables bold text rendering.
   *
   * Value: Boolean, default false.
   *
   * @note Bold formatting does not affect font selection/shaping and only
   * applies fake bold effects during rendering. Use FontDescriptor with
   * FontStyle::Bold() for proper font selection.
   */
 public:
  bool GetBold() const;
  void SetBold(const bool& val);

  /**
   * @brief Enables italic text rendering.
   *
   * Value: Boolean, default false.
   *
   * @note Italic formatting does not affect font selection/shaping and only
   * applies fake italic effects during rendering. Use FontDescriptor with
   * FontStyle::Italic() for proper font selection.
   */
 public:
  bool GetItalic() const;
  void SetItalic(const bool& val);

  /**
   * @brief Vertical alignment of text relative to the baseline.
   *
   * Controls how text is positioned vertically, particularly useful for
   * superscript, subscript, and mixed-size text alignment.
   *
   * Value: See CharacterVerticalAlignment enum, default kBaseLine.
   */
 public:
  CharacterVerticalAlignment GetVerticalAlignment() const;
  void SetVerticalAlignment(const CharacterVerticalAlignment& val);

  /**
   * @brief Additional spacing between words.
   *
   * Adds extra space between words in addition to the natural word spacing
   * determined by the font metrics.
   *
   * Value: Float value in pixels, default 0.0f.
   *
   * @warning This feature is not yet implemented.
   */
 public:
  float GetWordSpacing() const;
  void SetWordSpacing(const float& val);

  /**
   * @brief Additional spacing between individual characters.
   *
   * Adds extra space between each character in addition to the font's
   * natural character spacing and kerning.
   *
   * Value: Float value in pixels, default 0.0f.
   *
   * Applied after font kerning calculations and affects text measurement and
   * layout width.
   */
 public:
  float GetLetterSpacing() const;
  void SetLetterSpacing(const float& val);

  /**
   * @brief Custom painter for text foreground rendering.
   *
   * Allows complete control over text rendering by specifying a Painter
   * object, which can set advanced effects like shadows, font styling,
   * fill styles, and more. @see Painter for more details.
   *
   * Value: Painter pointer, default nullptr (use ForegroundColor).
   *
   * When set, this painter s foreground properties set in the Style
   * object, including foreground color, font size, bold, italic, and shadows.
   */
 public:
  Painter* GetForegroundPainter() const;
  void SetForegroundPainter(Painter* val);

  /**
   * @brief Custom painter for text background rendering.
   *
   * Allows complete control over text background rendering by specifying a
   * Painter object, which can set advanced effects like shadows, fill styles,
   * and more. @see Painter for more details.
   *
   * Value: Painter pointer, default nullptr (use BackgroundColor).
   *
   * When set, this painter s background properties set in the Style
   * object, including background color.
   */
 public:
  Painter* GetBackgroundPainter() const;
  void SetBackgroundPainter(Painter* val);

  /**
   * @brief Controls word breaking behavior for line wrapping.
   *
   * Determines how words should be broken when text needs to wrap to the
   * next line.
   *
   * Value: See WordBreakType enum, default WordBreakType::kNormal.
   * - kNormal: Breaks words at normal word boundaries.
   * - kBreakAll: Breaks words anywhere to prevent overflow.
   * - kKeepAll: Keeps words together as a single unit.
   *
   * Interacts with ParagraphStyle::OverflowWrap settings for complete control.
   */
 public:
  WordBreakType GetWordBreak() const;
  void SetWordBreak(const WordBreakType& val);

  /**
   * @brief Vertical offset from the normal baseline position.
   *
   * Shifts text vertically from its normal baseline position, useful for
   * fine-tuning vertical alignment or creating special effects.
   *
   * Value: Float value in pixels, default 0.0f.
   */
 public:
  float GetBaselineOffset() const;
  void SetBaselineOffset(const float& val);

  /**
   * @brief Horizontal skew applied during text rendering.
   *
   * Value: Float skew factor, default 0.0f.
   */
 public:
  float GetTextSkew() const;
  void SetTextSkew(const float& val);

  /**
   * @brief Text shadow effects.
   *
   * Enables multiple shadow effects on text, each with independent offset,
   * blur radius, and color. Shadows are drawn in the order they appear in
   * the list.
   *
   * Value: std::vector<TextShadow>, default empty vector (no shadows).
   *
   * @see TextShadow struct for individual shadow properties.
   */
 public:
  const TextShadowList& GetTextShadowList() const;
  void SetTextShadowList(const TextShadowList& val);

 public:
  float GetScaledTextSize() const;
  const ShapeStyle& GetShapeStyle() const;
  bool HasAttribute(const AttributeType type) const;
  bool HasStyleAttribute(const AttrType style_flag) const;

 public:
  static constexpr AttrType FontDescriptorFlag = 1u << (kFontDescriptor);
  static constexpr AttrType TextSizeFlag = 1u << (kTextSize);
  static constexpr AttrType TextScaleFlag = 1u << (kTextScale);
  static constexpr AttrType ForegroundColorFlag = 1u << (kForegroundColor);
  static constexpr AttrType BackgroundColorFlag = 1u << (kBackgroundColor);
  static constexpr AttrType DecorationColorFlag = 1u << (kDecorationColor);
  static constexpr AttrType DecorationTypeFlag = 1u << (kDecorationType);
  static constexpr AttrType DecorationStyleFlag = 1u << (kDecorationStyle);
  static constexpr AttrType DecorationThicknessMultiplierFlag =
      1u << (kDecorationThicknessMultiplier);
  static constexpr AttrType DecorationElementLengthFlag =
      1u << (kDecorationElementLength);
  static constexpr AttrType DecorationGapLengthFlag = 1u
                                                      << (kDecorationGapLength);
  static constexpr AttrType DecorationSideMarginFlag =
      1u << (kDecorationSideMargin);
  static constexpr AttrType TextStrokeStyleFlag = 1u << (kTextStrokeStyle);
  static constexpr AttrType BoldFlag = 1u << (kBold);
  static constexpr AttrType ItalicFlag = 1u << (kItalic);
  static constexpr AttrType VerticalAlignmentFlag = 1u << (kVerticalAlignment);
  static constexpr AttrType WordSpacingFlag = 1u << (kWordSpacing);
  static constexpr AttrType LetterSpacingFlag = 1u << (kLetterSpacing);
  static constexpr AttrType ForegroundPainterFlag = 1u << (kForegroundPainter);
  static constexpr AttrType BackgroundPainterFlag = 1u << (kBackgroundPainter);
  static constexpr AttrType WordBreakFlag = 1u << (kWordBreak);
  static constexpr AttrType BaselineOffsetFlag = 1u << (kBaselineOffset);
  static constexpr AttrType TextSkewFlag = 1u << (kTextSkew);
  static constexpr FlagType TextShadowListFlag = 1u << (kTextShadowList);

 public:
  static constexpr FlagType FullFlag = (1u << kMaxAttrType) - 1;
  static constexpr FlagType SubSupFlag =
      TextSizeFlag | TextScaleFlag | VerticalAlignmentFlag;
  static constexpr FlagType MeasureFlag =
      FontDescriptorFlag | SubSupFlag | LetterSpacingFlag;
  static constexpr FlagType LayoutFlag =
      MeasureFlag | BoldFlag | ItalicFlag | TextSkewFlag;
  static constexpr FlagType DecorationFlag =
      DecorationColorFlag | DecorationStyleFlag | DecorationTypeFlag |
      DecorationThicknessMultiplierFlag | DecorationElementLengthFlag |
      DecorationGapLengthFlag | DecorationSideMarginFlag;

 private:
  std::unique_ptr<StyleImpl> impl_;

 public:
  static const Style& DefaultStyle();
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_STYLE_H_
