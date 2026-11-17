// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_TYPES_H_
#define PUBLIC_TEXTRA_C_TEXTRA_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#define TEXTRA_CALL __cdecl
#define TEXTRA_API
#else
#define TEXTRA_CALL
#define TEXTRA_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
#define TEXTRA_EXTERN_C_BEGIN extern "C" {
#define TEXTRA_EXTERN_C_END }
#define TEXTRA_NOEXCEPT noexcept
#else
#define TEXTRA_EXTERN_C_BEGIN
#define TEXTRA_EXTERN_C_END
#define TEXTRA_NOEXCEPT
#endif

TEXTRA_EXTERN_C_BEGIN

/* Except for documented optional arguments, handles and out pointers must be
 * valid. Borrowed handles must not be destroyed. Callbacks must not throw.
 * Enum arguments must satisfy the corresponding C++ interface contract.
 * Boolean inputs treat zero as false and every nonzero value as true;
 * boolean outputs are 0 or 1.
 * Allocation failure follows the C++ runtime's fatal failure policy; no C++
 * exception crosses this ABI. All public value structs have fixed layouts. */
typedef struct textra_style textra_style_t;
typedef struct textra_paragraph_style textra_paragraph_style_t;
typedef struct textra_paragraph textra_paragraph_t;
typedef struct textra_fontmgr_collection textra_fontmgr_collection_t;
typedef struct textra_text_layout textra_text_layout_t;
typedef struct textra_tttext_context textra_tttext_context_t;
typedef struct textra_layout_region textra_layout_region_t;
typedef struct textra_text_line textra_text_line_t;
typedef struct textra_layout_drawer textra_layout_drawer_t;
typedef struct textra_font_manager textra_font_manager_t;
typedef struct textra_canvas_helper textra_canvas_helper_t;
typedef struct textra_buffer_output_stream textra_buffer_output_stream_t;
typedef struct textra_run_delegate textra_run_delegate_t;
typedef struct textra_layout_page_listener textra_layout_page_listener_t;
typedef struct textra_font_descriptor textra_font_descriptor_t;
typedef struct textra_typeface_helper textra_typeface_helper_t;
typedef struct textra_gradient textra_gradient_t;
typedef struct textra_resolved_gradient textra_resolved_gradient_t;

typedef int32_t textra_status_t;
enum {
  TEXTRA_STATUS_OK = 0,
  TEXTRA_STATUS_INVALID_ARGUMENT = 1,
  TEXTRA_STATUS_OUT_OF_RANGE = 3,
  TEXTRA_STATUS_UNSUPPORTED = 5,
  TEXTRA_STATUS_BUFFER_TOO_SMALL = 6,
  TEXTRA_STATUS_BACKEND_ERROR = 8
};

