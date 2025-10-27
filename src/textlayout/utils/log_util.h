// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_LOG_UTIL_H_
#define SRC_TEXTLAYOUT_UTILS_LOG_UTIL_H_

#include <textra/macro.h>

#include <chrono>  // NOLINT [build/c++11]
#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

// TODO(hfuttyh): Because lynx had define this marco
// in some file, these marco will cause redefine, so undef here
#undef LOGF
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#undef LOGP

#define TAG "textlayout"

#ifndef TTTEXT_DEBUG
#define LOGW(...)
#define LOGI(...)
#define LOGD(...)
#ifndef ENABLE_VERBOSE_LOG
#define LOGV(...)
#endif
#endif

#ifndef PERFLOG
#define LOGP(...)
#endif

#ifdef ANDROID
#include <android/log.h>

#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#ifdef TTTEXT_DEBUG
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#ifdef ENABLE_VERBOSE_LOG
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#endif
#endif

#ifdef PERFLOG
#define LOGP(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#endif
#endif

#ifdef TTTEXT_OS_OH
#include <hilog/log.h>

#define LOGF(...) OH_LOG_Print(LOG_APP, LOG_FATAL, 0, TAG, __VA_ARGS__)
#define LOGE(...) OH_LOG_Print(LOG_APP, LOG_ERROR, 0, TAG, __VA_ARGS__)
#ifdef TTTEXT_DEBUG
#define LOGW(...) OH_LOG_Print(LOG_APP, LOG_WARN, 0, TAG, __VA_ARGS__)
#define LOGI(...) OH_LOG_Print(LOG_APP, LOG_INFO, 0, TAG, __VA_ARGS__)
#define LOGD(...) OH_LOG_Print(LOG_APP, LOG_DEBUG, 0, TAG, __VA_ARGS__)
#ifdef ENABLE_VERBOSE_LOG
#define LOGV(...) OH_LOG_Print(LOG_APP, LOG_DEBUG, 0, TAG, __VA_ARGS__)
#endif
#endif

#ifdef PERFLOG
#define LOGP(...) OH_LOG_Print(LOG_APP, LOG_DEBUG, 0, TAG, __VA_ARGS__)
#endif
#endif

#if defined(TTTEXT_OS_IOS) || defined(TTTEXT_OS_MAC)
enum class LogLevel {
  kVerbose,
  kDebug,
  kPerf,
  kInfo,
  kWarning,
  kError,
};
template <typename... Args>
static inline void PrintInternal(LogLevel level, const char* format,
                                 const Args&... args) {
  switch (level) {
    case LogLevel::kVerbose:
      PrintHead("V");
      break;
    case LogLevel::kDebug:
      PrintHead("D");
      break;
    case LogLevel::kPerf:
      PrintHead("P");
      break;
    case LogLevel::kInfo:
      PrintHead("I");
      break;
    case LogLevel::kWarning:
      PrintHead("W");
      break;
    case LogLevel::kError:
      PrintHead("E");
      break;
  }
}
static void PrintHead(const char* head) {
  using HOURS = std::chrono::hours;
  using MINUTES = std::chrono::minutes;
  using SECONDS = std::chrono::seconds;
  using MILLISECONDS = std::chrono::milliseconds;
  using SYSTEMCLOCK = std::chrono::system_clock;
  using DURATION = std::chrono::system_clock::duration;
  using DAYS = std::chrono::duration<
      int, std::ratio_multiply<HOURS::period, std::ratio<24> >::type>;
  SYSTEMCLOCK::time_point now = SYSTEMCLOCK::now();
  DURATION tp = now.time_since_epoch();
  DAYS d = std::chrono::duration_cast<DAYS>(tp);
  tp -= d;
  HOURS h = std::chrono::duration_cast<HOURS>(tp);
  tp -= h;
  MINUTES m = std::chrono::duration_cast<MINUTES>(tp);
  tp -= m;
  SECONDS s = std::chrono::duration_cast<SECONDS>(tp);
  tp -= s;
  MILLISECONDS ms = std::chrono::duration_cast<MILLISECONDS>(tp);
  tp -= ms;
  printf("[%s/%" PRId32 ":%" PRId32 ":%" PRId64 ".%" PRId64 "] ", head,
         (int32_t)(h.count() + 8), (int32_t)(m.count()), (int64_t)s.count(),
         (int64_t)ms.count());
}

#define LOGF(...) PrintInternal(LogLevel::kError, __VA_ARGS__)
#define LOGE(...) PrintInternal(LogLevel::kError, __VA_ARGS__)
#ifdef TTTEXT_DEBUG
#define LOGW(...) PrintInternal(LogLevel::kWarning, __VA_ARGS__)
#define LOGI(...) PrintInternal(LogLevel::kInfo, __VA_ARGS__)
#define LOGD(...) PrintInternal(LogLevel::kDebug, __VA_ARGS__)
#ifdef ENABLE_VERBOSE_LOG
#define LOGV(...) PrintInternal(LogLevel::kVerbose, __VA_ARGS__)
#endif
#endif
#ifdef PERFLOG
#define LOGP(...) PrintInternal(LogLevel::kPerf, __VA_ARGS__)
#endif
#endif

class L_EXPORT LogUtil {
 public:
  template <typename... Args>
  static void V(const char* format, const Args&... args) {
    LOGV(format, args...);
  }
  template <typename... Args>
  static void I(const char* format, const Args&... args) {
    LOGD(format, args...);
  }
  template <typename... Args>
  static void D(const char* format, const Args&... args) {
    LOGD(format, args...);
  }
  template <typename... Args>
  static void W(const char* format, const Args&... args) {
    LOGW(format, args...);
  }
  template <typename... Args>
  static inline void E(const char* format, const Args&... args) {
    LOGE(format, args...);
  }
  template <typename... Args>
  static inline void P(const char* format, const Args&... args) {
    LOGP(format, args...);
  }
  static void I(const char* string) { LogUtil::I("%s", string); }
  static void D(const char* string) { LogUtil::D("%s", string); }
  static void W(const char* string) { LogUtil::W("%s", string); }
  static void E(const char* string) { LogUtil::E("%s", string); }
  static clock_t Clock() { return clock(); }
  static float PerfCount(clock_t* start) {
    float ret = (clock() - *start) * 1000.f / CLOCKS_PER_SEC;
    *start = clock();
    return ret;
  }
};

#undef LOGF
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#undef LOGP
#endif  // SRC_TEXTLAYOUT_UTILS_LOG_UTIL_H_
