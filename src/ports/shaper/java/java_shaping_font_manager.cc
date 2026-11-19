// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include "src/ports/shaper/java/java_shaping_font_manager.h"

#include <algorithm>
#include <utility>

#include "src/ports/native/java/java_font_manager.h"
#include "src/ports/native/java/tttext_jni_proxy.h"

namespace ttoffice {
namespace tttext {
JavaShapingFontManager::JavaShapingFontManager(
    std::shared_ptr<JavaFontManager> manager)
    : manager_(std::move(manager)) {}

jobject JavaShapingFontManager::GetInstance() const {
  return manager_->GetInstance();
}

JavaShapingFont JavaShapingFontManager::matchTypeface(const FontDescriptor& fd,
                                                      bool rounded,
                                                      uint32_t mask) {
  auto font = manager_->matchTypeface(fd);
  if (!font) {
    return {};
  }
  if (!rounded) {
    font->SetStaticLayoutMetricsEnabled(false);
    return {font, font->GetInstance()};
  }
  auto& entry = entries_[fd];
  if (entry.base != font) {
    entry = {};
    entry.base = font;
  }
  auto& variant = entry.rounded[mask & 3];
  if (!variant) {
    float ascent = font->GetAscentRatio(), descent = font->GetDescentRatio();
    auto fallback = manager_->GetDefaultChineseTypeface(false);
    auto fallback_manager = manager_;
    if (!fallback) {
      // Legacy hosts may initialize fallback metrics on TTText's default
      // manager before supplying a separate font collection.
      fallback_manager = TTTextJNIProxy::GetInstance().GetDefaultFontManager();
      if (fallback_manager)
        fallback = fallback_manager->GetDefaultChineseTypeface(false);
    }
    auto merge = [&](const std::shared_ptr<JavaTypeface>& face) {
      if (face) {
        ascent = std::min(ascent, face->GetAscentRatio());
        descent = std::max(descent, face->GetDescentRatio());
      }
    };
    if (mask & 1) merge(fallback);
    if ((mask & 2) && fallback_manager)
      merge(fallback_manager->GetDefaultChineseTypeface(true));
    auto copy = font->CopyWithFallbackMetrics(ascent, descent);
    copy->SetStaticLayoutMetricsEnabled(true);
    variant = std::move(copy);
  }
  return {variant, variant->GetInstance()};
}

std::unique_ptr<JavaShapingFontManager> JavaShapingFontManager::Create(
    const FontmgrCollection& fonts) {
  auto manager = fonts.GetDefaultFontManager();
  if (!manager) {
    return nullptr;
  }
  return std::make_unique<JavaShapingFontManager>(
      std::static_pointer_cast<JavaFontManager>(manager));
}
}  // namespace tttext
}  // namespace ttoffice
