// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_CAPI_TEXTRA_ENUM_CHECKS_H_
#define SRC_CAPI_TEXTRA_ENUM_CHECKS_H_

#include <type_traits>

#include "src/capi/textra_internal.h"

namespace textra_capi {
// Sharing values permits direct casts. Keep every published value and width
// checked so a C++ enum edit cannot silently change the C ABI.

static_assert(
    std::is_same_v<textra_line_style_t, std::underlying_type_t<tt::LineType>>);
static_assert(TEXTRA_LINE_STYLE_NONE ==
              static_cast<textra_line_style_t>(tt::LineType::kNone));
static_assert(TEXTRA_LINE_STYLE_HIDDEN ==
              static_cast<textra_line_style_t>(tt::LineType::kHidden));
static_assert(TEXTRA_LINE_STYLE_SOLID ==
              static_cast<textra_line_style_t>(tt::LineType::kSolid));
static_assert(TEXTRA_LINE_STYLE_DOTTED ==
              static_cast<textra_line_style_t>(tt::LineType::kDotted));
static_assert(TEXTRA_LINE_STYLE_DASHED ==
              static_cast<textra_line_style_t>(tt::LineType::kDashed));
static_assert(TEXTRA_LINE_STYLE_DOUBLE ==
              static_cast<textra_line_style_t>(tt::LineType::kDouble));
static_assert(TEXTRA_LINE_STYLE_INSET ==
              static_cast<textra_line_style_t>(tt::LineType::kInset));
static_assert(TEXTRA_LINE_STYLE_OUTSET ==
              static_cast<textra_line_style_t>(tt::LineType::kOutset));
static_assert(TEXTRA_LINE_STYLE_GROOVE ==
              static_cast<textra_line_style_t>(tt::LineType::kGroove));
static_assert(TEXTRA_LINE_STYLE_RIDGE ==
              static_cast<textra_line_style_t>(tt::LineType::kRidge));
static_assert(TEXTRA_LINE_STYLE_WAVY ==
              static_cast<textra_line_style_t>(tt::LineType::kWavy));

static_assert(
    std::is_same_v<textra_character_alignment_t,
                   std::underlying_type_t<tt::CharacterVerticalAlignment>>);
static_assert(TEXTRA_CHARACTER_ALIGNMENT_TOP ==
              static_cast<textra_character_alignment_t>(
                  tt::CharacterVerticalAlignment::kTop));
static_assert(TEXTRA_CHARACTER_ALIGNMENT_TEXT_TOP ==
              static_cast<textra_character_alignment_t>(
                  tt::CharacterVerticalAlignment::kTextTop));
static_assert(TEXTRA_CHARACTER_ALIGNMENT_SUPER_SCRIPT ==
              static_cast<textra_character_alignment_t>(
                  tt::CharacterVerticalAlignment::kSuperScript));
static_assert(TEXTRA_CHARACTER_ALIGNMENT_MIDDLE ==
              static_cast<textra_character_alignment_t>(
                  tt::CharacterVerticalAlignment::kMiddle));
static_assert(TEXTRA_CHARACTER_ALIGNMENT_BASELINE ==
              static_cast<textra_character_alignment_t>(
                  tt::CharacterVerticalAlignment::kBaseLine));
static_assert(TEXTRA_CHARACTER_ALIGNMENT_SUB_SCRIPT ==
              static_cast<textra_character_alignment_t>(
                  tt::CharacterVerticalAlignment::kSubScript));
static_assert(TEXTRA_CHARACTER_ALIGNMENT_TEXT_BOTTOM ==
              static_cast<textra_character_alignment_t>(
                  tt::CharacterVerticalAlignment::kTextBottom));
static_assert(TEXTRA_CHARACTER_ALIGNMENT_BOTTOM ==
              static_cast<textra_character_alignment_t>(
                  tt::CharacterVerticalAlignment::kBottom));

static_assert(std::is_same_v<textra_word_break_t,
                             std::underlying_type_t<tt::WordBreakType>>);
static_assert(TEXTRA_WORD_BREAK_NORMAL ==
              static_cast<textra_word_break_t>(tt::WordBreakType::kNormal));
static_assert(TEXTRA_WORD_BREAK_BREAK_ALL ==
              static_cast<textra_word_break_t>(tt::WordBreakType::kBreakAll));
static_assert(TEXTRA_WORD_BREAK_KEEP_ALL ==
              static_cast<textra_word_break_t>(tt::WordBreakType::kKeepAll));

static_assert(
    std::is_same_v<textra_horizontal_alignment_t,
                   std::underlying_type_t<tt::ParagraphHorizontalAlignment>>);
static_assert(TEXTRA_HORIZONTAL_ALIGNMENT_LEFT ==
              static_cast<textra_horizontal_alignment_t>(
                  tt::ParagraphHorizontalAlignment::kLeft));
static_assert(TEXTRA_HORIZONTAL_ALIGNMENT_CENTER ==
              static_cast<textra_horizontal_alignment_t>(
                  tt::ParagraphHorizontalAlignment::kCenter));
static_assert(TEXTRA_HORIZONTAL_ALIGNMENT_RIGHT ==
              static_cast<textra_horizontal_alignment_t>(
                  tt::ParagraphHorizontalAlignment::kRight));
static_assert(TEXTRA_HORIZONTAL_ALIGNMENT_JUSTIFY ==
              static_cast<textra_horizontal_alignment_t>(
                  tt::ParagraphHorizontalAlignment::kJustify));
static_assert(TEXTRA_HORIZONTAL_ALIGNMENT_DISTRIBUTED ==
              static_cast<textra_horizontal_alignment_t>(
                  tt::ParagraphHorizontalAlignment::kDistributed));
static_assert(TEXTRA_HORIZONTAL_ALIGNMENT_START ==
              static_cast<textra_horizontal_alignment_t>(
                  tt::ParagraphHorizontalAlignment::kStart));
static_assert(TEXTRA_HORIZONTAL_ALIGNMENT_END ==
              static_cast<textra_horizontal_alignment_t>(
                  tt::ParagraphHorizontalAlignment::kEnd));

static_assert(
    std::is_same_v<textra_vertical_alignment_t,
                   std::underlying_type_t<tt::ParagraphVerticalAlignment>>);
static_assert(TEXTRA_VERTICAL_ALIGNMENT_TOP ==
              static_cast<textra_vertical_alignment_t>(
                  tt::ParagraphVerticalAlignment::kTop));
static_assert(TEXTRA_VERTICAL_ALIGNMENT_CENTER ==
              static_cast<textra_vertical_alignment_t>(
                  tt::ParagraphVerticalAlignment::kCenter));
static_assert(TEXTRA_VERTICAL_ALIGNMENT_BOTTOM ==
              static_cast<textra_vertical_alignment_t>(
                  tt::ParagraphVerticalAlignment::kBottom));

static_assert(std::is_same_v<textra_dominant_baseline_t,
                             std::underlying_type_t<tt::DominantBaseline>>);
static_assert(
    TEXTRA_DOMINANT_BASELINE_ALPHABETIC ==
    static_cast<textra_dominant_baseline_t>(tt::DominantBaseline::kAlphabetic));
static_assert(
    TEXTRA_DOMINANT_BASELINE_TOP ==
    static_cast<textra_dominant_baseline_t>(tt::DominantBaseline::kTop));
static_assert(
    TEXTRA_DOMINANT_BASELINE_MIDDLE ==
    static_cast<textra_dominant_baseline_t>(tt::DominantBaseline::kMiddle));
static_assert(
    TEXTRA_DOMINANT_BASELINE_BOTTOM ==
    static_cast<textra_dominant_baseline_t>(tt::DominantBaseline::kBottom));
static_assert(TEXTRA_DOMINANT_BASELINE_IDEOGRAPHIC ==
              static_cast<textra_dominant_baseline_t>(
                  tt::DominantBaseline::kIdeographic));
static_assert(
    TEXTRA_DOMINANT_BASELINE_HANGING ==
    static_cast<textra_dominant_baseline_t>(tt::DominantBaseline::kHanging));

static_assert(std::is_same_v<textra_write_direction_t,
                             std::underlying_type_t<tt::WriteDirection>>);
static_assert(TEXTRA_WRITE_DIRECTION_AUTO ==
              static_cast<textra_write_direction_t>(tt::WriteDirection::kAuto));
static_assert(TEXTRA_WRITE_DIRECTION_LTR ==
              static_cast<textra_write_direction_t>(tt::WriteDirection::kLTR));
static_assert(TEXTRA_WRITE_DIRECTION_RTL ==
              static_cast<textra_write_direction_t>(tt::WriteDirection::kRTL));
static_assert(TEXTRA_WRITE_DIRECTION_TTB ==
              static_cast<textra_write_direction_t>(tt::WriteDirection::kTTB));
static_assert(TEXTRA_WRITE_DIRECTION_BTT ==
              static_cast<textra_write_direction_t>(tt::WriteDirection::kBTT));

static_assert(std::is_same_v<textra_line_height_trim_t,
                             std::underlying_type_t<tt::LineHeightTrim>>);
static_assert(
    TEXTRA_LINE_HEIGHT_TRIM_NONE ==
    static_cast<textra_line_height_trim_t>(tt::LineHeightTrim::kNone));
static_assert(
    TEXTRA_LINE_HEIGHT_TRIM_FIRST_LINE_TOP ==
    static_cast<textra_line_height_trim_t>(tt::LineHeightTrim::kFirstLineTop));
static_assert(TEXTRA_LINE_HEIGHT_TRIM_LAST_LINE_BOTTOM ==
              static_cast<textra_line_height_trim_t>(
                  tt::LineHeightTrim::kLastLineBottom));
static_assert(
    TEXTRA_LINE_HEIGHT_TRIM_BOTH ==
    static_cast<textra_line_height_trim_t>(tt::LineHeightTrim::kBoth));

static_assert(
    std::is_same_v<textra_inline_alignment_mode_t,
                   std::underlying_type_t<tt::InlineVerticalAlignmentMode>>);
static_assert(TEXTRA_INLINE_ALIGNMENT_MODE_LEGACY ==
              static_cast<textra_inline_alignment_mode_t>(
                  tt::InlineVerticalAlignmentMode::kLegacy));
static_assert(TEXTRA_INLINE_ALIGNMENT_MODE_LINE_BOX ==
              static_cast<textra_inline_alignment_mode_t>(
                  tt::InlineVerticalAlignmentMode::kLineBox));

static_assert(std::is_same_v<textra_overflow_wrap_t,
                             std::underlying_type_t<tt::OverflowWrap>>);
static_assert(TEXTRA_OVERFLOW_WRAP_NORMAL ==
              static_cast<textra_overflow_wrap_t>(tt::OverflowWrap::kNormal));
static_assert(TEXTRA_OVERFLOW_WRAP_ANYWHERE ==
              static_cast<textra_overflow_wrap_t>(tt::OverflowWrap::kAnywhere));
static_assert(
    TEXTRA_OVERFLOW_WRAP_BREAK_WORD ==
    static_cast<textra_overflow_wrap_t>(tt::OverflowWrap::kBreakWord));

static_assert(std::is_same_v<textra_decoration_t,
                             std::underlying_type_t<tt::DecorationType>>);
static_assert(TEXTRA_DECORATION_NONE ==
              static_cast<textra_decoration_t>(tt::DecorationType::kNone));
static_assert(TEXTRA_DECORATION_UNDERLINE ==
              static_cast<textra_decoration_t>(tt::DecorationType::kUnderLine));
static_assert(TEXTRA_DECORATION_OVERLINE ==
              static_cast<textra_decoration_t>(tt::DecorationType::kOverline));
static_assert(
    TEXTRA_DECORATION_LINE_THROUGH ==
    static_cast<textra_decoration_t>(tt::DecorationType::kLineThrough));

static_assert(std::is_same_v<textra_paragraph_type_t,
                             std::underlying_type_t<tt::ParagraphType>>);
static_assert(
    TEXTRA_PARAGRAPH_TYPE_GENERAL ==
    static_cast<textra_paragraph_type_t>(tt::ParagraphType::kGeneral));
static_assert(
    TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE ==
    static_cast<textra_paragraph_type_t>(tt::ParagraphType::kSingleLine));

static_assert(std::is_same_v<textra_layout_mode_t,
                             std::underlying_type_t<tt::LayoutMode>>);
static_assert(TEXTRA_LAYOUT_MODE_INDEFINITE ==
              static_cast<textra_layout_mode_t>(tt::LayoutMode::kIndefinite));
static_assert(TEXTRA_LAYOUT_MODE_DEFINITE ==
              static_cast<textra_layout_mode_t>(tt::LayoutMode::kDefinite));
static_assert(TEXTRA_LAYOUT_MODE_AT_MOST ==
              static_cast<textra_layout_mode_t>(tt::LayoutMode::kAtMost));

static_assert(std::is_same_v<textra_layout_flow_t,
                             std::underlying_type_t<tt::LayoutResult>>);
static_assert(TEXTRA_LAYOUT_FLOW_NORMAL ==
              static_cast<textra_layout_flow_t>(tt::LayoutResult::kNormal));
static_assert(
    TEXTRA_LAYOUT_FLOW_RELAYOUT_REGION ==
    static_cast<textra_layout_flow_t>(tt::LayoutResult::kRelayoutPage));
static_assert(
    TEXTRA_LAYOUT_FLOW_RELAYOUT_LINE ==
    static_cast<textra_layout_flow_t>(tt::LayoutResult::kRelayoutLine));
static_assert(TEXTRA_LAYOUT_FLOW_BREAK_REGION ==
              static_cast<textra_layout_flow_t>(tt::LayoutResult::kBreakPage));
static_assert(
    TEXTRA_LAYOUT_FLOW_BREAK_COLUMN ==
    static_cast<textra_layout_flow_t>(tt::LayoutResult::kBreakColumn));
static_assert(TEXTRA_LAYOUT_FLOW_BREAK_LINE ==
              static_cast<textra_layout_flow_t>(tt::LayoutResult::kBreakLine));

static_assert(std::is_same_v<textra_line_height_rule_t,
                             std::underlying_type_t<tt::RulerType>>);
static_assert(TEXTRA_LINE_HEIGHT_RULE_AT_LEAST ==
              static_cast<textra_line_height_rule_t>(tt::RulerType::kAtLeast));
static_assert(TEXTRA_LINE_HEIGHT_RULE_AUTO ==
              static_cast<textra_line_height_rule_t>(tt::RulerType::kAuto));
static_assert(TEXTRA_LINE_HEIGHT_RULE_EXACT ==
              static_cast<textra_line_height_rule_t>(tt::RulerType::kExact));

static_assert(std::is_same_v<textra_feature_t,
                             std::underlying_type_t<tt::FeatureOption>>);
static_assert(
    TEXTRA_FEATURE_LAST_LINE_CAN_OVERFLOW ==
    static_cast<textra_feature_t>(tt::FeatureOption::kLastLineCanOverflow));
static_assert(TEXTRA_FEATURE_SKIP_SPACING_BEFORE_FIRST_LINE ==
              static_cast<textra_feature_t>(
                  tt::FeatureOption::kSkipSpaceBeforeFirstLine));
static_assert(
    TEXTRA_FEATURE_TRIM_LINE_TAIL_SPACE ==
    static_cast<textra_feature_t>(tt::FeatureOption::kTrimLineTailSpace));
static_assert(TEXTRA_FEATURE_HARMONY_SHAPER_FORCE_LOW_API ==
              static_cast<textra_feature_t>(
                  tt::FeatureOption::kHarmonyShaperForceUseLowAPI));
static_assert(
    TEXTRA_FEATURE_SYSTEM_FONT_ADJUST ==
    static_cast<textra_feature_t>(tt::FeatureOption::kSystemFontAdjust));
static_assert(
    TEXTRA_FEATURE_DISABLE_SHAPE_CACHE ==
    static_cast<textra_feature_t>(tt::FeatureOption::kDisableShapeCache));
static_assert(
    TEXTRA_FEATURE_ANDROID_METRICS_ROUND ==
    static_cast<textra_feature_t>(tt::FeatureOption::kAndroidMetricsRound));

static_assert(std::is_same_v<textra_punctuation_type_t,
                             std::underlying_type_t<tt::PunctuationType>>);
static_assert(
    TEXTRA_PUNCTUATION_TYPE_NONE ==
    static_cast<textra_punctuation_type_t>(tt::PunctuationType::kNone));
static_assert(
    TEXTRA_PUNCTUATION_TYPE_OPEN ==
    static_cast<textra_punctuation_type_t>(tt::PunctuationType::kOpen));
static_assert(
    TEXTRA_PUNCTUATION_TYPE_CLOSE ==
    static_cast<textra_punctuation_type_t>(tt::PunctuationType::kClose));
static_assert(
    TEXTRA_PUNCTUATION_TYPE_CENTER ==
    static_cast<textra_punctuation_type_t>(tt::PunctuationType::kCenter));

static_assert(
    std::is_same_v<textra_punctuation_compress_t,
                   std::underlying_type_t<tt::PunctuationCompressOption>>);
static_assert(TEXTRA_PUNCTUATION_COMPRESS_NONE ==
              static_cast<textra_punctuation_compress_t>(
                  tt::PunctuationCompressOption::kNone));
static_assert(TEXTRA_PUNCTUATION_COMPRESS_ALL ==
              static_cast<textra_punctuation_compress_t>(
                  tt::PunctuationCompressOption::kAll));
static_assert(TEXTRA_PUNCTUATION_COMPRESS_LINE_EDGE ==
              static_cast<textra_punctuation_compress_t>(
                  tt::PunctuationCompressOption::kLineEdge));
static_assert(TEXTRA_PUNCTUATION_COMPRESS_ADJACENT ==
              static_cast<textra_punctuation_compress_t>(
                  tt::PunctuationCompressOption::kAdjacent));

static_assert(std::is_same_v<textra_gradient_type_t,
                             std::underlying_type_t<tt::GradientType>>);
static_assert(TEXTRA_GRADIENT_TYPE_LINEAR ==
              static_cast<textra_gradient_type_t>(tt::GradientType::kLinear));
static_assert(TEXTRA_GRADIENT_TYPE_RADIAL ==
              static_cast<textra_gradient_type_t>(tt::GradientType::kRadial));
static_assert(TEXTRA_GRADIENT_TYPE_CONIC ==
              static_cast<textra_gradient_type_t>(tt::GradientType::kConic));

static_assert(std::is_same_v<textra_theme_category_t,
                             std::underlying_type_t<tt::ThemeCategory>>);
static_assert(TEXTRA_THEME_CATEGORY_NORMAL ==
              static_cast<textra_theme_category_t>(tt::ThemeCategory::kNormal));
static_assert(TEXTRA_THEME_CATEGORY_LINK ==
              static_cast<textra_theme_category_t>(tt::ThemeCategory::kLink));
static_assert(
    TEXTRA_THEME_CATEGORY_BACKGROUND ==
    static_cast<textra_theme_category_t>(tt::ThemeCategory::kBackground));
static_assert(TEXTRA_THEME_CATEGORY_BORDER ==
              static_cast<textra_theme_category_t>(tt::ThemeCategory::kBorder));
static_assert(
    TEXTRA_THEME_CATEGORY_FOOTNOTE ==
    static_cast<textra_theme_category_t>(tt::ThemeCategory::kFootnote));

static_assert(std::is_same_v<textra_font_slant_t, tt::Slant>);
static_assert(TEXTRA_FONT_WEIGHT_INVISIBLE == tt::FontStyle::kInvisible_Weight);
static_assert(TEXTRA_FONT_WEIGHT_THIN == tt::FontStyle::kThin_Weight);
static_assert(TEXTRA_FONT_WEIGHT_EXTRA_LIGHT ==
              tt::FontStyle::kExtraLight_Weight);
static_assert(TEXTRA_FONT_WEIGHT_LIGHT == tt::FontStyle::kLight_Weight);
static_assert(TEXTRA_FONT_WEIGHT_NORMAL == tt::FontStyle::kNormal_Weight);
static_assert(TEXTRA_FONT_WEIGHT_MEDIUM == tt::FontStyle::kMedium_Weight);
static_assert(TEXTRA_FONT_WEIGHT_SEMI_BOLD == tt::FontStyle::kSemiBold_Weight);
static_assert(TEXTRA_FONT_WEIGHT_BOLD == tt::FontStyle::kBold_Weight);
static_assert(TEXTRA_FONT_WEIGHT_EXTRA_BOLD ==
              tt::FontStyle::kExtraBold_Weight);
static_assert(TEXTRA_FONT_WEIGHT_BLACK == tt::FontStyle::kBlack_Weight);
static_assert(TEXTRA_FONT_WEIGHT_EXTRA_BLACK ==
              tt::FontStyle::kExtraBlack_Weight);
static_assert(TEXTRA_FONT_WIDTH_ULTRA_CONDENSED ==
              tt::FontStyle::kUltraCondensed_Width);
static_assert(TEXTRA_FONT_WIDTH_EXTRA_CONDENSED ==
              tt::FontStyle::kExtraCondensed_Width);
static_assert(TEXTRA_FONT_WIDTH_CONDENSED == tt::FontStyle::kCondensed_Width);
static_assert(TEXTRA_FONT_WIDTH_SEMI_CONDENSED ==
              tt::FontStyle::kSemiCondensed_Width);
static_assert(TEXTRA_FONT_WIDTH_NORMAL == tt::FontStyle::kNormal_Width);
static_assert(TEXTRA_FONT_WIDTH_SEMI_EXPANDED ==
              tt::FontStyle::kSemiExpanded_Width);
static_assert(TEXTRA_FONT_WIDTH_EXPANDED == tt::FontStyle::kExpanded_Width);
static_assert(TEXTRA_FONT_WIDTH_EXTRA_EXPANDED ==
              tt::FontStyle::kExtraExpanded_Width);
static_assert(TEXTRA_FONT_WIDTH_ULTRA_EXPANDED ==
              tt::FontStyle::kUltraExpanded_Width);
static_assert(TEXTRA_FONT_SLANT_UPRIGHT == tt::FontStyle::kUpright_Slant);
static_assert(TEXTRA_FONT_SLANT_ITALIC == tt::FontStyle::kItalic_Slant);
static_assert(TEXTRA_FONT_SLANT_OBLIQUE == tt::FontStyle::kOblique_Slant);
}  // namespace textra_capi
#endif  // SRC_CAPI_TEXTRA_ENUM_CHECKS_H_
