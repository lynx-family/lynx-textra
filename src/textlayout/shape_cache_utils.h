// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_SHAPE_CACHE_UTILS_H_
#define SRC_TEXTLAYOUT_SHAPE_CACHE_UTILS_H_

#include <textra/font_info.h>

#include <string>

namespace ttoffice {
namespace tttext {

inline std::string CanonicalizeShapeCacheFamily(const std::string& family) {
  std::string canonical = family;
  for (char& character : canonical) {
    if (character >= 'A' && character <= 'Z') {
      character = static_cast<char>(character - 'A' + 'a');
    }
  }
  return canonical;
}

inline bool FontDescriptorContainsShapeCacheFamily(
    const FontDescriptor& descriptor, const std::string& canonical_family) {
  for (const auto& family : descriptor.font_family_list_) {
    if (CanonicalizeShapeCacheFamily(family) == canonical_family) {
      return true;
    }
  }
  return false;
}

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_SHAPE_CACHE_UTILS_H_
