// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_HELPER_H_
#include <textra/i_canvas_helper.h>
#include <textra/i_font_manager.h>
#include <textra/macro.h>

#include <memory>
namespace ttoffice {
namespace tttext {
enum class PlatformType {
  kSystem,
  kSkity,
  kSkia,
};
class L_EXPORT PlatformHelper {
 public:
  static std::unique_ptr<ICanvasHelper> CreateCanvasHelper(PlatformType type,
                                                           void* handler);
  static std::shared_ptr<IFontManager> CreateFontManager(PlatformType type);
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_HELPER_H_
