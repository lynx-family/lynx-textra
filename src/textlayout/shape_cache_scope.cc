// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/shape_cache_scope.h>

#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "src/textlayout/shape_cache.h"
#include "src/textlayout/shape_cache_scope_internal.h"
#include "src/textlayout/shape_cache_utils.h"

namespace ttoffice {
namespace tttext {

class ShapeCacheScope::Impl final {
 public:
  bool IsScopedLocked(const FontDescriptor& descriptor) const {
    if (descriptor.platform_font_ != 0) {
      return true;
    }
    for (const auto& family : descriptor.font_family_list_) {
      if (custom_families_.find(CanonicalizeShapeCacheFamily(family)) !=
          custom_families_.end()) {
        return true;
      }
    }
    return false;
  }

  mutable std::shared_mutex generation_mutex_;
  uint64_t generation_ = 0;
  std::unordered_set<std::string> custom_families_;
  ShapeCache shape_cache_;
  std::unordered_map<const void*, std::shared_ptr<ShapeCachePlatformState>>
      platform_states_;
};

ShapeCacheScope::ShapeCacheScope() : impl_(std::make_unique<Impl>()) {}

ShapeCacheScope::~ShapeCacheScope() = default;

void ShapeCacheScope::NotifyCustomFontChanged(const std::string& family) {
  const std::string canonical_family = CanonicalizeShapeCacheFamily(family);
  if (canonical_family.empty()) {
    return;
  }

  std::unique_lock lock(impl_->generation_mutex_);
  impl_->custom_families_.insert(canonical_family);
  ++impl_->generation_;
  impl_->shape_cache_.RemoveFamily(canonical_family);
  for (const auto& state : impl_->platform_states_) {
    state.second->RemoveFamily(canonical_family);
  }
}

void ShapeCacheScope::Clear() {
  std::unique_lock lock(impl_->generation_mutex_);
  ++impl_->generation_;
  impl_->shape_cache_.Clear();
  for (const auto& state : impl_->platform_states_) {
    state.second->Clear();
  }
}

ShapeCacheScopeInternal::Route ShapeCacheScopeInternal::CaptureRoute(
    std::shared_ptr<ShapeCacheScope> scope, const FontDescriptor& descriptor) {
  Route route;
  route.scope = std::move(scope);
  if (route.scope == nullptr) {
    return route;
  }

  std::shared_lock lock(route.scope->impl_->generation_mutex_);
  route.generation = route.scope->impl_->generation_;
  route.use_scoped_cache = route.scope->impl_->IsScopedLocked(descriptor);
  return route;
}

ShapeCache& ShapeCacheScopeInternal::GetShapeCache(const Route& route) {
  if (route.use_scoped_cache) {
    return route.scope->impl_->shape_cache_;
  }
  return ShapeCache::GetInstance();
}

bool ShapeCacheScopeInternal::RunIfCurrent(
    const Route& route, const std::function<void()>& operation) {
  if (route.scope == nullptr) {
    operation();
    return true;
  }

  std::shared_lock lock(route.scope->impl_->generation_mutex_);
  if (route.generation != route.scope->impl_->generation_) {
    return false;
  }
  operation();
  return true;
}

std::shared_ptr<ShapeCachePlatformState>
ShapeCacheScopeInternal::GetOrCreatePlatformState(
    const Route& route, const void* identity,
    const std::function<std::shared_ptr<ShapeCachePlatformState>()>& factory) {
  if (!route.use_scoped_cache || route.scope == nullptr) {
    return nullptr;
  }

  std::unique_lock lock(route.scope->impl_->generation_mutex_);
  if (route.generation != route.scope->impl_->generation_) {
    return nullptr;
  }
  auto [iterator, inserted] =
      route.scope->impl_->platform_states_.emplace(identity, nullptr);
  if (inserted) {
    iterator->second = factory();
  }
  return iterator->second;
}

}  // namespace tttext
}  // namespace ttoffice
