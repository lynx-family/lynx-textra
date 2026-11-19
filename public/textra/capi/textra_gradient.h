// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_GRADIENT_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_GRADIENT_H_

#include <textra/capi/textra_types.h>

/* Resolved gradients use drawing coordinates. Input stops are frozen records;
 * colors/positions are copied at creation. At least two finite, nondecreasing
 * positions in [0,1] are required; duplicate positions are allowed.
 * Linear: start/end. Radial: center, radius_x/y > 0. Conic: center and angle
 * in degrees (0 points up, positive clockwise). Unused fields are zero.
 * This accepts resolved geometry; CSS parsing/unit resolution stays with
 * caller.
 */
typedef struct textra_gradient_stop {
  uint32_t argb;
  float position;
} textra_gradient_stop_t;

typedef struct textra_resolved_gradient_options {
  uint32_t struct_size;
  textra_gradient_type_t type; /* Default LINEAR. */
  textra_point_t start;
  textra_point_t end;
  textra_point_t center;
  float radius_x;
  float radius_y;
  float start_angle;
  const textra_gradient_stop_t* stops;
  uint32_t stop_count;
} textra_resolved_gradient_options_t;

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL textra_resolved_gradient_create(
    const textra_resolved_gradient_options_t* options,
    textra_resolved_gradient_t** out_gradient);
TEXTRA_API void TEXTRA_CALL
textra_resolved_gradient_destroy(textra_resolved_gradient_t* gradient);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_GRADIENT_H_
