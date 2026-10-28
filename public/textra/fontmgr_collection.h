// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_FONTMGR_COLLECTION_H_
#define PUBLIC_TEXTRA_FONTMGR_COLLECTION_H_

#include <textra/font_info.h>
#include <textra/i_font_manager.h>

#include <array>
#include <memory>
#include <shared_mutex>
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
  FontmgrCollection() = default;
  explicit FontmgrCollection(FontManagerRef default_font_mgr);

  static constexpr char kDefaultFontFamily[] = "sans-serif";
  size_t getFontManagersCount() const;

  void SetDefaultSystemFontFamily(std::string familyName) {
    DetachTypefaceCache();
    default_system_font_family_ = familyName;
  }

  std::vector<TypefaceRef> findTypefaces(const FontDescriptor& fd) const;

  TypefaceRef defaultFallback(Unichar unicode, FontStyle fontStyle,
                              const std::string& locale) const;
  TypefaceRef defaultFallback() const;

  void disableFontFallback();
  void enableFontFallback();
  bool fontFallbackEnabled() const { return fEnableFontFallback; }

  void clearCaches();

  void SetAssetFontManager(const FontManagerRef& fontManager) {
    DetachTypefaceCache();
    asset_font_manager_ = fontManager;
  }
  void SetDynamicFontManager(const FontManagerRef& fontManager) {
    DetachTypefaceCache();
    dynamic_font_manager_ = fontManager;
  }
  void SetTestFontManager(const FontManagerRef& fontManager) {
    DetachTypefaceCache();
    test_font_manager_ = fontManager;
  }
  void SetDefaultFontManager(const FontManagerRef& fontManager) {
    DetachTypefaceCache();
    default_font_manager_ = fontManager;
  }
  FontManagerRef GetAssetFontManager() const { return asset_font_manager_; }
  FontManagerRef GetDynamicFontManager() const { return dynamic_font_manager_; }
  FontManagerRef GetTestFontManager() const { return test_font_manager_; }
  FontManagerRef GetDefaultFontManager() const { return default_font_manager_; }

 private:
  struct TypefaceCacheState {
    mutable std::shared_mutex mutex;
    uint64_t epoch = 0;
    std::unordered_map<FontDescriptor, std::vector<TypefaceRef>,
                       FontDescriptor::Hasher>
        entries;
  };

  struct FontManagerOrder {
    std::array<FontManagerRef, 4> managers{};
    size_t count = 0;

    void push_back(const FontManagerRef& manager) {
      managers[count++] = manager;
    }
    size_t size() const { return count; }
    const FontManagerRef* begin() const { return managers.data(); }
    const FontManagerRef* end() const { return managers.data() + count; }
  };

  FontManagerOrder getFontManagerOrder() const;
  void DetachTypefaceCache() {
    typeface_cache_ = std::make_shared<TypefaceCacheState>();
  }

  TypefaceRef matchTypeface(const std::string& familyName, FontStyle fontStyle,
                            bool literal = false) const;

  bool fEnableFontFallback = true;
  std::string default_system_font_family_ = kDefaultFontFamily;
  // Copies made for independent TextLayout/TTShaper instances share only
  // immutable resolved typefaces. Manager policy remains call-local; explicit
  // clear invalidates every copy from the same font-resource generation.
  std::shared_ptr<TypefaceCacheState> typeface_cache_ =
      std::make_shared<TypefaceCacheState>();
  std::vector<std::string> fDefaultFamilyNames;
  FontManagerRef default_font_manager_;
  FontManagerRef asset_font_manager_;
  FontManagerRef dynamic_font_manager_;
  FontManagerRef test_font_manager_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_FONTMGR_COLLECTION_H_
