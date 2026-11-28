// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
// Standalone host test: link Skity and Textra with HarfBuzz shaping enabled.
#include <textra/capi/platform/textra_capi_skity.h>

#include "rendering_backend_skity_cases.hpp"

int main(int argc, char** argv) {
  RunSkityCanvasCallbacksTest(textra_skity_canvas_callbacks_initialize,
                              TEXTRA_RENDERING_BACKEND_SKITY);
  return RunSkityBackendTest(
      argc, argv, textra_skity_font_manager_callbacks_initialize,
      textra_skity_typeface_callbacks_initialize,
      textra_skity_canvas_callbacks_initialize, TEXTRA_RENDERING_BACKEND_SKITY);
}
