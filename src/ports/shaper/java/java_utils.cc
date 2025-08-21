// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/java/java_utils.h"

#include <android/log.h>
#include <jni.h>

#include <string>

namespace ttoffice {
namespace tttext {
bool HasException(JNIEnv* env) { return env->ExceptionCheck() != JNI_FALSE; }

bool ClearException(JNIEnv* env) {
  if (!HasException(env)) return false;
  env->ExceptionDescribe();
  env->ExceptionClear();
  return true;
}

jclass GetClass(JNIEnv* env, const char* class_name) {
  jclass clazz = env->FindClass(class_name);
  if (ClearException(env) || !clazz) {
    std::string msg = "Failed to find class " + std::string(class_name);
    __android_log_write(ANDROID_LOG_FATAL, "textra", msg.c_str());
  }
  return clazz;
}
jmethodID GetMethod(JNIEnv* env, jclass clazz, MethodType type,
                    const char* method_name, const char* jni_signature) {
  jmethodID id = nullptr;
  if (clazz) {
    if (type == STATIC_METHOD) {
      id = env->GetStaticMethodID(clazz, method_name, jni_signature);
    } else if (type == INSTANCE_METHOD) {
      id = env->GetMethodID(clazz, method_name, jni_signature);
    }
    if (ClearException(env) || !id) {
      std::string msg = "Failed to find " +
                        std::string((type == STATIC_METHOD) ? "static" : "") +
                        std::string(method_name ?: "") +
                        std::string(jni_signature ?: "");
      __android_log_write(ANDROID_LOG_FATAL, "textra", msg.c_str());
    }
  }
  return id;
}
jfieldID GetField(JNIEnv* env, jclass clazz, const char* field_name,
                  const char* jni_signature) {
  jfieldID field = nullptr;
  if (clazz) {
    field = env->GetFieldID(clazz, field_name, jni_signature);
  }
  if (ClearException(env) || !field) {
    std::string msg = "Failed to find " + std::string(field_name ?: "") +
                      std::string(jni_signature ?: "");
    __android_log_write(ANDROID_LOG_FATAL, "textra", msg.c_str());
  }
  return field;
}
}  // namespace tttext
}  // namespace ttoffice
