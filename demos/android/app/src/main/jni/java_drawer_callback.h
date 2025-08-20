// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_ANDROID_APP_SRC_MAIN_JNI_JAVA_DRAWER_CALLBACK_H_
#define DEMOS_ANDROID_APP_SRC_MAIN_JNI_JAVA_DRAWER_CALLBACK_H_

#include <jni.h>
#include <textra/layout_drawer_listener.h>
#include <textra/macro.h>

#include <map>
class L_EXPORT JavaDrawerCallback final : public tttext::LayoutDrawerListener {
 public:
  JavaDrawerCallback(JNIEnv* env, jobject callback) {}
  ~JavaDrawerCallback() override = default;

 public:
  void SetJNIEnv(JNIEnv* env) {}
};

#endif  // DEMOS_ANDROID_APP_SRC_MAIN_JNI_JAVA_DRAWER_CALLBACK_H_
