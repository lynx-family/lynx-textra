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

enum class LogLevel {
  kVerbose,
  kDebug,
  kPerf,
  kInfo,
  kWarning,
  kError,
};

#define ENABLE_LOG

// TODO(hfuttyh): Because lynx had define this marco
// in some file, these marco will cause redefine, so undef here
#undef LOGF
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#undef LOGP

#ifdef ANDROID
#ifdef TTOFFICE_TEXT_LAYOUT_ONLY
// TODO(hfuttyh): add android logging support when building with lynx
#define LOGD(...) printf(__VA_ARGS__)
#define LOGW(...) printf(__VA_ARGS__)
#define LOGE(...) printf(__VA_ARGS__)
#define LOGV(...) printf(__VA_ARGS__)
#else
#include <android/log.h>
#define TAG "textlayout"

#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#if TTTEXT_DEBUG
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGW(...)
#define LOGI(...)
#define LOGD(...)
#endif

#if TTTEXT_DEBUG && ENABLE_VERBOSE_LOG
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGV(...)
#endif

#if PERFLOG
#define LOGP(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGP(...)
#endif
#endif
#endif

class L_EXPORT LogUtil {
 public:
  template <typename... Args>
  static void V(const char* format, const Args&... args) {
#ifdef ANDROID
    LOGV(format, args...);
#elif VERBOSE_LOG
    LogUtil::PrintInternal(LogLevel::kInfo, format, args...);
#endif
  }
  template <typename... Args>
  static void I(const char* format, const Args&... args) {
#ifdef ANDROID
    LOGD(format, args...);
#elif TTTEXT_DEBUG
    LogUtil::PrintInternal(LogLevel::kInfo, format, args...);
#endif
  }
  template <typename... Args>
  static void D(const char* format, const Args&... args) {
#ifdef ANDROID
    LOGD(format, args...);
#elif TTTEXT_DEBUG
    LogUtil::PrintInternal(LogLevel::kDebug, format, args...);
#endif
  }
  template <typename... Args>
  static void W(const char* format, const Args&... args) {
#ifdef ANDROID
    LOGW(format, args...);
#elif TTTEXT_DEBUG
    LogUtil::PrintInternal(LogLevel::kWarning, format, args...);
#endif
  }
  template <typename... Args>
  static inline void E(const char* format, const Args&... args) {
#ifdef ANDROID
    LOGE(format, args...);
#else
    LogUtil::PrintInternal(LogLevel::kError, format, args...);
#endif
  }
  template <typename... Args>
  static inline void P(const char* format, const Args&... args) {
#ifdef ANDROID
    LOGP(format, args...);
#elif PERF_LOG
    LogUtil::PrintInternal(LogLevel::kPerf, format, args...);
#endif
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

 private:
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
#ifdef ENABLE_LOG
    printf(format, args...);
    if (format[strlen(format) - 1] != '\n') printf("\n");
#endif
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
};

#undef LOGF
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#undef LOGP
#endif  // SRC_TEXTLAYOUT_UTILS_LOG_UTIL_H_
