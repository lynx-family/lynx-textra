// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_FONT_MANAGER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_FONT_MANAGER_H_

#include <textra/i_font_manager.h>
#include <textra/platform/skia/skia_typeface_helper.h>

#include <cassert>
#include <memory>
#include <utility>

#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_directory.h"

namespace ttoffice {
namespace tttext {
/**
 * @brief A font manager implementation backed by Skia's FontManager.
 */
class SkiaFontManager : public IFontManager {
 public:
  SkiaFontManager() = delete;
  explicit SkiaFontManager(sk_sp<SkFontMgr> sk_font_mgr)
      : sk_font_mgr_(sk_font_mgr) {
    assert(sk_font_mgr != nullptr);
  }
  ~SkiaFontManager() override = default;

 public:
  int countFamilies() const override { return sk_font_mgr_->countFamilies(); }
  TypefaceRef matchFamilyStyle(const char* familyName,
                               const FontStyle& style) override {
    const auto* sk_font_style = reinterpret_cast<const SkFontStyle*>(&style);
    auto sk_typeface =
        sk_font_mgr_->matchFamilyStyle(familyName, *sk_font_style);
    return sk_typeface == nullptr
               ? nullptr
               : std::make_shared<SkiaTypefaceHelper>(sk_typeface);
  }
  TypefaceRef matchFamilyStyleCharacter(const char* familyName,
                                        const FontStyle& style,
                                        const char** bcp47, int bcp47Count,
                                        uint32_t character) override {
    const auto* sk_font_style = reinterpret_cast<const SkFontStyle*>(&style);
    auto sk_typeface = sk_font_mgr_->matchFamilyStyleCharacter(
        familyName, *sk_font_style, bcp47, bcp47Count, character);
    return sk_typeface == nullptr
               ? nullptr
               : std::make_shared<SkiaTypefaceHelper>(sk_typeface);
  }
  TypefaceRef makeFromFile(const char* path, int ttcIndex) override {
    auto sk_typeface = sk_font_mgr_->makeFromFile(path, ttcIndex);
    return sk_typeface == nullptr
               ? nullptr
               : std::make_shared<SkiaTypefaceHelper>(sk_typeface);
  }
  TypefaceRef makeFromStream(std::unique_ptr<SkStreamAsset> stream,
                             int ttcIndex) {
    auto sk_typeface =
        sk_font_mgr_->makeFromStream(std::move(stream), ttcIndex);
    return sk_typeface == nullptr
               ? nullptr
               : std::make_shared<SkiaTypefaceHelper>(sk_typeface);
  }
  TypefaceRef legacyMakeTypeface(const char* familyName,
                                 FontStyle style) const override {
    const auto* sk_font_style = reinterpret_cast<const SkFontStyle*>(&style);
    auto sk_typeface =
        sk_font_mgr_->legacyMakeTypeface(familyName, *sk_font_style);
    return sk_typeface == nullptr
               ? nullptr
               : std::make_shared<SkiaTypefaceHelper>(sk_typeface);
  }

 private:
  sk_sp<SkFontMgr> sk_font_mgr_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_FONT_MANAGER_H_
