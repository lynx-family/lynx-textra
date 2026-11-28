// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <textra/capi/platform/textra_capi_skia.h>

textra_status_t InitializeSkiaCanvasInPeer(textra_canvas_options_t* options) {
  return textra_skia_canvas_callbacks_initialize(options);
}
textra_status_t InitializeSkiaTypefaceInPeer(
    textra_typeface_options_t* options) {
  return textra_skia_typeface_callbacks_initialize(options);
}
