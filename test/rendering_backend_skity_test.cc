// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
// Standalone host test: link a CoreText-enabled Skity and Textra with both
// HarfBuzz and CoreText shaping enabled.
#include <examples/capi/platform/skity/textra_skity_backend.h>
#include <examples/capi/platform/skity_coretext/textra_skity_coretext_backend.h>

#include "rendering_backend_skity_cases.hpp"

int main(int argc, char** argv) {
  RunSkityCanvasCallbacksTest(textra_skity_canvas_callbacks_initialize,
                              TEXTRA_RENDERING_BACKEND_SKITY);
  RunSkityCanvasCallbacksTest(textra_skity_coretext_canvas_callbacks_initialize,
                              TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT);
  RunSkityBackendTest(
      argc, argv, textra_skity_font_manager_callbacks_initialize,
      textra_skity_typeface_callbacks_initialize,
      textra_skity_canvas_callbacks_initialize, TEXTRA_RENDERING_BACKEND_SKITY);
  return RunSkityBackendTest(
      argc, argv, textra_skity_coretext_font_manager_callbacks_initialize,
      textra_skity_coretext_typeface_callbacks_initialize,
      textra_skity_coretext_canvas_callbacks_initialize,
      TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT);
}
