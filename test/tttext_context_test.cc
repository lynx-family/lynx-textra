// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/tttext_context.h>

#include "src/textlayout/layout_position.h"

using namespace ttoffice::tttext;
using namespace ::testing;

namespace ttoffice {
namespace tttext {

/**
 * @brief Use friend relationship to access private functions of TTTextContext.
 */
class TTTextContextTest : public ::testing::Test {
 public:
  static std::pair<uint32_t, uint32_t> GetLayoutPosition(
      TTTextContext& context) {
    return context.GetLayoutPosition();
  }
  static void Reset(TTTextContext& context) { context.Reset(); }
  static void SetLayoutPosition(TTTextContext& context, uint32_t run_idx,
                                uint32_t char_idx_in_run) {
    context.SetLayoutPosition(run_idx, char_idx_in_run);
  }
  static float GetLayoutBottom(TTTextContext& context) {
    return context.GetLayoutBottom();
  }
  static void SetLayoutBottom(TTTextContext& context, float layout_bottom) {
    context.SetLayoutBottom(layout_bottom);
  }
};

TEST_F(TTTextContextTest, ConfigurationGettersAndSetters) {
  TTTextContext context;
  EXPECT_EQ(context.IsLastLineCanOverflow(), true);
  EXPECT_EQ(context.IsSkipSpacingBeforeFirstLine(), false);

  context.SetLastLineCanOverflow(false);
  context.SetSkipSpacingBeforeFirstLine(true);

  EXPECT_EQ(context.IsLastLineCanOverflow(), false);
  EXPECT_EQ(context.IsSkipSpacingBeforeFirstLine(), true);
}

TEST_F(TTTextContextTest, ShapeCacheModeDefaultsToGlobalAndCanBeChanged) {
  TTTextContext context;

  EXPECT_EQ(context.GetShapeCacheMode(), ShapeCacheMode::kGlobal);

  context.SetShapeCacheMode(ShapeCacheMode::kInstance);
  EXPECT_EQ(context.GetShapeCacheMode(), ShapeCacheMode::kInstance);

  context.SetShapeCacheMode(ShapeCacheMode::kDisabled);
  EXPECT_EQ(context.GetShapeCacheMode(), ShapeCacheMode::kDisabled);
}

TEST_F(TTTextContextTest, Reset) {
  TTTextContext context;

  const float new_layout_bottom = 20.f;
  context.SetShapeCacheMode(ShapeCacheMode::kInstance);
  TTTextContextTest::SetLayoutPosition(context, 5, 10);
  TTTextContextTest::SetLayoutBottom(context, new_layout_bottom);
  auto result = context.GetLayoutPosition();
  EXPECT_EQ(result.first, 5);
  EXPECT_EQ(result.second, 10);
  EXPECT_EQ(TTTextContextTest::GetLayoutBottom(context), new_layout_bottom);

  TTTextContextTest::Reset(context);
  result = context.GetLayoutPosition();
  EXPECT_EQ(result.first, 0);
  EXPECT_EQ(result.second, 0);
  EXPECT_EQ(TTTextContextTest::GetLayoutBottom(context), 0.f);
  EXPECT_EQ(context.GetShapeCacheMode(), ShapeCacheMode::kInstance);
}

TEST_F(TTTextContextTest, LegacyShapeCacheFeatureUsesLastCall) {
  TTTextContext context;

  context.SetShapeCacheMode(ShapeCacheMode::kInstance);
  context.EnableFeature(kDisableShapeCache, true);
  EXPECT_EQ(context.GetShapeCacheMode(), ShapeCacheMode::kDisabled);

  context.EnableFeature(kDisableShapeCache, false);
  EXPECT_EQ(context.GetShapeCacheMode(), ShapeCacheMode::kGlobal);

  context.SetShapeCacheMode(ShapeCacheMode::kInstance);
  EXPECT_EQ(context.GetShapeCacheMode(), ShapeCacheMode::kInstance);
}

TEST_F(TTTextContextTest, ResetLayoutPosition) {
  TTTextContext context;
  auto result = context.GetLayoutPosition();
  EXPECT_EQ(result.first, 0);
  EXPECT_EQ(result.second, 0);

  TTTextContextTest::SetLayoutPosition(context, 5, 10);
  result = context.GetLayoutPosition();
  EXPECT_EQ(result.first, 5);
  EXPECT_EQ(result.second, 10);
}

}  // namespace tttext
}  // namespace ttoffice
