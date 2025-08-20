// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_STYLE_H_
#define PUBLIC_TEXTRA_STYLE_H_

#include <textra/font_info.h>
#include <textra/layout_definition.h>
#include <textra/macro.h>
#include <textra/painter.h>

#include <memory>
#include <vector>

namespace ttoffice {
namespace tttext {
class BaseRun;
class GhostRun;
class ShapeStyle;
class StyleManager;

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
  /**
   * Define new attributes using this macro, no need to write all Getter/Setter,
   * note that DefaultStyle and constructor initialization need to be adapted
   */

  using FlagType = AttrType;
  using TextShadowList = std::vector<TextShadow>;

#define DEFINE_STYLE_ATTRIBUTE(TYPE_NAME, VAR_NAME, TYPE)           \
 public:                                                            \
  static constexpr FlagType TYPE_NAME##Flag() {                     \
    return 1u << (int32_t)AttributeType::k##TYPE_NAME;              \
  }                                                                 \
  bool Has##TYPE_NAME() const { return flag_ & TYPE_NAME##Flag(); } \
  const TYPE& Get##TYPE_NAME() const {                              \
    return Has##TYPE_NAME() ? (VAR_NAME) : DefaultStyle().VAR_NAME; \
  }                                                                 \
  void Set##TYPE_NAME(const TYPE& val) {                            \
    VAR_NAME = val;                                                 \
    flag_ |= TYPE_NAME##Flag();                                     \
    if (Style::LayoutFlag() & TYPE_NAME##Flag()) ClearShapeStyle(); \
  }                                                                 \
                                                                    \
 private:                                                           \
  TYPE VAR_NAME = DefaultStyle().VAR_NAME;

