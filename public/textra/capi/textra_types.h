// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_TYPES_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_TYPES_H_

#include <textra/capi/textra_common.h>
#include <textra/rendering_backend.h>

/* Fixed-width types and explicit constants, usable from C11. Types with a
 * corresponding C++ enum share its width and numeric values. Published widths
 * and values must not change; add new values without renumbering old ones. */

typedef uint8_t textra_paragraph_type_t;
#define TEXTRA_PARAGRAPH_TYPE_GENERAL ((textra_paragraph_type_t)0)
#define TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE ((textra_paragraph_type_t)1)

typedef uint8_t textra_layout_mode_t;
#define TEXTRA_LAYOUT_MODE_INDEFINITE ((textra_layout_mode_t)0)
#define TEXTRA_LAYOUT_MODE_DEFINITE ((textra_layout_mode_t)1)
#define TEXTRA_LAYOUT_MODE_AT_MOST ((textra_layout_mode_t)2)

typedef uint8_t textra_layout_flow_t;
#define TEXTRA_LAYOUT_FLOW_NORMAL ((textra_layout_flow_t)0)
#define TEXTRA_LAYOUT_FLOW_RELAYOUT_REGION ((textra_layout_flow_t)1)
#define TEXTRA_LAYOUT_FLOW_RELAYOUT_LINE ((textra_layout_flow_t)2)
#define TEXTRA_LAYOUT_FLOW_BREAK_REGION ((textra_layout_flow_t)3)
#define TEXTRA_LAYOUT_FLOW_BREAK_COLUMN ((textra_layout_flow_t)4)
#define TEXTRA_LAYOUT_FLOW_BREAK_LINE ((textra_layout_flow_t)5)

typedef uint8_t textra_horizontal_alignment_t;
#define TEXTRA_HORIZONTAL_ALIGNMENT_LEFT ((textra_horizontal_alignment_t)0)
#define TEXTRA_HORIZONTAL_ALIGNMENT_CENTER ((textra_horizontal_alignment_t)1)
#define TEXTRA_HORIZONTAL_ALIGNMENT_RIGHT ((textra_horizontal_alignment_t)2)
#define TEXTRA_HORIZONTAL_ALIGNMENT_JUSTIFY ((textra_horizontal_alignment_t)3)
#define TEXTRA_HORIZONTAL_ALIGNMENT_DISTRIBUTED \
  ((textra_horizontal_alignment_t)4)
#define TEXTRA_HORIZONTAL_ALIGNMENT_START ((textra_horizontal_alignment_t)5)
#define TEXTRA_HORIZONTAL_ALIGNMENT_END ((textra_horizontal_alignment_t)6)

typedef uint8_t textra_vertical_alignment_t;
#define TEXTRA_VERTICAL_ALIGNMENT_TOP ((textra_vertical_alignment_t)0)
#define TEXTRA_VERTICAL_ALIGNMENT_CENTER ((textra_vertical_alignment_t)1)
#define TEXTRA_VERTICAL_ALIGNMENT_BOTTOM ((textra_vertical_alignment_t)2)

typedef uint8_t textra_character_alignment_t;
#define TEXTRA_CHARACTER_ALIGNMENT_TOP ((textra_character_alignment_t)0)
#define TEXTRA_CHARACTER_ALIGNMENT_TEXT_TOP ((textra_character_alignment_t)1)
#define TEXTRA_CHARACTER_ALIGNMENT_SUPER_SCRIPT \
  ((textra_character_alignment_t)2)
#define TEXTRA_CHARACTER_ALIGNMENT_MIDDLE ((textra_character_alignment_t)3)
#define TEXTRA_CHARACTER_ALIGNMENT_BASELINE ((textra_character_alignment_t)4)
#define TEXTRA_CHARACTER_ALIGNMENT_SUB_SCRIPT ((textra_character_alignment_t)5)
#define TEXTRA_CHARACTER_ALIGNMENT_TEXT_BOTTOM ((textra_character_alignment_t)6)
#define TEXTRA_CHARACTER_ALIGNMENT_BOTTOM ((textra_character_alignment_t)7)

typedef uint8_t textra_write_direction_t;
#define TEXTRA_WRITE_DIRECTION_AUTO ((textra_write_direction_t)0)
#define TEXTRA_WRITE_DIRECTION_LTR ((textra_write_direction_t)1)
#define TEXTRA_WRITE_DIRECTION_RTL ((textra_write_direction_t)2)
#define TEXTRA_WRITE_DIRECTION_TTB ((textra_write_direction_t)3)
#define TEXTRA_WRITE_DIRECTION_BTT ((textra_write_direction_t)4)

typedef uint8_t textra_line_height_rule_t;
#define TEXTRA_LINE_HEIGHT_RULE_AT_LEAST ((textra_line_height_rule_t)0)
#define TEXTRA_LINE_HEIGHT_RULE_AUTO ((textra_line_height_rule_t)1)
#define TEXTRA_LINE_HEIGHT_RULE_EXACT ((textra_line_height_rule_t)2)

