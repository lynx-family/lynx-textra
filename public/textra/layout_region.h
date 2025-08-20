// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_LAYOUT_REGION_H_
#define PUBLIC_TEXTRA_LAYOUT_REGION_H_

#include <textra/layout_page_listener.h>
#include <textra/macro.h>
#include <textra/text_line.h>
#include <textra/tttext_context.h>

#include <memory>
#include <utility>
#include <vector>

namespace ttoffice {
namespace tttext {
class FontInfo;
class LineRange;
class ParagraphImpl;
class TextLine;
class LayoutDrawer;
class TextLayout;
// class BlockRegion;
class AttributesRangeList;
class LayoutRegionDistribute;
enum class ParagraphHorizontalAlignment : uint8_t;
enum class LayoutMode : uint8_t;

/**
 * @brief Defines a rectangular area for text layout and stores layout results.
 * Also used by LayoutDrawer to render the laid-out content.
 *
 * LayoutRegion serves three main purposes:
 * 1. Defines the dimensions and constraints of the layout area (e.g., width,
 * height, layout modes)
 * 2. Stores the results of layout operations (e.g., lines of text, paragraphs)
 * 3. Acts as a parameter for LayoutDrawer to render the content
 *
 * It works closely with TextLayout to perform layout operations, and with
 * LayoutDrawer for rendering. It also supports special cases like floating
 * elements that affect text flow.
 */
class L_EXPORT LayoutRegion {
  friend TextLayoutImpl;
  friend LayoutDrawer;
  friend class LayoutRegionDistribute;

 public:
  explicit LayoutRegion(float width, float height,
                        LayoutMode width_mode = LayoutMode::kDefinite,
                        LayoutMode height_mode = LayoutMode::kDefinite,
                        LayoutPageListener* listener = nullptr);
  LayoutRegion() = delete;
  virtual ~LayoutRegion();

 public:
  virtual std::vector<std::array<float, 2>> GetRangeList(float* top,
                                                         float range_height,
                                                         float start_indent,
                                                         float end_indent);
  virtual std::pair<LayoutResult, bool> ProcessFloatObject(
      const TTTextContext& context, const BaseRun& run, int char_x,
      float line_y);

 public:
  LayoutMode GetWidthMode() const { return width_mode_; }
  LayoutMode GetHeightMode() const { return height_mode_; }
  float GetPageWidth() const { return size_.first; }
  float GetPageHeight() const { return size_.second; }
  LayoutResult AddLine(std::unique_ptr<TextLine> line,
                       const TTTextContext& context);
  void UpdateLayoutedSize(TextLine* line, const TTTextContext& context);
  float GetLayoutedWidth() const { return layouted_width_; }
  float GetLayoutedHeight() const { return layouted_bottom_; }

  uint32_t GetLineCount() const {
    return static_cast<uint32_t>(line_lst_.size());
  }
  void SetLinePitch(float pitch) { line_pitch_ = pitch; }
  float GetLinePitch() const { return line_pitch_; }
  TextLine* GetLine(uint32_t idx) const {
    if (idx >= line_lst_.size()) return nullptr;
    return line_lst_[idx].get();
  }
  void SetFullFilled(bool full) { full_filled_ = full; }
  bool IsFullFilled() const { return full_filled_; }
  bool IsEmpty() const { return GetLineCount() == 0; }

  void SetExceededMaxLines(bool exceeded_max_lines) {
    exceeded_max_lines_ = exceeded_max_lines;
  }
  bool DidExceedMaxLines() const { return exceeded_max_lines_; }

 protected:
  std::vector<Paragraph*> paragraph_list_;
  std::vector<std::unique_ptr<TextLine>> line_lst_;
  std::pair<float, float> size_;  // Allowed layout area width and height
  LayoutMode width_mode_;
  LayoutMode height_mode_;
  /**
   * A structure ported from word layout that represents the line height of the
   * document grid, calculated based on page height and number of lines per
   * page. For other scenarios where this is not needed, it can be set to 0.
   */
  float line_pitch_;
  bool full_filled_;
  bool exceeded_max_lines_ = false;
  float layouted_width_;
  float layouted_bottom_;
  LayoutPageListener* listener_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_LAYOUT_REGION_H_
