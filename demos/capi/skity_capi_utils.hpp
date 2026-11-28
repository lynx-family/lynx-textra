// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef DEMOS_CAPI_SKITY_CAPI_UTILS_HPP_
#define DEMOS_CAPI_SKITY_CAPI_UTILS_HPP_

#include <skity/c/skity.h>

#include <memory>
#include <type_traits>

namespace ttoffice {
namespace tttext {
namespace skity_c {
// These owners manage C handles; they never expose Skity C++ SDK objects.
template <typename H>
std::shared_ptr<typename std::remove_pointer<H>::type> Take(
    H handle, void (*destroy)(H)) {
  if (!handle) return {};
  return {handle, destroy};
}
using Typeface = std::shared_ptr<skity_typeface_s>;
using Bitmap = std::shared_ptr<skity_bitmap_s>;
using Context = std::shared_ptr<skity_context_s>;
using Surface = std::shared_ptr<skity_surface_s>;
using Canvas = std::shared_ptr<skity_canvas_s>;

inline Typeface LoadTypeface(const char* path) {
  return Take(skity_typeface_make_from_file(path), skity_typeface_destroy);
}
inline Bitmap MakeBitmap(uint32_t width, uint32_t height,
                         skity_alpha_type alpha = SKITY_ALPHA_TYPE_UNPREMUL,
                         skity_color_type color = SKITY_COLOR_TYPE_RGBA) {
  return Take(skity_bitmap_create(width, height, alpha, color),
              skity_bitmap_destroy);
}
inline Canvas MakeCanvas(skity_bitmap bitmap) {
  return Take(skity_canvas_make_software_canvas(bitmap), skity_canvas_destroy);
}
}  // namespace skity_c
}  // namespace tttext
}  // namespace ttoffice
#endif  // DEMOS_CAPI_SKITY_CAPI_UTILS_HPP_
