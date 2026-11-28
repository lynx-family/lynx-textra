// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <textra/capi/platform/textra_capi_skity.h>
#include <textra/capi/platform/textra_capi_skity_coretext.h>

/* Create in a different C translation unit, then use/destroy in the caller. */
textra_status_t skity_header_peer_initialize(
    textra_typeface_options_t* options);
textra_status_t skity_header_peer_initialize(
    textra_typeface_options_t* options) {
  return textra_skity_typeface_callbacks_initialize(options);
}
