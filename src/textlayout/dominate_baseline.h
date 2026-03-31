// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_DOMINATE_BASELINE_H_
#define SRC_TEXTLAYOUT_DOMINATE_BASELINE_H_

#include <cstdint>
#include <utility>

namespace ttoffice {
namespace tttext {
enum class DominantBaseline : uint8_t;
class BaseRun;
class TextLineImpl;

class DominateBaselineHelper {
 public:
  static void ApplyDominateBaseline(TextLineImpl* line);

 private:
  static float CalcBaselineOffset(DominantBaseline baseline, float ascent,
                                  float descent);
  static std::pair<float, float> CalcRunEmHeight(const BaseRun* run);
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_DOMINATE_BASELINE_H_
