// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifdef TEXTRA_ENABLE_ANDROID_BACKEND
#include <jni.h>
#include <textra/capi/textra_platform.h>
#include <textra/platform_helper.h>

#include "src/ports/shaper/java/java_utils.h"
// Reuse the original Java/C++ SDK initialization, including native peers and
// fallback metrics. No separate C font manager or Java bridge is initialized.
extern "C" textra_status_t TEXTRA_CALL
textra_android_runtime_initialize(void* pointer) {
  if (!pointer) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto* env = static_cast<JNIEnv*>(pointer);
  auto cls = env->FindClass("com/lynx/textra/TTText");
  auto init = cls ? env->GetStaticMethodID(cls, "Initial", "(Z)V") : nullptr;
  if (init && !env->ExceptionCheck())
    env->CallStaticVoidMethod(cls, init, JNI_FALSE);
  if (cls) env->DeleteLocalRef(cls);
  if (ttoffice::tttext::ClearException(env))
    return TEXTRA_STATUS_INTERNAL_ERROR;
  return ttoffice::tttext::PlatformHelper::CreateFontManager(
             ttoffice::tttext::RenderingBackend::kAndroid)
             ? TEXTRA_STATUS_OK
             : TEXTRA_STATUS_INVALID_STATE;
}
#endif  // TEXTRA_ENABLE_ANDROID_BACKEND
