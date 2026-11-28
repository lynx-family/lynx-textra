// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_COMMON_H_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_COMMON_H_

#include <limits.h>
#include <math.h>
#include <skity/c/skity.h>
#include <stdlib.h>
#include <string.h>
#include <textra/capi/textra_platform.h>

TEXTRA_EXTERN_C_BEGIN
static inline void* textra_skity_impl_allocate(size_t count, size_t size) {
  if (count && size > SIZE_MAX / count) return NULL;
  return malloc(count * size);
}

/* Internal implementation shared by the two header-only adapters. All handles
 * returned by Skity are owned unless explicitly marked borrowed. */
TEXTRA_EXTERN_C_END
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_COMMON_H_
