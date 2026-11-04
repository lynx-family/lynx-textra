// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_SINGLE_LINE_PARAGRAPH_IMPL_H_
#define SRC_TEXTLAYOUT_SINGLE_LINE_PARAGRAPH_IMPL_H_

#include <textra/paragraph.h>

#include <memory>
#include <string>
#include <utility>

#include "src/textlayout/style/style_impl.h"
#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
class BoundaryAnalyst;
// One text/style snapshot and its shaped result. Never creates ParagraphImpl,
// runs or line data, and never retains a shaper. Boundary analysis is lazy.
class L_HIDDEN SingleLineParagraphImpl final : public Paragraph {
 public:
  SingleLineParagraphImpl();
  ~SingleLineParagraphImpl() override;
  ParagraphType GetType() const override { return ParagraphType::kSingleLine; }
  float GetWidth() const override { return width; }
  float GetHeight() const override { return baseline + descent; }
  float GetBaseline() const override { return baseline; }
  float GetAscent() const override { return baseline; }
  float GetDescent() const override { return descent; }
  uint32_t GetCharCount() const override;
  uint32_t GetRunCount() const override;
  std::string GetContentString(uint32_t start, uint32_t count) const override;
  ParagraphStyle& GetParagraphStyle() override;
  void SetParagraphStyle(const ParagraphStyle* style) override;
  void AddTextRun(const Style* style, const char* text,
                  uint32_t length) override;
  void AddShapeRun(const Style* style, std::shared_ptr<RunDelegate> shape,
                   bool is_float) override;
  void AddGhostShapeRun(const Style* style,
                        std::shared_ptr<RunDelegate> shape) override;
  void ApplyStyleInRange(const Style& style, uint32_t start,
                         uint32_t length) const override;
  void SaveStyle() override;
  void RestoreStyle() override;
  float GetMaxIntrinsicWidth() const override;
  float GetMinIntrinsicWidth() const override;
  std::pair<uint32_t, uint32_t> GetWordBoundary(uint32_t offset) const override;
  std::pair<uint32_t, uint32_t> GetGraphemeBoundary(
      uint32_t offset) const override;
  void QueryStyle(uint32_t offset, Style* style) override;
  WriteDirection GetResolvedWriteDirection() const override;
  bool HasUnsupportedOperations() const { return unsupported_; }
  void Invalidate() const;

  StyleImpl style;
  StyleImpl input_style;
  std::u32string text;
  mutable std::shared_ptr<const ShapeResult> shaped;
  mutable float width = 0;
  mutable float baseline = 0;
  mutable float descent = 0;
  float skew_extra = 0;

 private:
  void RejectOperation(const char* operation) const;
  const BoundaryAnalyst& GetBoundaries() const;
  mutable bool unsupported_ = false;
  mutable std::unique_ptr<BoundaryAnalyst> boundaries_;
  // Required only to return a reference from the unsupported mutable getter.
  // Its values never participate in single-line layout.
  std::unique_ptr<ParagraphStyle> rejected_paragraph_style_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_SINGLE_LINE_PARAGRAPH_IMPL_H_
