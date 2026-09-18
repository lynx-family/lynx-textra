// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>

#include <array>
#include <limits>

#include "test_utils.h"

namespace {

TEST(SkityTypefaceHelperTest, GetWidthBoundsOverwritesOutput) {
  std::array<GlyphID, 2> glyphs = {TFH_DEFAULT->UnicharToGlyph(U'A'),
                                   TFH_DEFAULT->UnicharToGlyph(U'g')};
  std::array<float, 4> bounds_from_positive_sentinel = {
      std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
  std::array<float, 4> bounds_from_negative_sentinel = {
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest()};

  TFH_DEFAULT->GetWidthBounds(bounds_from_positive_sentinel.data(),
                              glyphs.data(), glyphs.size(), 20.f);
  TFH_DEFAULT->GetWidthBounds(bounds_from_negative_sentinel.data(),
                              glyphs.data(), glyphs.size(), 20.f);

  EXPECT_EQ(bounds_from_positive_sentinel, bounds_from_negative_sentinel);
  EXPECT_LT(bounds_from_positive_sentinel[1], bounds_from_positive_sentinel[3]);
}

}  // namespace