typedef uint32_t textra_attribute_type_t;
enum {
  TEXTRA_ATTRIBUTE_TYPE_LAYOUT_ATTR_START = 0,
  TEXTRA_ATTRIBUTE_TYPE_FONT_DESCRIPTOR = 0,
  TEXTRA_ATTRIBUTE_TYPE_TEXT_SIZE = 1,
  TEXTRA_ATTRIBUTE_TYPE_TEXT_SCALE = 2,
  TEXTRA_ATTRIBUTE_TYPE_VERTICAL_ALIGNMENT = 3,
  TEXTRA_ATTRIBUTE_TYPE_WORD_SPACING = 4,
  TEXTRA_ATTRIBUTE_TYPE_LETTER_SPACING = 5,
  TEXTRA_ATTRIBUTE_TYPE_LAYOUT_ATTR_END = 6,
  TEXTRA_ATTRIBUTE_TYPE_STYLE_MANAGER_ATTR_START = 6,
  TEXTRA_ATTRIBUTE_TYPE_FOREGROUND_COLOR = 6,
  TEXTRA_ATTRIBUTE_TYPE_BACKGROUND_COLOR = 7,
  TEXTRA_ATTRIBUTE_TYPE_DECORATION_COLOR = 8,
  TEXTRA_ATTRIBUTE_TYPE_DECORATION_TYPE = 9,
  TEXTRA_ATTRIBUTE_TYPE_DECORATION_STYLE = 10,
  TEXTRA_ATTRIBUTE_TYPE_DECORATION_THICKNESS_MULTIPLIER = 11,
  TEXTRA_ATTRIBUTE_TYPE_TEXT_STROKE_STYLE = 12,
  TEXTRA_ATTRIBUTE_TYPE_BOLD = 13,
  TEXTRA_ATTRIBUTE_TYPE_ITALIC = 14,
  TEXTRA_ATTRIBUTE_TYPE_TEXT_SHADOW_LIST = 15,
  TEXTRA_ATTRIBUTE_TYPE_FOREGROUND_PAINTER = 16,
  TEXTRA_ATTRIBUTE_TYPE_BACKGROUND_PAINTER = 17,
  TEXTRA_ATTRIBUTE_TYPE_WORD_BREAK = 18,
  TEXTRA_ATTRIBUTE_TYPE_BASELINE_OFFSET = 19,
  TEXTRA_ATTRIBUTE_TYPE_TEXT_SKEW = 20,
  TEXTRA_ATTRIBUTE_TYPE_DECORATION_ELEMENT_LENGTH = 21,
  TEXTRA_ATTRIBUTE_TYPE_DECORATION_GAP_LENGTH = 22,
  TEXTRA_ATTRIBUTE_TYPE_DECORATION_SIDE_MARGIN = 23,
  TEXTRA_ATTRIBUTE_TYPE_FOREGROUND_GRADIENT = 24,
  TEXTRA_ATTRIBUTE_TYPE_STYLE_MANAGER_ATTR_END = 25,
  TEXTRA_ATTRIBUTE_TYPE_MAX_ATTR_TYPE = 25,
  TEXTRA_ATTRIBUTE_TYPE_EXTRA_ATTR_START = 128,
  TEXTRA_ATTRIBUTE_TYPE_EXTRA_BASELINE_OFFSET = 129
};

typedef uint32_t textra_layout_result_t;
enum {
  TEXTRA_LAYOUT_RESULT_NORMAL = 0,
  TEXTRA_LAYOUT_RESULT_RELAYOUT_PAGE = 1,
  TEXTRA_LAYOUT_RESULT_RELAYOUT_LINE = 2,
  TEXTRA_LAYOUT_RESULT_BREAK_PAGE = 3,
  TEXTRA_LAYOUT_RESULT_BREAK_COLUMN = 4,
  TEXTRA_LAYOUT_RESULT_BREAK_LINE = 5,
  TEXTRA_LAYOUT_RESULT_UNSUPPORTED = 6
};

typedef uint32_t textra_paragraph_horizontal_alignment_t;
enum {
  TEXTRA_PARAGRAPH_HORIZONTAL_ALIGNMENT_LEFT = 0,
  TEXTRA_PARAGRAPH_HORIZONTAL_ALIGNMENT_CENTER = 1,
  TEXTRA_PARAGRAPH_HORIZONTAL_ALIGNMENT_RIGHT = 2,
  TEXTRA_PARAGRAPH_HORIZONTAL_ALIGNMENT_JUSTIFY = 3,
  TEXTRA_PARAGRAPH_HORIZONTAL_ALIGNMENT_DISTRIBUTED = 4,
  TEXTRA_PARAGRAPH_HORIZONTAL_ALIGNMENT_START = 5,
  TEXTRA_PARAGRAPH_HORIZONTAL_ALIGNMENT_END = 6
};

typedef uint32_t textra_paragraph_vertical_alignment_t;
enum {
  TEXTRA_PARAGRAPH_VERTICAL_ALIGNMENT_TOP = 0,
  TEXTRA_PARAGRAPH_VERTICAL_ALIGNMENT_CENTER = 1,
  TEXTRA_PARAGRAPH_VERTICAL_ALIGNMENT_BOTTOM = 2
};

