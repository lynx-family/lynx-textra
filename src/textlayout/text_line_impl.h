// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_TEXT_LINE_IMPL_H_
#define SRC_TEXTLAYOUT_TEXT_LINE_IMPL_H_

#include <textra/text_line.h>

#include <memory>
#include <vector>

#include "src/textlayout/internal/run_range.h"
#include "src/textlayout/layout_position.h"
#include "src/textlayout/utils/tt_string_piece.h"
namespace ttoffice {
namespace tttext {
enum class LineType : uint8_t;
using DrawerPiece = RunRange;
class TextLineImpl : public TextLine {
  friend LayoutRegion;
  friend RegionPosition;
  friend TextLayoutImpl;
  friend LayoutDrawer;
  friend class CompressRowLine;

 public:
  TextLineImpl() = delete;
  explicit TextLineImpl(ParagraphImpl* paragraph, LayoutRegion* lp,
                        const LayoutPosition& pos);
  ~TextLineImpl() override;

 private:
  void SetRangeLst(const std::vector<std::array<float, 2>>& lst);
  void SplitToWordDrawer(const LineRange& line_range, float word_spacing);
  void CreateDrawerPiece();
  void InsertDrawerPiece(std::unique_ptr<DrawerPiece> drawer_piece);

 public:
  LayoutPosition UpdateLine(LayoutPosition pos, float max_ascent,
                            float max_descent, float desired_height);
  LineRange* GetCurrentRange() const {
    return range_lst_[current_available_range_index_].get();
  }
  bool IsEqualRangeList(const std::vector<std::array<float, 2>>& list) const;
  bool IsEmpty() const override;
  float GetLineLeft() const override;
  float GetLineRight() const override;

  LayoutPosition GetStartLayoutPosition() const { return line_start_pos_; }
  LayoutPosition GetEndLayoutPosition() const { return line_end_pos_; }
  CharPos GetStartCharPos() const override;
  CharPos GetEndCharPos() const override;

  void ClearForRelayout();
  bool IsLastLineOfParagraph() const override;
  ParagraphImpl* GetParagraph() const override { return paragraph_; }
  void StripByEllipsis(const char32_t* ellipsis) override;
  uint32_t GetRangeCount() const {
    return static_cast<uint32_t>(range_lst_.size());
  }
  LineRange* GetLineRange(const uint32_t idx) const {
    return range_lst_[idx].get();
  }
  void UpdateXMax(float width) override;
  void ApplyAlignment() override;
  void ApplyAlignment(ParagraphHorizontalAlignment h_align);
  void ModifyHorizontalAlignment(ParagraphHorizontalAlignment h_align) override;
  bool StripContentByWidth(float space);
  void AppendGhostRun(std::unique_ptr<BaseRun> ghost_run);
  TTStringPiece GetText() const;

 public:
  void GetBoundingRectByCharRange(float bounding_rect[4],
                                  CharPos start_char_pos,
                                  CharPos end_char_pos) const override;
  CharPos GetCharPosByCoordinateX(float x) const override;

 private:
  ParagraphImpl* paragraph_;
  LayoutRegion* layout_page_;
  LayoutPosition line_start_pos_{0, 0};
  LayoutPosition line_end_pos_{0, 0};
  int current_available_range_index_ = -1;
  std::vector<std::unique_ptr<LineRange>> range_lst_;
  std::vector<std::unique_ptr<DrawerPiece>> drawer_list_;
  std::vector<std::unique_ptr<BaseRun>> extra_contents_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_TEXT_LINE_IMPL_H_
