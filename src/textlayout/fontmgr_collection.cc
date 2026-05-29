// Copyright 2019 Google LLC.
#include <textra/fontmgr_collection.h>
#include <textra/i_font_manager.h>

#include "src/textlayout/tt_shaper.h"
#ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
#include "src/ports/shaper/skshaper/sk_shaper.h"
#endif
#ifdef TTTEXT_OS_ANDROID
#include <textra/platform/java/tttext_jni_proxy.h>
#endif

namespace ttoffice {
namespace tttext {
namespace {
constexpr const char* kEmojiBcp47Tag = "und-Zsye";

struct UnicodeRange {
  Unichar start;
  Unichar end;
};

bool IsCommonEmoji(Unichar unicode) {
  // Keep this heuristic local to avoid depending on optional Minikin/ICU emoji
  // tables from the core font fallback path.
  static constexpr UnicodeRange kCommonEmojiRanges[] = {
      {0x00A9, 0x00A9}, {0x00AE, 0x00AE}, {0x203C, 0x203C},   {0x2049, 0x2049},
      {0x2122, 0x2122}, {0x2139, 0x2139}, {0x2194, 0x2199},   {0x21A9, 0x21AA},
      {0x231A, 0x231B}, {0x2328, 0x2328}, {0x23CF, 0x23CF},   {0x23E9, 0x23F3},
      {0x23F8, 0x23FA}, {0x24C2, 0x24C2}, {0x25AA, 0x25AB},   {0x25B6, 0x25B6},
      {0x25C0, 0x25C0}, {0x25FB, 0x25FE}, {0x2600, 0x27BF},   {0x2934, 0x2935},
      {0x2B05, 0x2B07}, {0x2B1B, 0x2B1C}, {0x2B50, 0x2B55},   {0x3030, 0x3030},
      {0x303D, 0x303D}, {0x3297, 0x3299}, {0x1F000, 0x1FAFF},
  };

  for (const auto& range : kCommonEmojiRanges) {
    if (unicode >= range.start && unicode <= range.end) {
      return true;
    }
  }
  return false;
}
}  // namespace

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
    auto match = matchTypeface(default_system_font_family_, font_style);
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
    if (IsCommonEmoji(unicode) && locale != kEmojiBcp47Tag) {
      bcp47.push_back(kEmojiBcp47Tag);
    }
    TypefaceRef typeface(manager->matchFamilyStyleCharacter(
        nullptr, fontStyle, bcp47.data(), static_cast<int>(bcp47.size()),
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
