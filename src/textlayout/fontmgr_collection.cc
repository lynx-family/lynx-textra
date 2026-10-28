// Copyright 2019 Google LLC.
#include <textra/fontmgr_collection.h>
#include <textra/i_font_manager.h>

#include <shared_mutex>

#include "src/textlayout/tt_shaper.h"
#include "src/textlayout/utils/grapheme_utils.h"
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
FontmgrCollection::FontManagerOrder FontmgrCollection::getFontManagerOrder()
    const {
  FontManagerOrder order;
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
  uint64_t cache_epoch = 0;
  // Look inside the font collections cache first
  {
    std::shared_lock lock(typeface_cache_->mutex);
    cache_epoch = typeface_cache_->epoch;
    auto found = typeface_cache_->entries.find(fd);
    if (found != typeface_cache_->entries.end()) {
      return found->second;
    }
  }

  auto& font_style = fd.font_style_;
  std::vector<TypefaceRef> typefaces;
  bool has_css_generic = false;
  for (size_t index = 0; index < fd.font_family_list_.size(); ++index) {
    const auto& font_family = fd.font_family_list_[index];
    const bool literal = index < fd.font_family_literals_.size() &&
                         fd.font_family_literals_[index];
    const bool typed = index < fd.font_family_literals_.size();
    if (typed && !literal) {
      has_css_generic = true;
      if (default_font_manager_ && fEnableFontFallback) {
        auto match = default_font_manager_->matchGenericFamily(
            font_family.c_str(), font_style);
        if (match) typefaces.emplace_back(std::move(match));
      }
      continue;
    }
    if (TypefaceRef match = matchTypeface(font_family, font_style, literal)) {
      typefaces.emplace_back(std::move(match));
    }
  }

  if (typefaces.empty()) {
    // Do not reintroduce a raw generic asset key through the all-missing
    // fallback. Named-only/legacy descriptors keep their existing behavior.
    auto match = has_css_generic
                     ? (default_font_manager_ && fEnableFontFallback
                            ? default_font_manager_->matchGenericFamily(
                                  "sans-serif", font_style)
                            : nullptr)
                     : matchTypeface(default_system_font_family_, font_style);
    if (match) {
      typefaces.emplace_back(match);
    }
  }

  {
    std::unique_lock lock(typeface_cache_->mutex);
    if (cache_epoch != typeface_cache_->epoch) return typefaces;
    auto [found, inserted] =
        typeface_cache_->entries.emplace(fd, std::move(typefaces));
    return found->second;
  }
}

TypefaceRef FontmgrCollection::matchTypeface(const std::string& familyName,
                                             FontStyle fontStyle,
                                             bool literal) const {
  if (literal && familyName.empty()) return nullptr;
  for (const auto& manager : this->getFontManagerOrder()) {
    auto match =
        literal
            ? manager->matchFamilyStyleLiteral(familyName.c_str(), fontStyle)
            : manager->matchFamilyStyle(familyName.c_str(), fontStyle);
    if (match) {
      return match;
    }
  }

  for (const auto& manager : this->getFontManagerOrder()) {
    auto match =
        literal
            ? manager->legacyMakeTypefaceLiteral(familyName.c_str(), fontStyle)
            : manager->legacyMakeTypeface(familyName.c_str(), fontStyle);
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
    // At most the requested locale and the emoji tag, both borrowed only for
    // this synchronous manager call. Keep each manager's original evaluation.
    const char* bcp47[2] = {};
    int bcp47_count = 0;
    if (!locale.empty()) {
      bcp47[bcp47_count++] = locale.c_str();
    }
    if ((IsCommonEmoji(unicode) || IsCombiningEnclosingKeycap(unicode)) &&
        locale != kEmojiBcp47Tag) {
      bcp47[bcp47_count++] = kEmojiBcp47Tag;
    }
    TypefaceRef typeface(manager->matchFamilyStyleCharacter(
        nullptr, fontStyle, bcp47_count == 0 ? nullptr : bcp47, bcp47_count,
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

void FontmgrCollection::disableFontFallback() {
  DetachTypefaceCache();
  fEnableFontFallback = false;
}
void FontmgrCollection::enableFontFallback() {
  DetachTypefaceCache();
  fEnableFontFallback = true;
}

void FontmgrCollection::clearCaches() {
  {
    std::unique_lock lock(typeface_cache_->mutex);
    typeface_cache_->entries.clear();
    ++typeface_cache_->epoch;
  }
#ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
  SkShaper::PurgeCaches();
#endif
}
}  // namespace tttext
}  // namespace ttoffice
