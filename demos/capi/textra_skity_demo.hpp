// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef DEMOS_CAPI_TEXTRA_SKITY_DEMO_HPP_
#define DEMOS_CAPI_TEXTRA_SKITY_DEMO_HPP_
#include <textra/capi/platform/textra_capi_skity.h>
#include <textra/platform/skity/skity_canvas_helper.h>
#include <textra/platform/skity/skity_font_manager.h>

#include <skity/graphic/image.hpp>
#include <skity/io/data.hpp>
#include <string>
#include <vector>

#include "demos/capi/skity_capi_utils.hpp"
#include "textra_demo_cpp.hpp"

namespace textra_demo {
inline textra_status_t RenderSkity(skity_canvas native, uint32_t sample) {
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
  textra_text_layout_clear_shape_cache();
  return status;
}
// The GPU demo remains a C++ Skity consumer. The SDK has no C/C++ canvas
// handle conversion, so its C API demonstration renders offscreen and presents
// an owned image; ordinary C++ rendering keeps its original direct path.
inline textra_status_t RenderSkity(skity::Canvas* native, uint32_t sample) {
  if (!native) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto bitmap = ttoffice::tttext::skity_c::MakeBitmap(
      320, 200, SKITY_ALPHA_TYPE_PREMUL, SKITY_COLOR_TYPE_RGBA);
  if (!bitmap) return TEXTRA_STATUS_OUT_OF_MEMORY;
  auto canvas = ttoffice::tttext::skity_c::MakeCanvas(bitmap.get());
  if (!canvas) return TEXTRA_STATUS_OUT_OF_MEMORY;
  skity_canvas_draw_color(canvas.get(), 0, SKITY_BLEND_MODE_CLEAR);
  auto status = RenderSkity(canvas.get(), sample);
  if (status) return status;
  const auto row_bytes = skity_bitmap_get_row_bytes(bitmap.get());
  auto data = skity::Data::MakeWithCopy(skity_bitmap_get_pixels(bitmap.get()),
                                        row_bytes * 200);
  if (!data) return TEXTRA_STATUS_OUT_OF_MEMORY;
  auto pixels = std::make_shared<skity::Pixmap>(
      data, row_bytes, 320, 200, skity::AlphaType::kPremul_AlphaType,
      skity::ColorType::kRGBA);
  auto image = skity::Image::MakeImage(pixels);
  if (!image) return TEXTRA_STATUS_OUT_OF_MEMORY;
  native->DrawImage(image, 0, 0);
  return TEXTRA_STATUS_OK;
}
}  // namespace textra_demo
#endif  // DEMOS_CAPI_TEXTRA_SKITY_DEMO_HPP_
