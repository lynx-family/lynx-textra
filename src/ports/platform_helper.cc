// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/platform_helper.h>

#ifdef TTTEXT_OS_OH
#include "src/ports/renderer/ark_graphics/ag_canvas_helper.h"
#include "src/ports/renderer/ark_graphics/ag_font_manager.h"
#endif
namespace ttoffice {
namespace tttext {
std::unique_ptr<ICanvasHelper> PlatformHelper::CreateCanvasHelper(
    PlatformType type, void* handler) {
  if (type == PlatformType::kSystem) {
#ifdef TTTEXT_OS_OH
    return std::make_unique<AGCanvasHelper>(handler);
#endif
  }
  return nullptr;
}
std::shared_ptr<IFontManager> PlatformHelper::CreateFontManager(
    PlatformType type) {
  if (type == PlatformType::kSystem) {
#ifdef TTTEXT_OS_OH
    return std::make_shared<AGFontManager>();
#endif
  }
  return nullptr;
}
}  // namespace tttext
}  // namespace ttoffice
