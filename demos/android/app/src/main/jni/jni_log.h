// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_ANDROID_APP_SRC_MAIN_JNI_JNI_LOG_H_
#define DEMOS_ANDROID_APP_SRC_MAIN_JNI_JNI_LOG_H_

#include <android/log.h>
#define TAG "textlayout"

#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#if DEBUG
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGW(...)
#define LOGI(...)
#define LOGD(...)
#endif

#if DEBUG && ENABLE_VERBOSE_LOG
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGV(...)
#endif

#if PERFLOG
#define LOGP(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGP(...)
#endif

#endif  // DEMOS_ANDROID_APP_SRC_MAIN_JNI_JNI_LOG_H_
