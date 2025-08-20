// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_MODULE_H_
#define PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_MODULE_H_

#include <arkui/native_interface.h>
#include <arkui/native_node.h>
#include <arkui/native_node_napi.h>
#include <arkui/native_type.h>

#include <cassert>

namespace NativeModule {

class NativeModuleInstance {
 public:
  static NativeModuleInstance* GetInstance() {
    static NativeModuleInstance instance;
    return &instance;
  }

  NativeModuleInstance() {
    // Get the function pointer structure object of the NDK interface, for
    // subsequent operations.
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1,
                                arkUINativeNodeApi_);
    assert(arkUINativeNodeApi_);
  }
  // Expose to other modules for use.
  ArkUI_NativeNodeAPI_1* GetNativeNodeAPI() { return arkUINativeNodeApi_; }

 private:
  ArkUI_NativeNodeAPI_1* arkUINativeNodeApi_ = nullptr;
};

}  // namespace NativeModule

#endif  // PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_MODULE_H_
