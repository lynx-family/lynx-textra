// Copyright 2019 Google LLC.
#include <textra/fontmgr_collection.h>
#include <textra/i_font_manager.h>

#include "src/textlayout/tt_shaper.h"
#ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
#include "src/ports/shaper/skshaper/sk_shaper.h"
#endif

namespace ttoffice {
namespace tttext {
FontmgrCollection::FontmgrCollection(FontManagerRef default_fontmgr)
    : fEnableFontFallback(true),
      fDefaultFamilyNames({std::string{kDefaultFontFamily}}) {
  SetDefaultFontManager(default_fontmgr);
}

size_t FontmgrCollection::getFontManagersCount() const {
  return this->getFontManagerOrder().size();
}

// Return the available font managers in the order they should be queried.
std::vector<FontManagerRef> FontmgrCollection::getFontManagerOrder() const {
  std::vector<FontManagerRef> order;
  if (dynamic_font_manager_) {
    order.push_back(dynamic_font_manager_);
  }
  if (asset_font_manager_) {
    order.push_back(asset_font_manager_);
  }
  if (test_font_manager_) {
    order.push_back(test_font_manager_);
  }
  if (default_font_manager_ && fEnableFontFallback) {
    order.push_back(default_font_manager_);
  }
  return order;
}

std::vector<TypefaceRef> FontmgrCollection::findTypefaces(
    const FontDescriptor& fd) const {
  // Look inside the font collections cache first
  auto found = fTypefaces.find(fd);
  if (found != fTypefaces.end()) {
    return found->second;
  }

  auto& font_style = fd.font_style_;
  std::vector<TypefaceRef> typefaces;
  for (const auto& font_family : fd.font_family_list_) {
    if (TypefaceRef match = matchTypeface(font_family, font_style)) {
      typefaces.emplace_back(std::move(match));
    }
  }

  if (typefaces.empty()) {
    auto match = matchTypeface("sans-serif", font_style);
    if (match) {
      typefaces.emplace_back(match);
    }
  }

  fTypefaces[fd] = typefaces;
  return typefaces;
}

TypefaceRef FontmgrCollection::matchTypeface(const std::string& familyName,
                                             FontStyle fontStyle) const {
  for (const auto& manager : this->getFontManagerOrder()) {
    auto match = manager->matchFamilyStyle(familyName.c_str(), fontStyle);
    if (match) {
      return match;
    }
  }

  for (const auto& manager : this->getFontManagerOrder()) {
    auto match = manager->legacyMakeTypeface(familyName.c_str(), fontStyle);
    if (match) {
      return match;
    }
  }

  return nullptr;
}

// Find ANY font in available font managers that resolves the unicode codepoint
TypefaceRef FontmgrCollection::defaultFallback(
    Unichar unicode, FontStyle fontStyle, const std::string& locale) const {
  for (const auto& manager : this->getFontManagerOrder()) {
    std::vector<const char*> bcp47;
    if (!locale.empty()) {
      bcp47.push_back(locale.c_str());
    }
    TypefaceRef typeface(manager->matchFamilyStyleCharacter(
        nullptr, fontStyle, bcp47.data(), static_cast<uint32_t>(bcp47.size()),
        unicode));
    if (typeface != nullptr) {
      return typeface;
    }
  }
  return nullptr;
}

TypefaceRef FontmgrCollection::defaultFallback() const {
  if (default_font_manager_ == nullptr) {
    return nullptr;
  }
  for (const std::string& familyName : fDefaultFamilyNames) {
    TypefaceRef match = default_font_manager_->matchFamilyStyle(
        familyName.c_str(), FontStyle());
    if (match) {
      return TypefaceRef(match);
    }
  }
  return nullptr;
}

void FontmgrCollection::disableFontFallback() { fEnableFontFallback = false; }
void FontmgrCollection::enableFontFallback() { fEnableFontFallback = true; }

void FontmgrCollection::clearCaches() {
  fTypefaces.clear();
#ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
  SkShaper::PurgeCaches();
#endif
}
}  // namespace tttext
}  // namespace ttoffice