typedef uint32_t textra_character_vertical_alignment_t;
enum {
  TEXTRA_CHARACTER_VERTICAL_ALIGNMENT_TOP = 0,
  TEXTRA_CHARACTER_VERTICAL_ALIGNMENT_TEXT_TOP = 1,
  TEXTRA_CHARACTER_VERTICAL_ALIGNMENT_SUPER_SCRIPT = 2,
  TEXTRA_CHARACTER_VERTICAL_ALIGNMENT_MIDDLE = 3,
  TEXTRA_CHARACTER_VERTICAL_ALIGNMENT_BASE_LINE = 4,
  TEXTRA_CHARACTER_VERTICAL_ALIGNMENT_SUB_SCRIPT = 5,
  TEXTRA_CHARACTER_VERTICAL_ALIGNMENT_TEXT_BOTTOM = 6,
  TEXTRA_CHARACTER_VERTICAL_ALIGNMENT_BOTTOM = 7
};

typedef uint32_t textra_ruler_type_t;
enum {
  TEXTRA_RULER_TYPE_AT_LEAST = 0,
  TEXTRA_RULER_TYPE_AUTO = 1,
  TEXTRA_RULER_TYPE_EXACT = 2
};

typedef uint32_t textra_line_height_trim_t;
enum {
  TEXTRA_LINE_HEIGHT_TRIM_NONE = 0,
  TEXTRA_LINE_HEIGHT_TRIM_FIRST_LINE_TOP = 1,
  TEXTRA_LINE_HEIGHT_TRIM_LAST_LINE_BOTTOM = 2,
  TEXTRA_LINE_HEIGHT_TRIM_BOTH = 3
};

typedef uint32_t textra_inline_vertical_alignment_mode_t;
enum {
  TEXTRA_INLINE_VERTICAL_ALIGNMENT_MODE_LEGACY = 0,
  TEXTRA_INLINE_VERTICAL_ALIGNMENT_MODE_LINE_BOX = 1
};

typedef uint32_t textra_word_break_type_t;
enum {
  TEXTRA_WORD_BREAK_TYPE_NORMAL = 0,
  TEXTRA_WORD_BREAK_TYPE_BREAK_ALL = 1,
  TEXTRA_WORD_BREAK_TYPE_KEEP_ALL = 2
};

typedef uint32_t textra_overflow_wrap_t;
enum {
  TEXTRA_OVERFLOW_WRAP_NORMAL = 0,
  TEXTRA_OVERFLOW_WRAP_ANYWHERE = 1,
  TEXTRA_OVERFLOW_WRAP_BREAK_WORD = 2
};

typedef uint32_t textra_write_direction_t;
enum {
  TEXTRA_WRITE_DIRECTION_AUTO = 0,
  TEXTRA_WRITE_DIRECTION_LTR = 1,
  TEXTRA_WRITE_DIRECTION_RTL = 2,
  TEXTRA_WRITE_DIRECTION_TTB = 3,
  TEXTRA_WRITE_DIRECTION_BTT = 4
};

typedef uint32_t textra_layout_mode_t;
enum {
  TEXTRA_LAYOUT_MODE_INDEFINITE = 0,
  TEXTRA_LAYOUT_MODE_DEFINITE = 1,
  TEXTRA_LAYOUT_MODE_AT_MOST = 2
};

typedef uint32_t textra_decoration_type_t;
enum {
  TEXTRA_DECORATION_TYPE_NONE = 0,
  TEXTRA_DECORATION_TYPE_UNDER_LINE = 1,
  TEXTRA_DECORATION_TYPE_OVERLINE = 2,
  TEXTRA_DECORATION_TYPE_LINE_THROUGH = 4
};

