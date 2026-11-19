// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_enum_checks.h"

using namespace textra_capi;

void TEXTRA_CALL textra_paragraph_destroy(textra_paragraph_t* paragraph) {
  Release(paragraph);
}

void TEXTRA_CALL textra_run_delegate_destroy(textra_run_delegate_t* delegate) {
  Release(delegate);
}

void TEXTRA_CALL
textra_resolved_gradient_destroy(textra_resolved_gradient_t* gradient) {
  Release(gradient);
}

void TEXTRA_CALL textra_layout_region_destroy(textra_layout_region_t* region) {
  Release(region);
}

void TEXTRA_CALL textra_style_destroy(textra_style_t* style) { Release(style); }

void TEXTRA_CALL
textra_font_descriptor_destroy(textra_font_descriptor_t* font_descriptor) {
  Release(font_descriptor);
}

void TEXTRA_CALL
textra_paragraph_style_destroy(textra_paragraph_style_t* paragraph_style) {
  Release(paragraph_style);
}

void TEXTRA_CALL textra_text_context_destroy(textra_text_context_t* context) {
  Release(context);
}

void TEXTRA_CALL textra_text_layout_destroy(textra_text_layout_t* layout) {
  Release(layout);
}

void TEXTRA_CALL textra_layout_drawer_destroy(textra_layout_drawer_t* drawer) {
  Release(drawer);
}

uint32_t TEXTRA_CALL textra_get_abi_version_major(void) {
  return TEXTRA_ABI_VERSION_MAJOR;
}

uint32_t TEXTRA_CALL textra_get_abi_version_minor(void) {
  return TEXTRA_ABI_VERSION_MINOR;
}

const char* TEXTRA_CALL textra_status_get_message(textra_status_t status) {
  static const char* const messages[] = {"OK",
                                         "Invalid argument",
                                         "Out of memory",
                                         "Unsupported",
                                         "Invalid state",
                                         "Out of range",
                                         "Buffer too small",
                                         "Busy",
                                         "Not found",
                                         "Backend mismatch",
                                         "Internal error"};
  return status >= 0 && status <= TEXTRA_STATUS_INTERNAL_ERROR
             ? messages[status]
             : "Unknown status";
}
