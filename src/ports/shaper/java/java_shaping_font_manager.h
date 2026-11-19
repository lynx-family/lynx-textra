// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef SRC_PORTS_SHAPER_JAVA_JAVA_SHAPING_FONT_MANAGER_H_
#define SRC_PORTS_SHAPER_JAVA_JAVA_SHAPING_FONT_MANAGER_H_
#include <jni.h>
#include <textra/fontmgr_collection.h>

#include <array>
#include <memory>
#include <unordered_map>

namespace ttoffice {
namespace tttext {
class JavaFontManager;
class JavaTypeface;
struct JavaShapingFont {
  TypefaceRef typeface;
  jobject instance = nullptr;  // Borrowed; retained by typeface/manager.
};
// Internal font-manager boundary shared by the original C++ and C adapters.
// Matching, immutable metrics variants and fallback policy belong here, not in
// the shaping loop. A JavaShaper and its manager require external
// serialization.
class JavaShapingFontManager final {
 public:
  explicit JavaShapingFontManager(std::shared_ptr<JavaFontManager> manager);
  static std::unique_ptr<JavaShapingFontManager> Create(
      const FontmgrCollection&);
  jobject GetInstance() const;
  // Chinese mask: bit 0 BMP, bit 1 supplementary; only used with metrics_round.
  JavaShapingFont matchTypeface(const FontDescriptor&, bool metrics_round,
                                uint32_t chinese_mask);

 private:
  struct Entry {
    std::shared_ptr<JavaTypeface> base;
    std::array<std::shared_ptr<JavaTypeface>, 4> rounded{};
  };
  std::shared_ptr<JavaFontManager> manager_;
  std::unordered_map<FontDescriptor, Entry, FontDescriptor::Hasher> entries_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_JAVA_JAVA_SHAPING_FONT_MANAGER_H_
