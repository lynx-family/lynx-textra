// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_ANDROID_APP_SRC_MAIN_JNI_TTTEXT_GLOBAL_CONFIG_H_
#define DEMOS_ANDROID_APP_SRC_MAIN_JNI_TTTEXT_GLOBAL_CONFIG_H_
#include <textra/macro.h>
namespace illusion {
class L_EXPORT TTTextGlobalConfig {
 private:
  TTTextGlobalConfig() = default;
  ~TTTextGlobalConfig() = default;

 public:
  static TTTextGlobalConfig& GetInstance() {
    static TTTextGlobalConfig config;
    return config;
  }
  float GetDpi() const { return dpi_; }
  void SetDpi(float dpi) { dpi_ = dpi; }

  bool IsAndroidEnableLayoutCache() const {
    return android_enable_layout_cache;
  }
  void SetAndroidEnableLayoutCache(bool enable) {
    android_enable_layout_cache = enable;
  }

 private:
  float dpi_{};
  bool android_enable_layout_cache{true};
};
}  // namespace illusion
#endif  // DEMOS_ANDROID_APP_SRC_MAIN_JNI_TTTEXT_GLOBAL_CONFIG_H_
