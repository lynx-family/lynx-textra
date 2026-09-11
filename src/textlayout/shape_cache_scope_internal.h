// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_SHAPE_CACHE_SCOPE_INTERNAL_H_
#define SRC_TEXTLAYOUT_SHAPE_CACHE_SCOPE_INTERNAL_H_

#include <textra/font_info.h>
#include <textra/shape_cache_scope.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace ttoffice {
namespace tttext {

class ShapeCache;

class ShapeCachePlatformState {
 public:
  virtual ~ShapeCachePlatformState() = default;
  virtual void Clear() = 0;
  virtual void RemoveFamily(const std::string& canonical_family) = 0;
};

class ShapeCacheScopeInternal final {
 public:
  struct Route {
    std::shared_ptr<ShapeCacheScope> scope;
    uint64_t generation = 0;
    bool use_scoped_cache = false;
  };

  static Route CaptureRoute(std::shared_ptr<ShapeCacheScope> scope,
                            const FontDescriptor& descriptor);
  static ShapeCache& GetShapeCache(const Route& route);

  // Executes |operation| while holding the scope's shared generation barrier.
  // Font updates and Clear() take the exclusive side of this barrier.
  static bool RunIfCurrent(const Route& route,
                           const std::function<void()>& operation);

  static std::shared_ptr<ShapeCachePlatformState> GetOrCreatePlatformState(
      const Route& route, const void* identity,
      const std::function<std::shared_ptr<ShapeCachePlatformState>()>& factory);
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_SHAPE_CACHE_SCOPE_INTERNAL_H_
