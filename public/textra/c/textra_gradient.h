// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_GRADIENT_H_
#define PUBLIC_TEXTRA_C_TEXTRA_GRADIENT_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_gradient_create(textra_gradient_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_gradient_destroy(textra_gradient_t* value)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_gradient_set_type(
    textra_gradient_t* gradient, textra_gradient_type_t type) TEXTRA_NOEXCEPT;

TEXTRA_API textra_gradient_type_t TEXTRA_CALL
textra_gradient_get_type(const textra_gradient_t* gradient) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_gradient_set_linear(
    textra_gradient_t* gradient, float angle,
    textra_linear_gradient_direction_t direction) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_gradient_set_radial(
    textra_gradient_t* gradient, textra_gradient_length_t center_x,
    textra_gradient_length_t center_y, textra_radial_gradient_size_t size,
    uint32_t ellipse, textra_gradient_length_t radius_x,
    textra_gradient_length_t radius_y) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_gradient_set_conic(
    textra_gradient_t* gradient, textra_gradient_length_t center_x,
    textra_gradient_length_t center_y, float start_angle) TEXTRA_NOEXCEPT;

/* Preserve Gradient::Resolve preconditions: normalized, finite, sorted stops.
 */
TEXTRA_API void TEXTRA_CALL textra_gradient_set_stops(
    textra_gradient_t* gradient, const textra_gradient_stop_t* stops,
    size_t count) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL
textra_gradient_resolve(const textra_gradient_t* gradient, textra_rect_t bounds,
                        textra_resolved_gradient_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_gradient_get_linear(
    const textra_gradient_t* gradient, float* angle,
    textra_linear_gradient_direction_t* direction) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_gradient_get_conic(
    const textra_gradient_t* gradient, textra_gradient_length_t* center_x,
    textra_gradient_length_t* center_y, float* start_angle) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_gradient_get_radial(
    const textra_gradient_t* gradient, textra_gradient_length_t* center_x,
    textra_gradient_length_t* center_y, textra_radial_gradient_size_t* size,
    uint32_t* ellipse, textra_gradient_length_t* radius_x,
    textra_gradient_length_t* radius_y) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_gradient_get_stops(
    const textra_gradient_t* gradient, textra_gradient_stop_t* stops,
    size_t capacity, size_t* required_count) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_GRADIENT_H_
