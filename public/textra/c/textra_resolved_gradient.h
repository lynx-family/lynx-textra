// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_RESOLVED_GRADIENT_H_
#define PUBLIC_TEXTRA_C_TEXTRA_RESOLVED_GRADIENT_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API void TEXTRA_CALL textra_resolved_gradient_retain(
    textra_resolved_gradient_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_resolved_gradient_release(
    textra_resolved_gradient_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_resolved_gradient_get_bounds(const textra_resolved_gradient_t* gradient,
                                    textra_rect_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API textra_gradient_type_t TEXTRA_CALL textra_resolved_gradient_get_type(
    const textra_resolved_gradient_t* gradient) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_resolved_gradient_get_start(
    const textra_resolved_gradient_t* gradient,
    textra_gradient_point_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_resolved_gradient_get_end(const textra_resolved_gradient_t* gradient,
                                 textra_gradient_point_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_resolved_gradient_get_center(
    const textra_resolved_gradient_t* gradient,
    textra_gradient_point_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_resolved_gradient_get_radius_x(
    const textra_resolved_gradient_t* gradient) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_resolved_gradient_get_radius_y(
    const textra_resolved_gradient_t* gradient) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_resolved_gradient_get_start_angle(
    const textra_resolved_gradient_t* gradient) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_resolved_gradient_get_stops(
    const textra_resolved_gradient_t* gradient, uint32_t* colors,
    float* positions, size_t capacity, size_t* required_count) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_RESOLVED_GRADIENT_H_
