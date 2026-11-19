// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// Without shaper implementation definitions every backend is unavailable.
#include <textra/platform_helper.h>

#include <cstdio>
#include <cstdlib>

#define CHECK(condition)            \
  do {                              \
    if (!(condition)) std::abort(); \
  } while (0)

int main() {
  using ttoffice::tttext::PlatformHelper;
  using ttoffice::tttext::RenderingBackend;
  for (auto platform :
       {RenderingBackend::kAndroid, RenderingBackend::kIOS,
        RenderingBackend::kMacOS, RenderingBackend::kOHOS,
        RenderingBackend::kLVGL, RenderingBackend::kSkia,
        RenderingBackend::kSkity, RenderingBackend::kSkityCoreText,
        RenderingBackend::kIOSSelfRendering,
        RenderingBackend::kMacOSSelfRendering, static_cast<RenderingBackend>(0),
        static_cast<RenderingBackend>(999)}) {
    CHECK(!PlatformHelper::IsBackendAvailable(platform));
  }
  std::puts("Disabled shaping backends are unavailable");
}
