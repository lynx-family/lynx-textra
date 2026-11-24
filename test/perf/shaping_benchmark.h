// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef TEST_PERF_SHAPING_BENCHMARK_H_
#define TEST_PERF_SHAPING_BENCHMARK_H_

#include <string>

// Test-only entry point for an iOS/Android/Harmony app host. The host
// initializes its platform runtime, calls this serially on one thread, and
// returns the JSON response to the desktop adapter. Setup/IPC is outside the
// measured interval.
int RunShapingBenchmark(const std::string& input, const std::string& metadata,
                        std::string* response);

#endif  // TEST_PERF_SHAPING_BENCHMARK_H_
