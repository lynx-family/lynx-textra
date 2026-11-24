// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <jni.h>

#include <string>

#include "test/perf/shaping_benchmark.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_lynx_textra_perf_PerfActivity_nativeRun(JNIEnv* env, jclass,
                                                 jbyteArray input,
                                                 jbyteArray meta) {
  std::string request(static_cast<size_t>(env->GetArrayLength(input)), '\0');
  std::string metadata(static_cast<size_t>(env->GetArrayLength(meta)), '\0');
  env->GetByteArrayRegion(input, 0, env->GetArrayLength(input),
                          reinterpret_cast<jbyte*>(request.data()));
  env->GetByteArrayRegion(meta, 0, env->GetArrayLength(meta),
                          reinterpret_cast<jbyte*>(metadata.data()));
  if (env->ExceptionCheck()) return nullptr;
  std::string response;
  const int status = RunShapingBenchmark(request, metadata, &response);
  if (status)
    response =
        "{\"error\":\"benchmark failed\",\"code\":" + std::to_string(status) +
        "}";
  return env->NewStringUTF(response.c_str());
}
