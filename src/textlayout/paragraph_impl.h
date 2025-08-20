// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_PARAGRAPH_IMPL_H_
#define SRC_TEXTLAYOUT_PARAGRAPH_IMPL_H_

#include <textra/paragraph.h>
#include <textra/paragraph_style.h>
#include <textra/run_delegate.h>
#include <textra/style.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/textlayout/layout_position.h"
#include "src/textlayout/utils/tt_string.h"
#include "src/textlayout/utils/tt_string_piece.h"

class ParagraphTest;

namespace ttoffice {
namespace tttext {
class RunDelegate;
class TTShaper;
class LayoutPosition;
class BaseRu;
class RunRange;
class ObjectRun;
class LayoutMeasurer;
class LayoutDrawer;
class TextLayoutImpl;
class AttachmentManager;
class StyleManager;
class LayoutMetrics;
class TextLine;
class LayoutRegion;
class RegionPosition;
class BoundaryAnalyst;
class ShaperSkShaper;
class FontmgrCollection;
class TTTextContext;
class TextLineImpl;
enum class LineBreakType : uint8_t;
enum class RunType : uint8_t;
enum class WriteDirection : uint8_t;
class ParagraphImpl : public Paragraph {
  friend BaseRun;
  friend GhostRun;
  friend LayoutDrawer;
  friend TTShaper;
  friend ShaperSkShaper;
  friend TextLayoutImpl;
  friend TextLineImpl;
  friend LayoutRegion;
  friend RegionPosition;
  friend LayoutDrawer;
  friend ParagraphTest;

 public:
  ParagraphImpl();
  ParagraphImpl(const ParagraphImpl& para) = delete;
  ~ParagraphImpl() override;

 public:
  ParagraphStyle& GetParagraphStyle() override { return paragraph_style_; }
  uint32_t GetCharCount() const override { return content_.GetCharCount(); }
  std::string GetContentString(uint32_t start_char,
                               uint32_t char_count) const override {
    return GetContent(start_char, char_count).ToString();
  }
  void SetParagraphStyle(const ParagraphStyle* paragraph_style) override {
    paragraph_style_ = *paragraph_style;
  }
  using Paragraph::AddTextRun;
  void AddTextRun(const Style* style, const char* content,
                  uint32_t length) override {
    AddTextRun(style == nullptr ? paragraph_style_.GetDefaultStyle() : *style,
               content, length, false);
  }
  void AddShapeRun(const Style* style, std::shared_ptr<RunDelegate> shape,
                   bool is_float) override {
    AddShapeRun(style == nullptr ? paragraph_style_.GetDefaultStyle() : *style,
                std::move(shape), true, is_float, 0);
  }
  void AddGhostShapeRun(const Style* style,
                        std::shared_ptr<RunDelegate> shape) override {
    AddShapeRun(style == nullptr ? paragraph_style_.GetDefaultStyle() : *style,
                std::move(shape), false, false, 0);
  }
  uint32_t GetRunCount() const override {
    return static_cast<uint32_t>(run_lst_.size());
  }
  void ApplyStyleInRange(const Style& style, CharPos start,
                         uint32_t len) const override;
  float GetMaxIntrinsicWidth() const override;
  float GetMinIntrinsicWidth() const override;

  std::pair<uint32_t, uint32_t> GetWordBoundary(uint32_t offset) const override;

 public:
  TTStringPiece GetContent(
      CharPos start_char = 0,
      uint32_t char_count = std::numeric_limits<uint32_t>::max()) const {
    if (start_char >= GetCharCount()) return content_.SubStr(0, 0);
    // prevent integer overflow
    if (char_count > GetCharCount() - start_char)
      char_count = GetCharCount() - start_char;
    return content_.SubStr(start_char, char_count);
  }
  const ParagraphStyle& GetParagraphStyle() const { return paragraph_style_; }
  const Style& GetDefaultStyle() const {
    return paragraph_style_.GetDefaultStyle();
  }
  void AddTextRun(const Style& style, const char* content, uint32_t length,
                  bool ghost_text = false);
  void AddShapeRun(const Style& style, std::shared_ptr<RunDelegate> shape,
                   bool need_placeholder = true, bool is_float = false,
                   float offset_y = 0);
  void FormatRunList();
  void FormatIndent();
  uint32_t LayoutPositionToCharPos(const LayoutPosition& pos) const;
  LayoutPosition CharPosToLayoutPosition(uint32_t char_pos) const;
  bool IsFirstCharOfParagraph(uint32_t char_pos) const;
  uint32_t GetVisualOrder(uint32_t char_pos) const {
    return char_pos < visual_map_.size() ? visual_map_[char_pos]
                                         : visual_map_.back();
  }
  bool IsRtlCharacter(CharPos pos) const {
    auto level = pos < bidi_level_.size() ? bidi_level_[pos]
                                          : bidi_level_[bidi_level_.size() - 1];
    return level % 2 == 1;
  }
  LayoutPosition FindNextBoundary(const LayoutPosition& start,
                                  const BoundaryType& type) const;
  LayoutPosition FindPrevBoundary(const LayoutPosition& start,
                                  const BoundaryType& type) const;
  BoundaryType GetBoundaryTypeBefore(const LayoutPosition& position) const;
  BoundaryType GetBoundaryType(const LayoutPosition& position) const;
  void SetShaper(TTShaper* shaper) { shaper_ = shaper; }
  void ClearLayout() { formated_ = false; }
  RunDelegate* GetRunDelegateForChar(uint32_t char_index) const;

 private:
  BaseRun* GetRun(uint32_t idx) const {
    return idx >= run_lst_.size() ? nullptr : run_lst_[idx].get();
  }
  uint32_t AddTextContent(const std::string& text) {
    if (!text.empty()) content_ += text;
    return content_.GetCharCount();
  }
  bool SplitRun(uint32_t idx, uint32_t char_pos_in_run);

#ifdef TTTEXT_DEBUG
  std::u32string GetContentWithGhost() const;
#endif

 protected:
  ParagraphStyle paragraph_style_;
  bool formated_;
  TTString content_;
  std::unique_ptr<StyleManager> style_manager_;
  std::unique_ptr<BoundaryAnalyst> boundary_analyst_;
  // even: ltr, odd: rtl
  std::vector<uint8_t> bidi_level_;
  std::vector<uint32_t> visual_map_;
  std::vector<uint32_t> logical_map_;
  std::vector<std::unique_ptr<BaseRun>> run_lst_;
  TTShaper* shaper_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_PARAGRAPH_IMPL_H_
