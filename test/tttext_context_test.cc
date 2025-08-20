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
  static const LayoutPosition& GetPositionRef(TTTextContext& context) {
    return context.GetPositionRef();
  }
  static void Reset(TTTextContext& context) { context.Reset(); }
  static void ResetLayoutPosition(TTTextContext& context,
                                  const LayoutPosition& position) {
    context.ResetLayoutPosition(position);
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

TEST_F(TTTextContextTest, Reset) {
  TTTextContext context;

  const LayoutPosition new_position(5, 10);
  const float new_layout_bottom = 20.f;
  TTTextContextTest::ResetLayoutPosition(context, new_position);
  TTTextContextTest::SetLayoutBottom(context, new_layout_bottom);
  EXPECT_EQ(TTTextContextTest::GetPositionRef(context), new_position);
  EXPECT_EQ(TTTextContextTest::GetLayoutBottom(context), new_layout_bottom);

  TTTextContextTest::Reset(context);
  EXPECT_EQ(TTTextContextTest::GetPositionRef(context), LayoutPosition(0, 0));
  EXPECT_EQ(TTTextContextTest::GetLayoutBottom(context), 0.f);
}

TEST_F(TTTextContextTest, ResetLayoutPosition) {
  TTTextContext context;
  EXPECT_EQ(TTTextContextTest::GetPositionRef(context), LayoutPosition(0, 0));

  LayoutPosition new_position(5, 10);
  TTTextContextTest::ResetLayoutPosition(context, new_position);
  EXPECT_EQ(TTTextContextTest::GetPositionRef(context), new_position);
}

}  // namespace tttext
}  // namespace ttoffice
