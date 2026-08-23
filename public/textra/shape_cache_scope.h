// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_SHAPE_CACHE_SCOPE_H_
#define PUBLIC_TEXTRA_SHAPE_CACHE_SCOPE_H_

#include <textra/macro.h>

#include <memory>
#include <string>

namespace ttoffice {
namespace tttext {

class ShapeCacheScopeInternal;

// Owns shaping and platform-font cache entries whose lifetime must be scoped
// to one client font environment. Callers normally keep one scope for the
// lifetime of that environment and pass it to each temporary TTTextContext.
class L_EXPORT ShapeCacheScope final {
 public:
  ShapeCacheScope();
  ~ShapeCacheScope();

  ShapeCacheScope(const ShapeCacheScope&) = delete;
  ShapeCacheScope& operator=(const ShapeCacheScope&) = delete;

  // Marks |family| as custom in this scope and invalidates cached values that
  // depend on it. Family matching is ASCII case-insensitive.
  void NotifyCustomFontChanged(const std::string& family);

  // Clears cached values owned by this scope while preserving its custom-font
  // family classification.
  void Clear();

 private:
  friend class ShapeCacheScopeInternal;
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_SHAPE_CACHE_SCOPE_H_
