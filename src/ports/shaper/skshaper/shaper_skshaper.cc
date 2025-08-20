// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/skshaper/shaper_skshaper.h"

#include <utility>
#include <vector>

#include "src/ports/shaper/skshaper/one_line_shaper.h"
#include "src/ports/shaper/skshaper/run.h"
#include "src/textlayout/utils/log_util.h"

namespace ttoffice {
namespace tttext {
ShaperSkShaper::ShaperSkShaper(FontmgrCollection& font_collection)
    : TTShaper(font_collection) {
  shaper_ = std::make_unique<OneLineShaper>(font_collection_);
}
ShaperSkShaper::~ShaperSkShaper() = default;

class ShapingBlockReader : public PlatformShapingResultReader {
 public:
  ShapingBlockReader() = delete;
  explicit ShapingBlockReader(
      const std::vector<OneLineShaper::RunBlock>& blocks)
      : block_list_(blocks),
        current_block_idx_(0),
        glyph_offset_(0),
        glyph_count_(0),
        text_count_(0) {
    for (auto& block : blocks) {
      glyph_count_ += block.fGlyphs.GetLength();
      text_count_ += block.fText.GetLength();
    }
    // TTASSERT(glyph_count_ == blocks.back().fGlyphs.GetEnd());
  }

 public:
  uint32_t GlyphCount() const override { return glyph_count_; }
  uint32_t TextCount() const override { return text_count_; }
  GlyphID ReadGlyphID(uint32_t idx) const override {
    auto& block = block_list_[SeekBlock(idx)];
    auto idx_in_block = idx - glyph_offset_;
    if (!block.fRun->leftToRight()) {
      idx_in_block = block.fGlyphs.GetLength() - idx_in_block - 1;
    }
    return block.fRun->GetGlyphs()[idx_in_block + block.fGlyphs.GetStart()];
  }
  float ReadAdvanceX(uint32_t idx) const override {
    auto& block = block_list_[SeekBlock(idx)];
    auto idx_in_block = idx - glyph_offset_;
    if (!block.fRun->leftToRight()) {
      idx_in_block = block.fGlyphs.GetLength() - idx_in_block - 1;
    }
    return block.fRun->GetAdvances()[idx_in_block + block.fGlyphs.GetStart()];
  }
  uint32_t ReadIndices(uint32_t idx) const override {
    auto& block = block_list_[SeekBlock(idx)];
    auto idx_in_block = idx - glyph_offset_;
    auto offset = block.fRun->textRange().GetStart();
    if (!block.fRun->leftToRight()) {
      idx_in_block = block.fGlyphs.GetLength() - idx_in_block - 1;
      // offset = block.fRun->textRange().GetEnd();
    }
    return block.fRun->GetIndices()[idx_in_block + block.fGlyphs.GetStart()] +
           offset;
  }
  TypefaceRef ReadFontId(uint32_t idx) const override {
    auto& block = block_list_[SeekBlock(idx)];
    return block.fRun->font().GetTypeface();
  }

 private:
  uint32_t SeekBlock(uint32_t idx) const {
    const auto& current_block = block_list_[current_block_idx_];
    if (idx >= glyph_offset_ &&
        idx < glyph_offset_ + current_block.fGlyphs.GetLength()) {
      return current_block_idx_;
    }
    auto start = current_block_idx_ + 1;
    auto glyph_offset = glyph_offset_ + current_block.fGlyphs.GetLength();
    if (idx < glyph_offset) {
      start = 0;
      glyph_offset = 0;
    }
    for (; start < block_list_.size(); start++) {
      if (idx >= glyph_offset &&
          idx < glyph_offset + block_list_[start].fGlyphs.GetLength()) {
        current_block_idx_ = start;
        glyph_offset_ = glyph_offset;
        return start;
      }
      glyph_offset += block_list_[start].fGlyphs.GetLength();
    }
    TTASSERT(false);
    return 0;
  }

 private:
  const std::vector<OneLineShaper::RunBlock>& block_list_;
  mutable uint32_t current_block_idx_;
  mutable uint32_t glyph_offset_;
  uint32_t glyph_count_;
  uint32_t text_count_;
};

void ShaperSkShaper::OnShapeText(const ShapeKey& key,
                                 ShapeResult* result) const {
  shaper_->shape(key.text_.c_str(), static_cast<uint32_t>(key.text_.length()),
                 key.style_, key.rtl_);
  //  TTASSERT(shaper_->fResolvedBlocks.size() == 1);
  const ShapingBlockReader reader(shaper_->fResolvedBlocks);
  result->AppendPlatformShapingResult(reader);
}
}  // namespace tttext
}  // namespace ttoffice