typedef uint8_t textra_line_height_trim_t;
#define TEXTRA_LINE_HEIGHT_TRIM_NONE ((textra_line_height_trim_t)0)
#define TEXTRA_LINE_HEIGHT_TRIM_FIRST_LINE_TOP ((textra_line_height_trim_t)1)
#define TEXTRA_LINE_HEIGHT_TRIM_LAST_LINE_BOTTOM ((textra_line_height_trim_t)2)
#define TEXTRA_LINE_HEIGHT_TRIM_BOTH ((textra_line_height_trim_t)3)

typedef uint8_t textra_inline_alignment_mode_t;
#define TEXTRA_INLINE_ALIGNMENT_MODE_LEGACY ((textra_inline_alignment_mode_t)0)
#define TEXTRA_INLINE_ALIGNMENT_MODE_LINE_BOX \
  ((textra_inline_alignment_mode_t)1)

typedef uint8_t textra_word_break_t;
#define TEXTRA_WORD_BREAK_NORMAL ((textra_word_break_t)0)
#define TEXTRA_WORD_BREAK_BREAK_ALL ((textra_word_break_t)1)
#define TEXTRA_WORD_BREAK_KEEP_ALL ((textra_word_break_t)2)

typedef uint8_t textra_overflow_wrap_t;
#define TEXTRA_OVERFLOW_WRAP_NORMAL ((textra_overflow_wrap_t)0)
#define TEXTRA_OVERFLOW_WRAP_ANYWHERE ((textra_overflow_wrap_t)1)
#define TEXTRA_OVERFLOW_WRAP_BREAK_WORD ((textra_overflow_wrap_t)2)

typedef uint8_t textra_dominant_baseline_t;
#define TEXTRA_DOMINANT_BASELINE_ALPHABETIC ((textra_dominant_baseline_t)0)
#define TEXTRA_DOMINANT_BASELINE_TOP ((textra_dominant_baseline_t)1)
#define TEXTRA_DOMINANT_BASELINE_MIDDLE ((textra_dominant_baseline_t)2)
#define TEXTRA_DOMINANT_BASELINE_BOTTOM ((textra_dominant_baseline_t)3)
#define TEXTRA_DOMINANT_BASELINE_IDEOGRAPHIC ((textra_dominant_baseline_t)4)
#define TEXTRA_DOMINANT_BASELINE_HANGING ((textra_dominant_baseline_t)5)

typedef uint8_t textra_line_style_t;
#define TEXTRA_LINE_STYLE_NONE ((textra_line_style_t)0)
#define TEXTRA_LINE_STYLE_HIDDEN ((textra_line_style_t)1)
#define TEXTRA_LINE_STYLE_SOLID ((textra_line_style_t)2)
#define TEXTRA_LINE_STYLE_DOTTED ((textra_line_style_t)3)
#define TEXTRA_LINE_STYLE_DASHED ((textra_line_style_t)4)
#define TEXTRA_LINE_STYLE_DOUBLE ((textra_line_style_t)5)
#define TEXTRA_LINE_STYLE_INSET ((textra_line_style_t)6)
#define TEXTRA_LINE_STYLE_OUTSET ((textra_line_style_t)7)
#define TEXTRA_LINE_STYLE_GROOVE ((textra_line_style_t)8)
#define TEXTRA_LINE_STYLE_RIDGE ((textra_line_style_t)9)
#define TEXTRA_LINE_STYLE_WAVY ((textra_line_style_t)10)

/* Named font styles share the C++ FontStyle values. Weight also accepts
 * intermediate values in 1..1000; these constants do not restrict that range.
 * Keep the existing font_style record field widths unchanged. */
#define TEXTRA_FONT_WEIGHT_INVISIBLE ((int32_t)0)
#define TEXTRA_FONT_WEIGHT_THIN ((int32_t)100)
#define TEXTRA_FONT_WEIGHT_EXTRA_LIGHT ((int32_t)200)
#define TEXTRA_FONT_WEIGHT_LIGHT ((int32_t)300)
#define TEXTRA_FONT_WEIGHT_NORMAL ((int32_t)400)
#define TEXTRA_FONT_WEIGHT_MEDIUM ((int32_t)500)
#define TEXTRA_FONT_WEIGHT_SEMI_BOLD ((int32_t)600)
#define TEXTRA_FONT_WEIGHT_BOLD ((int32_t)700)
#define TEXTRA_FONT_WEIGHT_EXTRA_BOLD ((int32_t)800)
#define TEXTRA_FONT_WEIGHT_BLACK ((int32_t)900)
#define TEXTRA_FONT_WEIGHT_EXTRA_BLACK ((int32_t)1000)

