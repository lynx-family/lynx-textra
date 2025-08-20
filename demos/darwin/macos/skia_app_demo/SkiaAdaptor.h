// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_DARWIN_MACOS_SKIA_APP_DEMO_SKIAADAPTOR_H_
#define DEMOS_DARWIN_MACOS_SKIA_APP_DEMO_SKIAADAPTOR_H_

#include <core/SkFontMgr.h>
#include <ports/SkFontMgr_directory.h>
#include <textra/platform/skia/skia_typeface_helper.h>

#include <memory>

inline auto skia_font_mgr =
    SkFontMgr_New_Custom_Directory("/System/Library/Fonts/");
inline auto TF_DEFAULT = skia_font_mgr->makeFromFile(
    FONT_ROOT "Noto_Sans_CJK/NotoSansCJK-Regular.ttc", 0);
inline auto TF_MENLO = skia_font_mgr->makeFromFile(
    FONT_ROOT "JetBrains_Mono/JetBrainsMono-VariableFont_wght.ttf", 0);
inline auto TF_HELVETICA = skia_font_mgr->makeFromFile(
    FONT_ROOT "Inter/Inter-VariableFont_opsz,wght.ttf", 0);
inline auto TF_EMOJI = skia_font_mgr->makeFromFile(
    FONT_ROOT "Noto_Color_Emoji/NotoColorEmoji-Regular.ttf", 0);
inline auto TFH_DEFAULT =
    std::make_shared<tttext::SkiaTypefaceHelper>(TF_DEFAULT);
inline auto TFH_MENLO = std::make_shared<tttext::SkiaTypefaceHelper>(TF_MENLO);
inline auto TFH_HELVETICA =
    std::make_shared<tttext::SkiaTypefaceHelper>(TF_HELVETICA);
inline auto TFH_EMOJI = std::make_shared<tttext::SkiaTypefaceHelper>(TF_EMOJI);

#endif  // DEMOS_DARWIN_MACOS_SKIA_APP_DEMO_SKIAADAPTOR_H_
