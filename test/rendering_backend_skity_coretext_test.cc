// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
// Standalone host test: link a CoreText-enabled Skity and Textra with both
// CoreText shaping enabled.
#include <textra/capi/platform/textra_capi_skity_coretext.h>

#include "rendering_backend_skity_cases.hpp"

int main(int argc, char** argv) {
  RunSkityCanvasCallbacksTest(textra_skity_coretext_canvas_callbacks_initialize,
                              TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT);
  textra_font_manager_options_t options{};
  options.struct_size = sizeof(options);
  const auto status =
      textra_skity_coretext_font_manager_callbacks_initialize(&options);
  if (status == TEXTRA_STATUS_UNSUPPORTED) {
    std::fprintf(stderr,
                 "SKIP: Skity C API has no typeface-to-CTFontRef export\n");
    return 77;
  }
  CHECK(status == TEXTRA_STATUS_OK);
  options.manager_callbacks->destroy(options.userdata, options.font_manager);
  return RunSkityBackendTest(
      argc, argv, textra_skity_coretext_font_manager_callbacks_initialize,
      textra_skity_coretext_typeface_callbacks_initialize,
      textra_skity_coretext_canvas_callbacks_initialize,
      TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT);
}
