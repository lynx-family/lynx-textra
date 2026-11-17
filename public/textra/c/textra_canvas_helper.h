// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_CANVAS_HELPER_H_
#define PUBLIC_TEXTRA_C_TEXTRA_CANVAS_HELPER_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

/* Creates an empty callback adapter. Setters below only accept such adapters.
 * A configured callback takes precedence even if it returns NULL. Otherwise
 * the method delegates to fallback; NULL clears a callback or fallback.
 * Fallback is borrowed and must outlive this adapter. It must not form a cycle.
 * user_data is destroyed once when this adapter is destroyed.
 * Callback arguments are borrowed only during the call; do not retain/store
 * shared handles received in callbacks. CreatePainter transfers its result's
 * ownership to the adapter's caller. With no callback/fallback, CreatePainter
 * returns a plain Painter; drawing/state methods do nothing. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_canvas_helper_create(void* user_data, textra_destroy_callback_t destroy,
                            textra_canvas_helper_t** out) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_fallback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_t* fallback) TEXTRA_NOEXCEPT;

typedef void(TEXTRA_CALL* textra_canvas_helper_start_paint_callback_t)(
    void* user_data);
typedef void(TEXTRA_CALL* textra_canvas_helper_end_paint_callback_t)(
    void* user_data);
typedef void(TEXTRA_CALL* textra_canvas_helper_save_callback_t)(
    void* user_data);
typedef void(TEXTRA_CALL* textra_canvas_helper_restore_callback_t)(
    void* user_data);
typedef void(TEXTRA_CALL* textra_canvas_helper_translate_callback_t)(
    void* user_data, float x, float y);
typedef void(TEXTRA_CALL* textra_canvas_helper_scale_callback_t)(
    void* user_data, float x, float y);
typedef void(TEXTRA_CALL* textra_canvas_helper_rotate_callback_t)(
    void* user_data, float degrees);
typedef void(TEXTRA_CALL* textra_canvas_helper_skew_callback_t)(void* user_data,
                                                                float x,
                                                                float y);
typedef void(TEXTRA_CALL* textra_canvas_helper_clip_rect_callback_t)(
    void* user_data, float left, float top, float right, float bottom,
    uint32_t antialias);
typedef void(TEXTRA_CALL* textra_canvas_helper_clear_callback_t)(
    void* user_data);
typedef void(TEXTRA_CALL* textra_canvas_helper_clear_rect_callback_t)(
    void* user_data, float left, float top, float right, float bottom);
typedef void(TEXTRA_CALL* textra_canvas_helper_fill_rect_callback_t)(
    void* user_data, float left, float top, float right, float bottom,
    uint32_t color);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_color_callback_t)(
    void* user_data, uint32_t color);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_line_callback_t)(
    void* user_data, float x1, float y1, float x2, float y2,
    textra_painter_t* painter);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_rect_callback_t)(
    void* user_data, float left, float top, float right, float bottom,
    textra_painter_t* painter);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_oval_callback_t)(
    void* user_data, float left, float top, float right, float bottom,
    textra_painter_t* painter);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_circle_callback_t)(
    void* user_data, float x, float y, float radius, textra_painter_t* painter);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_arc_callback_t)(
    void* user_data, float left, float top, float right, float bottom,
    float start_angle, float sweep_angle, uint32_t use_center,
    textra_painter_t* painter);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_arc_to_callback_t)(
    void* user_data, float start_x, float start_y, float mid_x, float mid_y,
    float end_x, float end_y, float radius, textra_painter_t* painter);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_round_rect_callback_t)(
    void* user_data, float left, float top, float right, float bottom,
    float radius, textra_painter_t* painter);
typedef textra_painter_t*(
    TEXTRA_CALL* textra_canvas_helper_create_painter_callback_t)(
    void* user_data);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_text_callback_t)(
    void* user_data, const textra_typeface_helper_t* font, const char* text,
    uint32_t text_bytes, float x, float y, textra_painter_t* painter);
typedef void(TEXTRA_CALL* textra_canvas_helper_draw_glyphs_callback_t)(
    void* user_data, const textra_typeface_helper_t* font, uint32_t glyph_count,
    const uint16_t* glyphs, const char* text, uint32_t text_bytes,
    float origin_x, float origin_y, float* x, float* y,
    textra_painter_t* painter);

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_start_paint_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_start_paint_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_end_paint_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_end_paint_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_save_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_save_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_restore_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_restore_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_translate_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_translate_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_scale_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_scale_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_rotate_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_rotate_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_skew_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_skew_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_clip_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_clip_rect_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_clear_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_clear_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_clear_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_clear_rect_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_fill_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_fill_rect_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_color_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_color_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_line_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_line_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_rect_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_oval_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_oval_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_circle_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_circle_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_arc_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_arc_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_arc_to_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_arc_to_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_round_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_round_rect_callback_t callback) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_create_painter_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_create_painter_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_text_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_text_callback_t callback) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_set_draw_glyphs_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_glyphs_callback_t callback) TEXTRA_NOEXCEPT;

/* Methods preserve ICanvasHelper argument units and backend behavior. An entry
 * point does not imply that every platform backend implements the operation.
 * Font and Painter arguments are borrowed during calls. */

TEXTRA_API void TEXTRA_CALL
textra_canvas_helper_save(textra_canvas_helper_t* canvas) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_canvas_helper_restore(textra_canvas_helper_t* canvas) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_start_paint(
    textra_canvas_helper_t* canvas) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_canvas_helper_end_paint(textra_canvas_helper_t* canvas) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_translate(
    textra_canvas_helper_t* canvas, float x, float y) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_scale(
    textra_canvas_helper_t* canvas, float x, float y) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_rotate(
    textra_canvas_helper_t* canvas, float degrees) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_clip_rect(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, uint32_t antialias) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_canvas_helper_destroy(textra_canvas_helper_t* canvas) TEXTRA_NOEXCEPT;

/* Owned painter; release with textra_painter_destroy. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_canvas_helper_create_painter(
    textra_canvas_helper_t* canvas, textra_painter_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_skew(
    textra_canvas_helper_t* value, float sx, float sy) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_canvas_helper_clear(textra_canvas_helper_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_clear_rect(
    textra_canvas_helper_t* value, float left, float top, float right,
    float bottom) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_fill_rect(
    textra_canvas_helper_t* value, float left, float top, float right,
    float bottom, uint32_t color) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_color(
    textra_canvas_helper_t* value, uint32_t color) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_line(
    textra_canvas_helper_t* canvas, float x1, float y1, float x2, float y2,
    textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_rect(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_oval(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_circle(
    textra_canvas_helper_t* canvas, float x, float y, float radius,
    textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_arc(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, float start_angle, float sweep_angle, uint32_t use_center,
    textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_arc_to(
    textra_canvas_helper_t* canvas, float start_x, float start_y, float mid_x,
    float mid_y, float end_x, float end_y, float radius,
    textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_round_rect(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, float radius, textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_text(
    textra_canvas_helper_t* canvas, const textra_typeface_helper_t* font,
    const char* text, uint32_t text_bytes, float x, float y,
    textra_painter_t* painter) TEXTRA_NOEXCEPT;

/* Position arrays remain writable, as in ICanvasHelper::DrawGlyphs. */
TEXTRA_API void TEXTRA_CALL textra_canvas_helper_draw_glyphs(
    textra_canvas_helper_t* canvas, const textra_typeface_helper_t* font,
    uint32_t glyph_count, const uint16_t* glyphs, const char* text,
    uint32_t text_bytes, float origin_x, float origin_y, float* x, float* y,
    textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_CANVAS_HELPER_H_