#define DEFINE_STYLE_ATTRIBUTE_PTR(TYPE_NAME, VAR_NAME, TYPE)       \
 public:                                                            \
  static constexpr FlagType TYPE_NAME##Flag() {                     \
    return 1u << (int32_t)AttributeType::k##TYPE_NAME;              \
  }                                                                 \
  bool Has##TYPE_NAME() const { return flag_ & TYPE_NAME##Flag(); } \
  TYPE Get##TYPE_NAME() const { return VAR_NAME; }                  \
  void Set##TYPE_NAME(TYPE val) {                                   \
    VAR_NAME = val;                                                 \
    flag_ |= TYPE_NAME##Flag();                                     \
  }                                                                 \
                                                                    \
 private:                                                           \
  TYPE VAR_NAME;

 public:
  Style();
  Style(const Style& style);
  explicit Style(FontDescriptor font_descriptor, float text_size,
                 float text_scale, const TTColor& fg_color,
                 const TTColor& bg_color, const TTColor& decoration_color,
                 DecorationType decoration_type, LineType decoration_style,
                 float decoration_thickness_multiplier, bool bold, bool italic,
                 CharacterVerticalAlignment vertical_alignment,
                 float word_spacing, float letter_spacing,
                 const TextShadowList& text_shadow_lists,
                 tttext::Painter* foreground_painter,
                 tttext::Painter* background_painter, WordBreakType word_break,
                 float baseline_offset, uint32_t flag);
  ~Style();

 public:
  void Reset() { flag_ = 0; }
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
  DEFINE_STYLE_ATTRIBUTE(FontDescriptor, font_descriptor_, FontDescriptor)

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
  DEFINE_STYLE_ATTRIBUTE(TextSize, text_size_, float)

  /**
   * @brief Scaling factor applied to the base font size.
   *
   * Multiplier applied to TextSize to achieve the final rendered font size.
   * Useful for proportional scaling without changing the base font size.
   *
   * Value: Positive float (typically 0.1f to 10.0f), default 1.0f.
   */
  DEFINE_STYLE_ATTRIBUTE(TextScale, text_scale_, float)

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
  DEFINE_STYLE_ATTRIBUTE(ForegroundColor, fg_color_, TTColor)

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
  DEFINE_STYLE_ATTRIBUTE(BackgroundColor, bg_color_, TTColor)

  /**
   * @brief Color for text decorations (underline, strikethrough, overline).
   *
   * Specifies the color used for drawing text decorations.
   *
   * Value: TTColor in ARGB format, default TTColor::UNDEFINED() (transparent).
   */
  DEFINE_STYLE_ATTRIBUTE(DecorationColor, decoration_color_, TTColor)

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
  DEFINE_STYLE_ATTRIBUTE(DecorationType, decoration_type_, DecorationType)

  /**
   * @brief Controls the visual appearance of text decoration lines (solid,
   * dashed, dotted, etc.).
   *
   * Value: See LineType enum, default LineType::kSolid.
   *
   * @note Currently only kSolid and kDashed are implemented for text
   * decorations. Other styles are ignored and no decoration is drawn.
   */
  DEFINE_STYLE_ATTRIBUTE(DecorationStyle, decoration_style_, LineType)

  /**
   * @brief Multiplier for decoration line thickness.
   *
   * Scales the default thickness of decoration lines. Useful for creating
   * thicker or thinner decorations relative to the font size.
   *
   * Value: Positive float (typically 0.1f to 5.0f), default 1.0f.
   */
  DEFINE_STYLE_ATTRIBUTE(DecorationThicknessMultiplier,
                         decoration_thickness_multiplier_, float)

  /**
   * @brief Enables bold text rendering.
   *
   * Value: Boolean, default false.
   *
   * @note Bold formatting does not affect font selection/shaping and only
   * applies fake bold effects during rendering. Use FontDescriptor with
   * FontStyle::Bold() for proper font selection.
   */
  DEFINE_STYLE_ATTRIBUTE(Bold, bold_, bool)

  /**
   * @brief Enables italic text rendering.
   *
   * Value: Boolean, default false.
   *
   * @note Italic formatting does not affect font selection/shaping and only
   * applies fake italic effects during rendering. Use FontDescriptor with
   * FontStyle::Italic() for proper font selection.
   */
  DEFINE_STYLE_ATTRIBUTE(Italic, italic_, bool)

  /**
   * @brief Vertical alignment of text relative to the baseline.
   *
   * Controls how text is positioned vertically, particularly useful for
   * superscript, subscript, and mixed-size text alignment.
   *
   * Value: See CharacterVerticalAlignment enum, default kBaseLine.
   */
  DEFINE_STYLE_ATTRIBUTE(VerticalAlignment, vertical_alignment_,
                         CharacterVerticalAlignment)

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
  DEFINE_STYLE_ATTRIBUTE(WordSpacing, word_spacing_, float)

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
  DEFINE_STYLE_ATTRIBUTE(LetterSpacing, letter_spacing_, float)

  /**
   * @brief Custom painter for text foreground rendering.
   *
   * Allows complete control over text rendering by specifying a Painter
   * object, which can set advanced effects like shadows, font styling,
   * fill styles, and more. @see Painter for more details.
   *
   * Value: Painter pointer, default nullptr (use ForegroundColor).
   *
   * When set, this painter overrides foreground properties set in the Style
   * object, including foreground color, font size, bold, italic, and shadows.
   */
  DEFINE_STYLE_ATTRIBUTE_PTR(ForegroundPainter, fg_painter_, Painter*)

  /**
   * @brief Custom painter for text background rendering.
   *
   * Allows complete control over text background rendering by specifying a
   * Painter object, which can set advanced effects like shadows, fill styles,
   * and more. @see Painter for more details.
   *
   * Value: Painter pointer, default nullptr (use BackgroundColor).
   *
   * When set, this painter overrides background properties set in the Style
   * object, including background color.
   */
  DEFINE_STYLE_ATTRIBUTE_PTR(BackgroundPainter, bg_painter_, Painter*)

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
  DEFINE_STYLE_ATTRIBUTE(WordBreak, word_break_, WordBreakType)

  /**
   * @brief Vertical offset from the normal baseline position.
   *
   * Shifts text vertically from its normal baseline position, useful for
   * fine-tuning vertical alignment or creating special effects.
   *
   * Value: Float value in pixels, default 0.0f.
   */
  DEFINE_STYLE_ATTRIBUTE(BaselineOffset, baseline_offset_, float);

 public:
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
  static constexpr FlagType TextShadowListFlag() {
    return 1u << (int32_t)AttributeType::kTextShadowList;
  }
  bool HasTextShadowList() const { return flag_ & TextShadowListFlag(); }
  const TextShadowList& GetTextShadowList() const { return text_shadow_list_; }
  void SetTextShadowList(const TextShadowList& val) {
    text_shadow_list_ = val;
    flag_ |= TextShadowListFlag();
  }

 private:
  TextShadowList text_shadow_list_;

 public:
  float GetScaledTextSize() const { return GetTextSize() * GetTextScale(); }
  const ShapeStyle& GetShapeStyle() const;
  bool HasAttribute(AttributeType type) const {
    return flag_ & (1u << (int32_t)type);
  }

 public:
  static constexpr FlagType FullFlag() {
    return (1u << (AttrType)AttributeType::kMaxAttrType) - 1;
  }
  static constexpr FlagType SubSupFlag() {
    return TextSizeFlag() | TextScaleFlag() | VerticalAlignmentFlag();
  }
  static constexpr FlagType MeasureFlag() {
    return FontDescriptorFlag() | SubSupFlag() | LetterSpacingFlag();
  }
  static constexpr FlagType LayoutFlag() {
    return MeasureFlag() | BoldFlag() | ItalicFlag();
  }

  static constexpr FlagType DecorationFlag() {
    return DecorationColorFlag() | DecorationStyleFlag() |
           DecorationTypeFlag() | DecorationThicknessMultiplierFlag();
  }

 public:
  Style& operator=(const Style& other);

 private:
  FlagType flag_;
  mutable std::unique_ptr<ShapeStyle> shape_style_{};

 public:
  static const Style& DefaultStyle();
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_STYLE_H_
