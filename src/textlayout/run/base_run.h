// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_RUN_BASE_RUN_H_
#define SRC_TEXTLAYOUT_RUN_BASE_RUN_H_

#include <textra/font_info.h>
#include <textra/macro.h>
#include <textra/run_delegate.h>
#include <textra/style.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/run/layout_metrics.h"
#include "src/textlayout/shape_cache.h"
#include "src/textlayout/utils/tt_range.h"
#include "src/textlayout/utils/tt_string.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
class ICanvasHelper;
class Painter;
class FontInfo;
class ParagraphImpl;
class Style;
class TextAttachment;
class TTStringPiece;
class ShapeResult;
class LayoutMeasurer;
enum class RunType : uint8_t;
enum class LineBreakType : uint8_t;
enum class CharacterVerticalAlignment : uint8_t;
enum class BoundaryType : uint8_t;
class BaseRun {
  friend ParagraphImpl;
  friend TextLineImpl;
  friend class LayoutDrawer;

 public:
  static constexpr const char* ObjectReplacementCharacter() {
    return u8"\ufffc";
  }

  static std::pair<RunType, const char*>* ControlRunList();
  static const char** SpaceList();

 public:
  BaseRun() = default;
  explicit BaseRun(ParagraphImpl* paragraph, const Style& style,
                   uint32_t start_char_pos, uint32_t end_char_pos,
                   RunType type) {
    this->Init(paragraph, style, start_char_pos, end_char_pos, type);
  }
  virtual ~BaseRun() = default;

 public:
  void Init(ParagraphImpl* paragraph, const Style& style,
            uint32_t start_char_pos, uint32_t end_char_pos, RunType type) {
    paragraph_ = paragraph;
    start_char_pos_ = start_char_pos;
    end_char_pos_ = end_char_pos;
    run_type_ = type;
    layout_style_ = style;
    ghost_content_ = "";
    UpdateRunContent();
  }
  ParagraphImpl* GetParagraph() const { return paragraph_; }
  uint32_t GetStartCharPos() const { return start_char_pos_; }
  uint32_t GetEndCharPos() const { return end_char_pos_; }
  uint32_t GetCharCount() const { return end_char_pos_ - start_char_pos_; }
  RunType GetType() const { return run_type_; }
  uint32_t GetVisualOrderIndex() const {
    return paragraph_->GetVisualOrder(GetStartCharPos());
  }
  bool IsControlRun() const { return run_type_ >= RunType::kControlRun; }
  bool IsTextRun() const {
    return GetType() == RunType::kTextRun || GetType() == RunType::kSpaceRun ||
           GetType() == RunType::kTabRun;
  }
  //  bool IsSpaceRun() const { return GetType() == RunType::kSpaceRun; };
  bool IsGhostRun() const {
    // ObjectRun without placeholder is a type of ghostrun
    return GetType() == RunType::kGhostRun ||
           ((GetType() == RunType::kInlineObject ||
             GetType() == RunType::kFloatObject) &&
            GetCharCount() == 0);
  }
  bool IsBlockRun() const {
    return GetType() == RunType::kBlockEnd || GetType() == RunType::kBlockStart;
  }
  bool IsObjectRun() const {
    return GetType() == RunType::kFloatObject ||
           GetType() == RunType::kInlineObject;
  }
  void UpdateRunContent();
  float GetWidth(uint32_t char_start_in_run) const {
    return GetWidth(char_start_in_run, GetCharCount() - char_start_in_run);
  }

  uint32_t LastNoneSpaceCharPos() const {
    auto ret = GetEndCharPos();
    while (--ret >= GetStartCharPos()) {
      if (!base::IsSpaceChar(paragraph_->content_[ret])) break;
    }
    return ret + 1;
  }
  const LayoutMetrics& GetMetrics() const { return metrics_; }
  BoundaryType GetBoundaryType() const { return boundary_type_; }
  void SetBoundaryType(BoundaryType type) { boundary_type_ = type; }
  TTStringPiece GetGhostContent() const { return ghost_content_.ToPiece(); }

 public:
  BaseRun& operator=(const BaseRun& run) {
    if (this != &run) {
      this->run_type_ = run.run_type_;
      this->start_char_pos_ = run.start_char_pos_;
      this->end_char_pos_ = run.end_char_pos_;
      this->boundary_type_ = run.boundary_type_;
    }
    return *this;
  }
  void SetRunDelegate(std::shared_ptr<RunDelegate> delegate) {
    TTASSERT(GetType() == RunType::kInlineObject ||
             GetType() == RunType::kFloatObject);
    if (delegate == nullptr) return;
    delegate_ = std::move(delegate);
    delegate_->Layout();
  }
  RunDelegate* GetRunDelegate() const { return delegate_.get(); }
  bool IsRtl() const {
    return IsTextRun() ? paragraph_->IsRtlCharacter(GetStartCharPos()) : false;
  }

 public:
  float GetWidth(uint32_t char_start_in_run, uint32_t char_count) const;
  void Layout();
  /**
   * Calculate how many characters starting from char_start_in_run have a total
   * width not exceeding max_width.
   * @param break_pos_in_run
   * @param max_width
   * @return the number of characters (char_end_in_run) whose width does not
   * exceed max_width.
   */
  float MeasureRunByWidth(uint32_t& break_pos_in_run, float max_width) const;
  const Style& GetLayoutStyle() const { return layout_style_; }
  const ShapeStyle& GetShapeStyle() const {
    return layout_style_.GetShapeStyle();
  }
  bool CanBeAppendToShaping(const BaseRun& prev_run) const {
    TTASSERT(!prev_run.IsGhostRun() && !prev_run.IsBlockRun());
    if (&prev_run == this) return true;
    if (IsGhostRun() || IsBlockRun() || IsObjectRun() || prev_run.IsObjectRun())
      return false;
    return prev_run.IsRtl() == IsRtl() &&
           prev_run.layout_style_.GetShapeStyle() ==
               layout_style_.GetShapeStyle();
  }

 protected:
  ParagraphImpl* paragraph_{};
  uint32_t start_char_pos_{};
  uint32_t end_char_pos_{};
  RunType run_type_{};
  ShapeResultPiece shape_result_{};
  LayoutMetrics metrics_{};
  Style layout_style_;
  TTString ghost_content_;
  std::shared_ptr<RunDelegate> delegate_{nullptr};
  BoundaryType boundary_type_ = BoundaryType::kNone;
  float baseline_offset_ = 0;
#ifdef TTTEXT_DEBUG

 public:
  std::u32string run_content_;
#endif
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_RUN_BASE_RUN_H_
