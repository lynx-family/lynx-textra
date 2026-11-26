// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_COMMON_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_COMMON_H_

#include <stddef.h>
#include <stdint.h>

/* Stable C facade. Implementations are in src/capi.
 * No existing C++ ABI or platform implementation is exposed by these headers.
 *
 * ABI rules for the implementation:
 * - A major version fixes calling conventions, symbol signatures and constants
 *   for a given OS/architecture ABI. C enum-like types share the corresponding
 *   C++ enum widths and values; those published definitions must remain fixed.
 * - Initialize every extensible struct to zero, then set struct_size to its
 *   sizeof. Only append fields at or beyond the previous published sizeof;
 *   never reuse a previous version's tail padding. Validate struct_size is at
 *   least sizeof(uint32_t), and require complete mandatory fields. Input
 *   option pointers themselves are required unless explicitly marked optional.
 *   Read/write complete fields within struct_size;
 *   never copy sizeof(the latest struct) into a caller's smaller allocation.
 *   Missing fields use the documented defaults. Reject unknown flags and
 *   enum inputs with TEXTRA_STATUS_UNSUPPORTED; never silently omit a feature.
 *   Output struct_size is input capacity and is never overwritten.
 * - Fixed records explicitly marked frozen must never grow, including in
 * arrays.
 * - All create/clone/get-handle calls return an owned reference unless stated
 *   otherwise. On failure, handle outputs are NULL. destroy(NULL) is a no-op.
 *   destroy releases one reference; dependencies can keep the object alive.
 *   Output pointers are required unless explicitly documented as optional.
 *   Dependencies retain resources internally; there is no public retain API.
 *   Boolean inputs accept only FALSE/TRUE.
 * - Non-handle outputs are unchanged on failure except documented size queries.
 *   Input buffers are borrowed for the call and copied if retained, unless a
 *   factory explicitly documents native-object lifetime rules.
 * - C++ exceptions are disabled; callbacks must report errors via status codes.
 *   Operations on the same mutable object,
 *   including drawing, require external serialization. Do not mutate/destroy
 *   active objects from callbacks. Callback data is borrowed for that call.
 *   In callback tables, a non-NULL destroy callback transfers userdata
 * ownership on successful creation. Otherwise userdata is borrowed and must
 * remain valid until the owning resource is finally destroyed. A table's
 * version field is mandatory even when every behavior callback is optional.
 * - UTF-8 lengths/capacities count bytes. Text positions count Unicode scalar
 *   values (not UTF-16 units or grapheme clusters); ranges are [start, end).
 *   Reject malformed UTF-8 with INVALID_ARGUMENT. Empty input allows NULL.
 * - Coordinates are float layout units, x right/y down. Drawing origins are
 *   layout-box tops unless documented otherwise. Colors are 0xAARRGGBB,
 *   unpremultiplied sRGB; explicit transparent black is not an unset property.
 * - String copies exclude a NUL terminator. out_size is required and receives
 *   required bytes. NULL/zero queries succeed. Insufficient capacity returns
 *   BUFFER_TOO_SMALL without a partial copy, and updates out_size.
 */

#ifndef TEXTRA_API
#if defined(_WIN32) && defined(TEXTRA_CAPI_SHARED)
#if defined(TEXTRA_CAPI_BUILD)
#define TEXTRA_API __declspec(dllexport)
#else
#define TEXTRA_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define TEXTRA_API __attribute__((visibility("default")))
#else
#define TEXTRA_API
#endif
#endif

#ifndef TEXTRA_CALL
#if defined(_WIN32)
#define TEXTRA_CALL __cdecl
#else
#define TEXTRA_CALL
#endif
#endif

#ifdef __cplusplus
#define TEXTRA_EXTERN_C_BEGIN extern "C" {
#define TEXTRA_EXTERN_C_END }
#else
#define TEXTRA_EXTERN_C_BEGIN
#define TEXTRA_EXTERN_C_END
#endif

#define TEXTRA_ABI_VERSION_MAJOR 1u
#define TEXTRA_ABI_VERSION_MINOR 1u

typedef int32_t textra_status_t;
#define TEXTRA_STATUS_OK ((textra_status_t)0)
#define TEXTRA_STATUS_INVALID_ARGUMENT ((textra_status_t)1)
#define TEXTRA_STATUS_OUT_OF_MEMORY ((textra_status_t)2)
#define TEXTRA_STATUS_UNSUPPORTED ((textra_status_t)3)
#define TEXTRA_STATUS_INVALID_STATE ((textra_status_t)4)
#define TEXTRA_STATUS_OUT_OF_RANGE ((textra_status_t)5)
#define TEXTRA_STATUS_BUFFER_TOO_SMALL ((textra_status_t)6)
#define TEXTRA_STATUS_BUSY ((textra_status_t)7)
#define TEXTRA_STATUS_NOT_FOUND ((textra_status_t)8)
#define TEXTRA_STATUS_BACKEND_MISMATCH ((textra_status_t)9)
#define TEXTRA_STATUS_INTERNAL_ERROR ((textra_status_t)10)

typedef uint32_t textra_bool_t;
#define TEXTRA_FALSE ((textra_bool_t)0)
#define TEXTRA_TRUE ((textra_bool_t)1)

typedef struct textra_canvas textra_canvas_t;
typedef struct textra_typeface textra_typeface_t;
typedef struct textra_font_descriptor textra_font_descriptor_t;
typedef struct textra_style textra_style_t;
typedef struct textra_paragraph_style textra_paragraph_style_t;
typedef struct textra_paragraph textra_paragraph_t;
typedef struct textra_text_context textra_text_context_t;
typedef struct textra_text_layout textra_text_layout_t;
typedef struct textra_layout_region textra_layout_region_t;
typedef struct textra_layout_drawer textra_layout_drawer_t;
typedef struct textra_run_delegate textra_run_delegate_t;
typedef struct textra_resolved_gradient textra_resolved_gradient_t;

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API uint32_t TEXTRA_CALL textra_get_abi_version_major(void);
TEXTRA_API uint32_t TEXTRA_CALL textra_get_abi_version_minor(void);
/* NUL-terminated diagnostic constant; static library-owned storage. */
TEXTRA_API const char* TEXTRA_CALL
textra_status_get_message(textra_status_t status);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_COMMON_H_
