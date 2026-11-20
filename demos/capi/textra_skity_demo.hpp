// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef DEMOS_CAPI_TEXTRA_SKITY_DEMO_HPP_
#define DEMOS_CAPI_TEXTRA_SKITY_DEMO_HPP_
#include <examples/capi/platform/skity/textra_skity_backend.h>
#include <textra/platform/skity/skity_canvas_helper.h>
#include <textra/platform/skity/skity_font_manager.h>

#include <skity/graphic/bitmap.hpp>
#include <string>
#include <vector>

#include "textra_demo_cpp.hpp"
namespace textra_demo {
inline textra_status_t RenderSkity(skity::Canvas* native, uint32_t sample) {
  auto status = TEXTRA_STATUS_OK;
  textra_font_manager_options_t manager{};
  manager.struct_size = sizeof(manager);
  if (!status)
    status = textra_skity_font_manager_callbacks_initialize(&manager);
  textra_canvas_options_t canvas{};
  canvas.struct_size = sizeof(canvas);
  canvas.native_canvas = native;
  if (!status) status = textra_skity_canvas_callbacks_initialize(&canvas);
  textra_demo_metrics_t metrics{};
  if (!status)
    status = textra_demo_render(TEXTRA_RENDERING_BACKEND_SKITY, &manager,
                                nullptr, &canvas, sample, 320, 200, &metrics);
  if (manager.manager_callbacks)
    manager.manager_callbacks->destroy(manager.userdata,
                                       const_cast<void*>(manager.font_manager));
  return status;
}
}  // namespace textra_demo
#endif  // DEMOS_CAPI_TEXTRA_SKITY_DEMO_HPP_
