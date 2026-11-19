// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_internal.h"

using namespace textra_capi;

textra_status_t TEXTRA_CALL textra_run_delegate_create(
    const textra_run_callbacks_t* callbacks, void* userdata,
    textra_run_delegate_t** out_delegate) {
  if (!out_delegate) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_delegate = nullptr;
  if (!ValidStruct(callbacks)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (TEXTRA_READ(callbacks, version, uint32_t{0}) != 1)
    return TEXTRA_STATUS_UNSUPPORTED;
  auto value = std::make_unique<textra_run_delegate>();
  value->impl = std::make_shared<Delegate>();
  auto& target = *value->impl;
  target.userdata = userdata;
  target.callbacks.struct_size = sizeof(target.callbacks);
  target.callbacks.version = 1;
  target.callbacks.measure =
      TEXTRA_READ(callbacks, measure, decltype(callbacks->measure){nullptr});
  target.callbacks.draw =
      TEXTRA_READ(callbacks, draw, decltype(callbacks->draw){nullptr});
  target.callbacks.destroy =
      TEXTRA_READ(callbacks, destroy, decltype(callbacks->destroy){nullptr});
  if (!target.callbacks.measure) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto status = target.Measure();
  if (status) return status;
  target.adopted = true;
  *out_delegate = value.release();
  return TEXTRA_STATUS_OK;
}
