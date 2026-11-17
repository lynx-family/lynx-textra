// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_CAPI_TEXTRA_CAPI_INTERNAL_H_
#define SRC_CAPI_TEXTRA_CAPI_INTERNAL_H_

#include <textra/c/textra_types.h>
#include <textra/gradient.h>
#include <textra/i_font_manager.h>
#include <textra/run_delegate.h>

#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>

// Implementation details of shared C handles, never part of the public API.
namespace ttoffice {
namespace tttext {
namespace c_api_internal {
template <typename T, typename H>
auto Raw(H* handle) {
  using Result = std::conditional_t<std::is_const_v<H>, const T, T>;
  return reinterpret_cast<Result*>(handle);
}
template <typename H, typename T>
H* Handle(T* value) {
  return reinterpret_cast<H*>(value);
}
template <typename H, typename T>
H* Share(std::shared_ptr<T> value) {
  return value ? new H(std::move(value)) : nullptr;
}
template <typename H>
void Retain(H* value) {
  value->refs.fetch_add(1, std::memory_order_relaxed);
}
template <typename H>
void Release(H* value) {
  if (value && value->refs.fetch_sub(1, std::memory_order_acq_rel) == 1)
    delete value;
}
template <typename T>
struct SharedHandle {
  explicit SharedHandle(std::shared_ptr<T> input) : value(std::move(input)) {}
  std::atomic<uint32_t> refs{1};
  std::shared_ptr<T> value;
};
}  // namespace c_api_internal
}  // namespace tttext
}  // namespace ttoffice

struct textra_font_manager
    : tttext::c_api_internal::SharedHandle<tttext::IFontManager> {
  using SharedHandle::SharedHandle;
};
struct textra_typeface_helper
    : tttext::c_api_internal::SharedHandle<tttext::ITypefaceHelper> {
  using SharedHandle::SharedHandle;
};
struct textra_run_delegate
    : tttext::c_api_internal::SharedHandle<tttext::RunDelegate> {
  using SharedHandle::SharedHandle;
};
struct textra_resolved_gradient
    : tttext::c_api_internal::SharedHandle<const tttext::ResolvedGradient> {
  using SharedHandle::SharedHandle;
};

#endif  // SRC_CAPI_TEXTRA_CAPI_INTERNAL_H_