typedef uint32_t textra_line_type_t;
enum {
  TEXTRA_LINE_TYPE_NONE = 0,
  TEXTRA_LINE_TYPE_HIDDEN = 1,
  TEXTRA_LINE_TYPE_SOLID = 2,
  TEXTRA_LINE_TYPE_DOTTED = 3,
  TEXTRA_LINE_TYPE_DASHED = 4,
  TEXTRA_LINE_TYPE_DOUBLE = 5,
  TEXTRA_LINE_TYPE_INSET = 6,
  TEXTRA_LINE_TYPE_OUTSET = 7,
  TEXTRA_LINE_TYPE_GROOVE = 8,
  TEXTRA_LINE_TYPE_RIDGE = 9,
  TEXTRA_LINE_TYPE_WAVY = 10
};

typedef uint32_t textra_punctuation_type_t;
enum {
  TEXTRA_PUNCTUATION_TYPE_NONE = 0,
  TEXTRA_PUNCTUATION_TYPE_OPEN = 1,
  TEXTRA_PUNCTUATION_TYPE_CLOSE = 2,
  TEXTRA_PUNCTUATION_TYPE_CENTER = 3
};

typedef uint32_t textra_punctuation_compress_option_t;
enum {
  TEXTRA_PUNCTUATION_COMPRESS_OPTION_NONE = 0,
  TEXTRA_PUNCTUATION_COMPRESS_OPTION_ALL = 1,
  TEXTRA_PUNCTUATION_COMPRESS_OPTION_LINE_EDGE = 2,
  TEXTRA_PUNCTUATION_COMPRESS_OPTION_ADJACENT = 4
};

typedef uint32_t textra_feature_option_t;
enum {
  TEXTRA_FEATURE_OPTION_LAST_LINE_CAN_OVERFLOW = 0,
  TEXTRA_FEATURE_OPTION_SKIP_SPACE_BEFORE_FIRST_LINE = 1,
  TEXTRA_FEATURE_OPTION_TRIM_LINE_TAIL_SPACE = 2,
  TEXTRA_FEATURE_OPTION_HARMONY_SHAPER_FORCE_USE_LOW_API = 3,
  TEXTRA_FEATURE_OPTION_SYSTEM_FONT_ADJUST = 4,
  TEXTRA_FEATURE_OPTION_DISABLE_SHAPE_CACHE = 5,
  TEXTRA_FEATURE_OPTION_ANDROID_METRICS_ROUND = 6
};

typedef uint32_t textra_dominant_baseline_t;
enum {
  TEXTRA_DOMINANT_BASELINE_ALPHABETIC = 0,
  TEXTRA_DOMINANT_BASELINE_TOP = 1,
  TEXTRA_DOMINANT_BASELINE_MIDDLE = 2,
  TEXTRA_DOMINANT_BASELINE_BOTTOM = 3,
  TEXTRA_DOMINANT_BASELINE_IDEOGRAPHIC = 4,
  TEXTRA_DOMINANT_BASELINE_HANGING = 5
};

typedef uint32_t textra_platform_type_t;
enum {
  TEXTRA_PLATFORM_TYPE_SYSTEM = 0,
  TEXTRA_PLATFORM_TYPE_SKITY = 1,
  TEXTRA_PLATFORM_TYPE_SKIA = 2
};

typedef uint32_t textra_shaper_type_t;
enum { TEXTRA_SHAPER_TYPE_SYSTEM = 0, TEXTRA_SHAPER_TYPE_SELF_RENDERING = 1 };

typedef uint32_t textra_paragraph_type_t;
enum {
  TEXTRA_PARAGRAPH_TYPE_GENERAL = 0,
  TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE = 1
};

typedef uint32_t textra_gradient_type_t;
enum {
  TEXTRA_GRADIENT_TYPE_LINEAR = 1,
  TEXTRA_GRADIENT_TYPE_RADIAL = 2,
  TEXTRA_GRADIENT_TYPE_CONIC = 3
};

