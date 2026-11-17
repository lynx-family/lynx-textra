// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_RUN_DELEGATE_H_
#define PUBLIC_TEXTRA_C_TEXTRA_RUN_DELEGATE_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

typedef float(TEXTRA_CALL* textra_run_metric_callback_t)(void* user_data);
typedef void(TEXTRA_CALL* textra_run_layout_callback_t)(void* user_data);
typedef void(TEXTRA_CALL* textra_run_draw_callback_t)(
    void* user_data, textra_canvas_helper_t* canvas, float x, float y);
typedef uint32_t(TEXTRA_CALL* textra_run_equals_callback_t)(
    void* user_data, textra_run_delegate_t* other);
/* Canvas and other are borrowed only during the callback. Do not retain,
 * destroy or store them. Configure mandatory metrics before adding a run.
 * Setters apply only to delegates created by this C create function. */

TEXTRA_API void TEXTRA_CALL
textra_run_delegate_retain(textra_run_delegate_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_run_delegate_release(textra_run_delegate_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL
textra_run_delegate_create(void* user_data, textra_destroy_callback_t destroy,
                           textra_run_delegate_t** out) TEXTRA_NOEXCEPT;

/* Only for delegates created by textra_run_delegate_create, including the
 * borrowed other handle in Equals callbacks. The result is borrowed. */
TEXTRA_API void* TEXTRA_CALL textra_run_delegate_get_user_data(
    const textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_run_delegate_set_ascent_callback(
    textra_run_delegate_t* delegate,
    textra_run_metric_callback_t callback) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_run_delegate_set_descent_callback(
    textra_run_delegate_t* delegate,
    textra_run_metric_callback_t callback) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_run_delegate_set_advance_callback(
    textra_run_delegate_t* delegate,
    textra_run_metric_callback_t callback) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_run_delegate_set_layout_callback(
    textra_run_delegate_t* delegate,
    textra_run_layout_callback_t callback) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_run_delegate_set_draw_callback(
    textra_run_delegate_t* delegate,
    textra_run_draw_callback_t callback) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_run_delegate_set_equals_callback(
    textra_run_delegate_t* delegate,
    textra_run_equals_callback_t callback) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_run_delegate_get_ascent(
    const textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_run_delegate_get_descent(
    const textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_run_delegate_get_advance(
    const textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_run_delegate_get_x_offset(
    const textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_run_delegate_get_y_offset(
    const textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_run_delegate_layout(textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_run_delegate_draw(
    textra_run_delegate_t* delegate, textra_canvas_helper_t* canvas, float x,
    float y) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_RUN_DELEGATE_H_
