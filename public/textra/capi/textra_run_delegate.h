// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_RUN_DELEGATE_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_RUN_DELEGATE_H_

#include <textra/capi/textra_types.h>

/* Non-text run callback contract:
 * measure is required; draw is optional for measurement-only content. If draw
 * is absent, drawing visible delegate content returns UNSUPPORTED.
 * The library copies complete known callback fields, never retains the table.
 * version must be 1. Missing optional fields are NULL. On successful creation,
 * userdata follows the common callback ownership rules; destroy is called
 * exactly once on final release if present. Failure leaves it with the caller.
 * Callbacks execute synchronously on the calling thread, must not throw, and
 * must not reenter layout/draw or mutate participating resources. They may
 * return status failures; those propagate out of the enclosing operation.
 *
 * Delegates must keep geometry and draw-visible state unchanged while used by
 * a live region or a valid single-line result. Callback canvas is borrowed and
 * cannot be retained/destroyed. Native drawing may be performed using
 * userdata's own backend context; no C++ canvas methods are exported through
 * this handle.
 */
typedef struct textra_run_metrics {
  uint32_t struct_size;
  float ascent;  /* Signed relative to baseline, normally <= 0. */
  float descent; /* Signed relative to baseline, normally >= 0. */
  float advance;
} textra_run_metrics_t;

typedef struct textra_run_callbacks {
  uint32_t struct_size;
  uint32_t version;
  textra_status_t(TEXTRA_CALL* measure)(void* userdata,
                                        textra_run_metrics_t* out_metrics);
  textra_status_t(TEXTRA_CALL* draw)(void* userdata, textra_canvas_t* canvas,
                                     float x, float baseline_y);
  void(TEXTRA_CALL* destroy)(void* userdata);
} textra_run_callbacks_t;

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL textra_run_delegate_create(
    const textra_run_callbacks_t* callbacks, void* userdata,
    textra_run_delegate_t** out_delegate);
TEXTRA_API void TEXTRA_CALL
textra_run_delegate_destroy(textra_run_delegate_t* delegate);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_RUN_DELEGATE_H_
