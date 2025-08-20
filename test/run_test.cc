// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/layout_definition.h>

#include <memory>
#include <unordered_map>

#include "paragraph_impl.h"
#include "run/base_run.h"
#include "run/ghost_run.h"
#include "test/test_utils.h"
#include "tt_shaper.h"

using namespace ttoffice::tttext;

TEST(BaseRun, Constructor) {
  ParagraphImpl paragraph;
  const Style style;
  const uint32_t start_char_pos = 0;
  const uint32_t end_char_pos = 2;
  const RunType type = RunType::kTextRun;

  BaseRun run(&paragraph, style, start_char_pos, end_char_pos, type);
  EXPECT_EQ(run.GetParagraph(), &paragraph);
  // EXPECT_EQ(run.GetLayoutStyle(), style); // no operator==
  EXPECT_FLOAT_EQ(run.GetStartCharPos(), start_char_pos);
  EXPECT_FLOAT_EQ(run.GetEndCharPos(), end_char_pos);
  EXPECT_EQ(run.GetType(), type);
}

TEST(BaseRun, AssignmentOperator) {
  ParagraphImpl paragraph;
  const Style style;
  const uint32_t start_char_pos = 0;
  const uint32_t end_char_pos = 2;
  const RunType type = RunType::kTextRun;
  BaseRun expected_run(&paragraph, style, start_char_pos, end_char_pos, type);
  BaseRun actual_run;
  actual_run = expected_run;
  // EXPECT_EQ(actual_run.GetParagraph(), &paragraph); // Should it copy?
  // EXPECT_EQ(actual_run.GetLayoutStyle(), style); // no operator==
  EXPECT_FLOAT_EQ(actual_run.GetStartCharPos(), start_char_pos);
  EXPECT_FLOAT_EQ(actual_run.GetEndCharPos(), end_char_pos);
  EXPECT_EQ(actual_run.GetType(), type);
}

TEST(BaseRun, GetTypeMethod) {
  using CheckRunTypeMethod = bool (BaseRun::*)() const;
  const std::unordered_map<RunType, CheckRunTypeMethod> map = {
      {RunType::kTextRun, &BaseRun::IsTextRun},
      {RunType::kGhostRun, &BaseRun::IsGhostRun},
      {RunType::kSpaceRun, &BaseRun::IsTextRun},
      {RunType::kTabRun, &BaseRun::IsTextRun},
      {RunType::kInlineObject, &BaseRun::IsObjectRun},
      {RunType::kFloatObject, &BaseRun::IsObjectRun},
      {RunType::kBlockStart, &BaseRun::IsBlockRun},
      {RunType::kBlockEnd, &BaseRun::IsBlockRun},
      {RunType::kControlRun, &BaseRun::IsControlRun},
      {RunType::kCRRun, &BaseRun::IsControlRun},
      {RunType::kFFRun, &BaseRun::IsControlRun},
      {RunType::kLFRun, &BaseRun::IsControlRun},
      {RunType::kNLFRun, &BaseRun::IsControlRun},
      {RunType::kCRLFRun, &BaseRun::IsControlRun},
  };
  for (const auto& pair : map) {
    ParagraphImpl paragraph;
    BaseRun run(&paragraph, Style(), 0, 2, pair.first);
    EXPECT_TRUE((run.*pair.second)());
  }
}

TEST(BaseRun, BoundaryType) {
  ParagraphImpl paragraph;
  BaseRun run(&paragraph, Style(), 0, 2, RunType::kTextRun);
  run.SetBoundaryType(BoundaryType::kWord);
  EXPECT_EQ(run.GetBoundaryType(), BoundaryType::kWord);
}

TEST(TextRun, LastNoneSpaceCharPos) {
  ParagraphImpl paragraph;
  const char content[] = " 1 3 ";
  paragraph.AddTextRun(nullptr, content);
  BaseRun run(&paragraph, Style(), 0, strlen(content), RunType::kTextRun);
  EXPECT_EQ(run.LastNoneSpaceCharPos(), 4u);
}

TEST(GhostRun, GhostContent) {
  ParagraphImpl paragraph;
  std::unique_ptr<TTShaper> shaper = TestUtils::getTestShaper();
  paragraph.SetShaper(shaper.get());
  std::u32string ghost_content(U"ghost content");
  GhostRun run(&paragraph, Style(), 0, ghost_content.c_str(),
               ghost_content.size());
  EXPECT_EQ(run.GetGhostContent().ToString(), "ghost content");
}

TEST(ObjectRun, RunDelegate) {
  ParagraphImpl paragraph;
  BaseRun run(&paragraph, Style(), 0, 2, RunType::kInlineObject);
  auto delegate = std::make_shared<TestShape>();
  run.SetRunDelegate(delegate);
  EXPECT_EQ(run.GetRunDelegate(), delegate.get());
}