typedef uint32_t textra_gradient_unit_t;
enum {
  TEXTRA_GRADIENT_UNIT_RATIO = 0,
  TEXTRA_GRADIENT_UNIT_LENGTH = 1,
  TEXTRA_GRADIENT_UNIT_ANGLE = 2
};

typedef uint32_t textra_linear_gradient_direction_t;
enum {
  TEXTRA_LINEAR_GRADIENT_DIRECTION_ANGLE = 0,
  TEXTRA_LINEAR_GRADIENT_DIRECTION_TO_TOP = 1,
  TEXTRA_LINEAR_GRADIENT_DIRECTION_TO_BOTTOM = 2,
  TEXTRA_LINEAR_GRADIENT_DIRECTION_TO_LEFT = 3,
  TEXTRA_LINEAR_GRADIENT_DIRECTION_TO_RIGHT = 4,
  TEXTRA_LINEAR_GRADIENT_DIRECTION_TO_TOP_RIGHT = 5,
  TEXTRA_LINEAR_GRADIENT_DIRECTION_TO_TOP_LEFT = 6,
  TEXTRA_LINEAR_GRADIENT_DIRECTION_TO_BOTTOM_RIGHT = 7,
  TEXTRA_LINEAR_GRADIENT_DIRECTION_TO_BOTTOM_LEFT = 8
};

typedef uint32_t textra_radial_gradient_size_t;
enum {
  TEXTRA_RADIAL_GRADIENT_SIZE_FARTHEST_CORNER = 0,
  TEXTRA_RADIAL_GRADIENT_SIZE_FARTHEST_SIDE = 1,
  TEXTRA_RADIAL_GRADIENT_SIZE_CLOSEST_CORNER = 2,
  TEXTRA_RADIAL_GRADIENT_SIZE_CLOSEST_SIDE = 3,
  TEXTRA_RADIAL_GRADIENT_SIZE_EXPLICIT = 4
};

typedef struct {
  uint32_t start, end;
} textra_char_range_t;
typedef struct {
  float left, top, width, height;
} textra_rect_t;
typedef struct {
  float left, ascent, right, descent;
} textra_tight_bounds_t;
typedef struct {
  uint32_t run_index, char_index_in_run;
} textra_layout_position_t;
typedef struct {
  uint16_t weight;
  uint8_t width, slant;
} textra_font_style_t;
typedef struct {
  float x, y;
} textra_gradient_point_t;
typedef struct {
  float value;
  textra_gradient_unit_t unit;
} textra_gradient_length_t;
typedef struct {
  uint32_t color;
  float position;
  textra_gradient_unit_t unit;
} textra_gradient_stop_t;
typedef struct {
  uint32_t color;
  float offset_x, offset_y;
  double blur_radius;
} textra_text_shadow_t;
typedef void(TEXTRA_CALL* textra_destroy_callback_t)(void* user_data);

typedef struct textra_painter textra_painter_t;
typedef struct textra_font_info textra_font_info_t;
typedef struct textra_font_em_height textra_font_em_height_t;
typedef struct {
  float left, top, right, bottom;
} textra_bounds_t;

typedef uint32_t textra_cap_t;
enum {
  TEXTRA_CAP_BUTT_CAP = 0,
  TEXTRA_CAP_ROUND_CAP = 1,
  TEXTRA_CAP_SQUARE_CAP = 2,
  TEXTRA_CAP_LAST_CAP = 2,
  TEXTRA_CAP_DEFAULT_CAP = 0
};
typedef uint32_t textra_join_t;
enum {
  TEXTRA_JOIN_MITER_JOIN = 0,
  TEXTRA_JOIN_ROUND_JOIN = 1,
  TEXTRA_JOIN_BEVEL_JOIN = 2,
  TEXTRA_JOIN_LAST_JOIN = 2,
  TEXTRA_JOIN_DEFAULT_JOIN = 0
};

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_TYPES_H_
