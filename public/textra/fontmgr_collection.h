// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_FONTMGR_COLLECTION_H_
#define PUBLIC_TEXTRA_FONTMGR_COLLECTION_H_

#include <textra/font_info.h>
#include <textra/i_font_manager.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ttoffice {
namespace tttext {
/**
 * @brief Manages multiple font managers for font matching and fallback.
 *
 * Given a FontDescriptor, finds the matching typeface by querying font managers
 * in the following priority order:
 * 1. Dynamic font manager (highest)
 * 2. Asset font manager
 * 3. Test font manager
 * 4. Default font manager (fallback; can be disabled)
 *
 * Caches resolved FontDescriptor to TypefaceRef mappings to optimize repeated
 * lookups.
 */
class L_EXPORT FontmgrCollection {
 public:
  FontmgrCollection() = delete;
  explicit FontmgrCollection(FontManagerRef default_font_mgr);

  static constexpr char kDefaultFontFamily[] = "sans-serif";
  size_t getFontManagersCount() const;

  std::vector<TypefaceRef> findTypefaces(const FontDescriptor& fd) const;

  TypefaceRef defaultFallback(Unichar unicode, FontStyle fontStyle,
                              const std::string& locale) const;
  TypefaceRef defaultFallback() const;

  void disableFontFallback();
  void enableFontFallback();
  bool fontFallbackEnabled() const { return fEnableFontFallback; }

  void clearCaches();

  void SetAssetFontManager(const FontManagerRef& fontManager) {
    asset_font_manager_ = fontManager;
  }
  void SetDynamicFontManager(const FontManagerRef& fontManager) {
    dynamic_font_manager_ = fontManager;
  }
  void SetTestFontManager(const FontManagerRef& fontManager) {
    test_font_manager_ = fontManager;
  }
  void SetDefaultFontManager(const FontManagerRef& fontManager) {
    default_font_manager_ = fontManager;
  }
  FontManagerRef GetAssetFontManager() const { return asset_font_manager_; }
  FontManagerRef GetDynamicFontManager() const { return dynamic_font_manager_; }
  FontManagerRef GetTestFontManager() const { return test_font_manager_; }
  FontManagerRef GetDefaultFontManager() const { return default_font_manager_; }

 private:
  std::vector<FontManagerRef> getFontManagerOrder() const;

  TypefaceRef matchTypeface(const std::string& familyName,
                            FontStyle fontStyle) const;

  bool fEnableFontFallback;
  mutable std::unordered_map<FontDescriptor, std::vector<TypefaceRef>,
                             FontDescriptor::Hasher>
      fTypefaces;
  std::vector<std::string> fDefaultFamilyNames;
  FontManagerRef default_font_manager_;
  FontManagerRef asset_font_manager_;
  FontManagerRef dynamic_font_manager_;
  FontManagerRef test_font_manager_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_FONTMGR_COLLECTION_H_
