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

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace ttoffice {
namespace tttext {
class BaseRun;
class GhostRun;
class ShapeStyle;
class StyleManager;

union DecorationStyle {
  uint64_t value = 0;
  struct {
    uint32_t color_ : 32;
    uint16_t fixed_10x_thickness_ : 16;
    LineType line_type_ : 8;
    uint8_t padding_ : 8;
  } style_;
};

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
  using FlagType = AttrType;
  using TextShadowList = std::vector<TextShadow>;

 public:
  Style();
  Style(const Style& style);
  Style& operator=(const Style& other);
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
 public:
  const FontDescriptor& GetFontDescriptor() const { return font_descriptor_; }
  void SetFontDescriptor(const FontDescriptor& val) {
    font_descriptor_ = val;
    flag_ |= FontDescriptorFlag;
    ClearShapeStyle();
  }

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
  float GetTextSize() const { return text_size_; }
  void SetTextSize(const float& val) {
    text_size_ = val;
    flag_ |= TextSizeFlag;
    ClearShapeStyle();
  }

  /**
   * @brief Scaling factor applied to the base font size.
   *
   * Multiplier applied to TextSize to achieve the final rendered font size.
   * Useful for proportional scaling without changing the base font size.
   *
   * Value: Positive float (typically 0.1f to 10.0f), default 1.0f.
   */
 public:
  float GetTextScale() const { return text_scale_; }
  void SetTextScale(const float& val) {
    text_scale_ = val;
    flag_ |= TextScaleFlag;
    ClearShapeStyle();
  }

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
  TTColor GetForegroundColor() const { return fg_color_; }
  void SetForegroundColor(const TTColor& val) {
    fg_color_ = val;
    flag_ |= ForegroundColorFlag;
  }

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
  TTColor GetBackgroundColor() const { return bg_color_; }
  void SetBackgroundColor(const TTColor& val) {
    bg_color_ = val;
    flag_ |= BackgroundColorFlag;
  }

  /**
   * @brief Color for text decorations (underline, strikethrough, overline).
   *
   * Specifies the color used for drawing text decorations.
   *
   * Value: TTColor in ARGB format, default TTColor::UNDEFINED()
   * (transparent).
   */
 public:
  TTColor GetDecorationColor() const { return decoration_color_; }
  void SetDecorationColor(const TTColor& val) {
    decoration_color_ = val;
    flag_ |= DecorationColorFlag;
  }

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
  DecorationType GetDecorationType() const { return decoration_type_; }
  void SetDecorationType(const DecorationType& val) {
    decoration_type_ = val;
    flag_ |= DecorationTypeFlag;
  }

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
  LineType GetDecorationStyle() const { return decoration_style_; }
  void SetDecorationStyle(const LineType& val) {
    decoration_style_ = val;
    flag_ |= DecorationStyleFlag;
  }

  /**
   * @brief Multiplier for decoration line thickness.
   *
   * Scales the default thickness of decoration lines. Useful for creating
   * thicker or thinner decorations relative to the font size.
   *
   * Value: Positive float (typically 0.1f to 5.0f), default 1.0f.
   */
 public:
  float GetDecorationThicknessMultiplier() const {
    return decoration_thickness_multiplier_;
  }

  void SetDecorationThicknessMultiplier(const float& val) {
    decoration_thickness_multiplier_ = val;
    flag_ |= DecorationThicknessMultiplierFlag;
  }

  /**
   * @brief Text Stroke Style.
   *
   * Useful for setting text stroke style
   *
   * Value: color default TTColor::Undefined, thickness is positive float
   * (typically 0.1f to 5.0f), default 1.0f.
   */
 public:
  void SetTextStrokeStyle(const TTColor& color, float thickness) {
    text_stroke_.style_.color_ = color;
    if (std::isinf(thickness)) thickness = 0.0f;
    auto clamped_thickness =
        std::fmin(std::fmax(thickness * 10, 0.0f),
                  static_cast<float>(std::numeric_limits<uint16_t>::max()));
    text_stroke_.style_.fixed_10x_thickness_ = std::round(clamped_thickness);
    flag_ |= TextStrokeStyleFlag;
  }
  TTColor GetTextStrokeColor() const { return text_stroke_.style_.color_; }
  float GetTextStrokeWidth() const {
    return text_stroke_.style_.fixed_10x_thickness_ / 10.f;
  }
  uint64_t GetTextStrokeValue() const { return text_stroke_.value; }
  void SetTextStrokeValue(uint64_t val) {
    text_stroke_.value = val;
    flag_ |= TextStrokeStyleFlag;
  }

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
  bool GetBold() const { return bold_; }
  void SetBold(const bool& val) {
    bold_ = val;
    flag_ |= BoldFlag;
  }

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
  bool GetItalic() const { return italic_; }
  void SetItalic(const bool& val) {
    italic_ = val;
    flag_ |= ItalicFlag;
  }

  /**
   * @brief Vertical alignment of text relative to the baseline.
   *
   * Controls how text is positioned vertically, particularly useful for
   * superscript, subscript, and mixed-size text alignment.
   *
   * Value: See CharacterVerticalAlignment enum, default kBaseLine.
   */
 public:
  CharacterVerticalAlignment GetVerticalAlignment() const {
    return vertical_alignment_;
  }
  void SetVerticalAlignment(const CharacterVerticalAlignment& val) {
    vertical_alignment_ = val;
    flag_ |= VerticalAlignmentFlag;
    ClearShapeStyle();
  }

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
  float GetWordSpacing() const { return word_spacing_; }
  void SetWordSpacing(const float& val) {
    word_spacing_ = val;
    flag_ |= WordSpacingFlag;
  }

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
  float GetLetterSpacing() const { return letter_spacing_; }
  void SetLetterSpacing(const float& val) {
    letter_spacing_ = val;
    flag_ |= LetterSpacingFlag;
    ClearShapeStyle();
  }

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
  Painter* GetForegroundPainter() const { return fg_painter_; }
  void SetForegroundPainter(Painter* val) {
    fg_painter_ = val;
    flag_ |= ForegroundPainterFlag;
  }

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
  Painter* GetBackgroundPainter() const { return bg_painter_; }
  void SetBackgroundPainter(Painter* val) {
    bg_painter_ = val;
    flag_ |= BackgroundPainterFlag;
  }

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
  WordBreakType GetWordBreak() const { return word_break_; }
  void SetWordBreak(const WordBreakType& val) {
    word_break_ = val;
    flag_ |= WordBreakFlag;
  }

  /**
   * @brief Vertical offset from the normal baseline position.
   *
   * Shifts text vertically from its normal baseline position, useful for
   * fine-tuning vertical alignment or creating special effects.
   *
   * Value: Float value in pixels, default 0.0f.
   */
 public:
  float GetBaselineOffset() const { return baseline_offset_; }
  void SetBaselineOffset(const float& val) {
    baseline_offset_ = val;
    flag_ |= BaselineOffsetFlag;
  }

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
  const TextShadowList& GetTextShadowList() const { return text_shadow_list_; }
  void SetTextShadowList(const TextShadowList& val) {
    text_shadow_list_ = val;
    flag_ |= TextShadowListFlag;
  }

 public:
  float GetScaledTextSize() const { return GetTextSize() * GetTextScale(); }
  const ShapeStyle& GetShapeStyle() const;
  bool HasAttribute(const AttributeType type) const {
    return flag_ & (1u << type);
  }

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
  static constexpr FlagType TextShadowListFlag = 1u << (kTextShadowList);

 public:
  static constexpr FlagType FullFlag = (1u << kMaxAttrType) - 1;
  static constexpr FlagType SubSupFlag =
      TextSizeFlag | TextScaleFlag | VerticalAlignmentFlag;
  static constexpr FlagType MeasureFlag =
      FontDescriptorFlag | SubSupFlag | LetterSpacingFlag;
  static constexpr FlagType LayoutFlag = MeasureFlag | BoldFlag | ItalicFlag;
  static constexpr FlagType DecorationFlag =
      DecorationColorFlag | DecorationStyleFlag | DecorationTypeFlag |
      DecorationThicknessMultiplierFlag;

 private:
  FontDescriptor font_descriptor_{{}, FontStyle::Normal(), 0};
  float text_size_ = 10.f * 96 / 72;
  float text_scale_ = 1.0f;
  TTColor fg_color_ = TTColor::BLACK;
  TTColor bg_color_ = TTColor::UNDEFINED;
  TTColor decoration_color_ = TTColor::UNDEFINED;
  DecorationType decoration_type_ = DecorationType::kNone;
  LineType decoration_style_ = LineType::kSolid;
  float decoration_thickness_multiplier_ = 1.0f;
  DecorationStyle text_stroke_ = {
      .style_ = {TTColor::UNDEFINED, 10, LineType::kSolid}};
  bool bold_ = false;
  bool italic_ = false;
  CharacterVerticalAlignment vertical_alignment_ =
      CharacterVerticalAlignment::kBaseLine;
  float word_spacing_ = 0;
  float letter_spacing_ = 0;
  Painter* fg_painter_ = nullptr;
  Painter* bg_painter_ = nullptr;
  WordBreakType word_break_ = WordBreakType::kNormal;
  float baseline_offset_ = 0.f;
  TextShadowList text_shadow_list_{};
  FlagType flag_ = 0;
  mutable std::unique_ptr<ShapeStyle> shape_style_{};

 public:
  static const Style& DefaultStyle();
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_STYLE_H_
