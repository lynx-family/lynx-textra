// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_callback_utils.h"
#include "src/capi/textra_internal.h"

using namespace textra_capi;

textra_status_t TEXTRA_CALL
textra_layout_drawer_create(const textra_layout_drawer_options_t* options,
                            textra_layout_drawer_t** out_drawer) {
  if (!out_drawer) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_drawer = nullptr;
  if (!ValidStruct(options)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto canvas_options = TEXTRA_READ(
      options, canvas, static_cast<const textra_canvas_options_t*>(nullptr));
  if (!canvas_options) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto value = std::make_unique<textra_layout_drawer>();
  value->listener.owner = value.get();
  auto table = TEXTRA_READ(
      options, callbacks,
      static_cast<const textra_layout_drawer_callbacks_t*>(nullptr));
  if (table) {
    if (!ValidStruct(table)) return TEXTRA_STATUS_INVALID_ARGUMENT;
    if (TEXTRA_READ(table, version, uint32_t{0}) != 1)
      return TEXTRA_STATUS_UNSUPPORTED;
    value->callbacks.struct_size = sizeof(value->callbacks);
    value->callbacks.version = 1;
    value->callbacks.resolve_color = TEXTRA_READ(
        table, resolve_color, decltype(table->resolve_color){nullptr});
    value->callbacks.destroy =
        TEXTRA_READ(table, destroy, decltype(table->destroy){nullptr});
  }
  value->userdata = TEXTRA_READ(options, userdata, static_cast<void*>(nullptr));
  textra_canvas_t* canvas = nullptr;
  auto status = textra_canvas_create(canvas_options, &canvas);
  if (status) return status;
  value->canvas = Ref<textra_canvas>::Adopt(canvas);
  value->adopted = true;
  *out_drawer = value.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_layout_drawer_draw_region(
    textra_layout_drawer_t* drawer, const textra_layout_region_t* region) {
  return Draw(drawer, region, nullptr, UINT32_MAX, nullptr, 0, 0);
}

textra_status_t TEXTRA_CALL textra_layout_drawer_draw_line(
    textra_layout_drawer_t* drawer, const textra_layout_region_t* region,
    uint32_t line_index, const textra_text_range_t* range) {
  return Draw(drawer, region, nullptr, line_index, range, 0, 0);
}

textra_status_t TEXTRA_CALL textra_layout_drawer_draw_single_line(
    textra_layout_drawer_t* drawer, const textra_paragraph_t* paragraph,
    float x, float y) {
  return Draw(drawer, nullptr, paragraph, 0, nullptr, x, y);
}