#define TEXTRA_FONT_WIDTH_ULTRA_CONDENSED ((uint32_t)1)
#define TEXTRA_FONT_WIDTH_EXTRA_CONDENSED ((uint32_t)2)
#define TEXTRA_FONT_WIDTH_CONDENSED ((uint32_t)3)
#define TEXTRA_FONT_WIDTH_SEMI_CONDENSED ((uint32_t)4)
#define TEXTRA_FONT_WIDTH_NORMAL ((uint32_t)5)
#define TEXTRA_FONT_WIDTH_SEMI_EXPANDED ((uint32_t)6)
#define TEXTRA_FONT_WIDTH_EXPANDED ((uint32_t)7)
#define TEXTRA_FONT_WIDTH_EXTRA_EXPANDED ((uint32_t)8)
#define TEXTRA_FONT_WIDTH_ULTRA_EXPANDED ((uint32_t)9)

typedef uint8_t textra_font_slant_t;
#define TEXTRA_FONT_SLANT_UPRIGHT ((textra_font_slant_t)0)
#define TEXTRA_FONT_SLANT_ITALIC ((textra_font_slant_t)1)
#define TEXTRA_FONT_SLANT_OBLIQUE ((textra_font_slant_t)2)

typedef uint32_t textra_font_manager_slot_t;
#define TEXTRA_FONT_MANAGER_SLOT_DEFAULT ((textra_font_manager_slot_t)0)
#define TEXTRA_FONT_MANAGER_SLOT_ASSET ((textra_font_manager_slot_t)1)
#define TEXTRA_FONT_MANAGER_SLOT_DYNAMIC ((textra_font_manager_slot_t)2)
#define TEXTRA_FONT_MANAGER_SLOT_TEST ((textra_font_manager_slot_t)3)

typedef uint8_t textra_feature_t;
#define TEXTRA_FEATURE_LAST_LINE_CAN_OVERFLOW ((textra_feature_t)0)
#define TEXTRA_FEATURE_SKIP_SPACING_BEFORE_FIRST_LINE ((textra_feature_t)1)
#define TEXTRA_FEATURE_TRIM_LINE_TAIL_SPACE ((textra_feature_t)2)
#define TEXTRA_FEATURE_HARMONY_SHAPER_FORCE_LOW_API ((textra_feature_t)3)
#define TEXTRA_FEATURE_SYSTEM_FONT_ADJUST ((textra_feature_t)4)
#define TEXTRA_FEATURE_DISABLE_SHAPE_CACHE ((textra_feature_t)5)
#define TEXTRA_FEATURE_ANDROID_METRICS_ROUND ((textra_feature_t)6)

typedef uint8_t textra_punctuation_type_t;
#define TEXTRA_PUNCTUATION_TYPE_NONE ((textra_punctuation_type_t)0)
#define TEXTRA_PUNCTUATION_TYPE_OPEN ((textra_punctuation_type_t)1)
#define TEXTRA_PUNCTUATION_TYPE_CLOSE ((textra_punctuation_type_t)2)
#define TEXTRA_PUNCTUATION_TYPE_CENTER ((textra_punctuation_type_t)3)

typedef uint8_t textra_gradient_type_t;
#define TEXTRA_GRADIENT_TYPE_LINEAR ((textra_gradient_type_t)1)
#define TEXTRA_GRADIENT_TYPE_RADIAL ((textra_gradient_type_t)2)
#define TEXTRA_GRADIENT_TYPE_CONIC ((textra_gradient_type_t)3)

typedef uint8_t textra_decoration_t;
#define TEXTRA_DECORATION_NONE ((textra_decoration_t)0)
#define TEXTRA_DECORATION_UNDERLINE ((textra_decoration_t)1)
#define TEXTRA_DECORATION_OVERLINE ((textra_decoration_t)2)
#define TEXTRA_DECORATION_LINE_THROUGH ((textra_decoration_t)4)

typedef uint8_t textra_punctuation_compress_t;
#define TEXTRA_PUNCTUATION_COMPRESS_NONE ((textra_punctuation_compress_t)0)
#define TEXTRA_PUNCTUATION_COMPRESS_ALL ((textra_punctuation_compress_t)1)
#define TEXTRA_PUNCTUATION_COMPRESS_LINE_EDGE ((textra_punctuation_compress_t)2)
#define TEXTRA_PUNCTUATION_COMPRESS_ADJACENT ((textra_punctuation_compress_t)4)

/* Frozen records: pass pointers, never extend these structures. */
typedef struct textra_point {
  float x;
  float y;
} textra_point_t;

typedef struct textra_rect {
  float left;
  float top;
  float right;
  float bottom;
} textra_rect_t;

typedef struct textra_text_range {
  uint32_t start;
  uint32_t end;
} textra_text_range_t;

typedef struct textra_font_style {
  int32_t weight; /* 1..1000; normal = 400. */
  uint32_t width; /* 1..9; normal = 5. */
  textra_font_slant_t slant;
} textra_font_style_t;

/* Extensible output; ascent/descent are nonnegative distances from baseline.
 * Baseline is measured from the first line top; descent spans to the last
 * line bottom. Query requires a successful, still-valid layout. */
typedef struct textra_paragraph_metrics {
  uint32_t struct_size;
  float width;
  float height;
  float baseline;
  float ascent;
  float descent;
} textra_paragraph_metrics_t;

/* No packed bitfields, C++ classes, or platform font pointers in these records.
 */

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_TYPES_H_
