// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>
#include <textra/c/textra_platform_helper.h>

#if defined(TTTEXT_OS_MAC)
#import <AppKit/AppKit.h>
#endif

// These existing platform headers contain deprecated APIs and unused locals.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wunused-variable"
#include <textra/platform/ios/ios_font_manager.h>
#if defined(TTTEXT_OS_MAC)
#include <textra/platform/mac/mac_canvas_base.h>
#else
#include <textra/platform/ios/ios_canvas_base.h>
#endif
#pragma clang diagnostic pop

#include "src/capi/textra_capi_internal.h"

textra_status_t TEXTRA_CALL
textra_font_manager_create_coretext(textra_font_manager_t** out) noexcept {
  *out = new textra_font_manager_t(
      std::make_shared<tttext::FontManagerCoreText>());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_canvas_helper_create_coregraphics(
    CGContextRef context, textra_canvas_helper_t** out) noexcept {
  *out = reinterpret_cast<textra_canvas_helper_t*>(
      static_cast<tttext::ICanvasHelper*>(new IOSCanvasBase(context)));
  return TEXTRA_STATUS_OK;
}
