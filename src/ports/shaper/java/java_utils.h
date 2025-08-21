// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_JAVA_JAVA_UTILS_H_
#define SRC_PORTS_SHAPER_JAVA_JAVA_UTILS_H_

#include <android/log.h>
#include <jni.h>

namespace ttoffice {
namespace tttext {
bool HasException(JNIEnv* env);

bool ClearException(JNIEnv* env);

jclass GetClass(JNIEnv* env, const char* class_name);
enum MethodType {
  STATIC_METHOD,
  INSTANCE_METHOD,
};
jmethodID GetMethod(JNIEnv* env, jclass clazz, MethodType type,
                    const char* method_name, const char* jni_signature);
jfieldID GetField(JNIEnv* env, jclass clazz, const char* field_name,
                  const char* jni_signature);
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_JAVA_JAVA_UTILS_H_
