// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "shaper_minikin.h"

#include "font/font_helper.h"
#include "icu_substitute/bidi/bidi_wrapper.h"
#include "lib/LayoutCache.h"
#include "lib/U16StringPiece.h"
#include "run/base_run.h"
#include "style/style_manager.h"
#include "utils/u_8_string.h"

using namespace ttoffice::textlayout;
std::unique_ptr<TTShaper> TTShaper::CreateInstance() {
  return std::make_unique<Shaper_Minikin>();
}

Shaper_Minikin::Shaper_Minikin() : font_manager_(IFontManager::RefDefault()) {
  minikin::FontStyle style(minikin::FontStyle::Weight::NORMAL,
                           minikin::FontStyle::Slant::UPRIGHT);
  std::vector<std::shared_ptr<minikin::FontFamily>> font_families;
  for (int i = 0; i < font_name_lst_.size(); i++) {
    auto family =
        font_manager_->matchFamilyStyle(font_name_lst_[i].c_str(), style);
    font_families.emplace_back(family);
  }
  font_collections_ = std::make_shared<minikin::FontCollection>(font_families);
  paint_ = std::make_unique<minikin::MinikinPaint>(font_collections_);
}
Shaper_Minikin::~Shaper_Minikin() {}
const ITypefaceHelper* Shaper_Minikin::GetFontById(uint32_t glyph_id) const {
  const auto minikin_font =
      font_collections_->GetFontById(glyph_font_idx_[glyph_id]);
  auto* font_helper =
      static_cast<const minikin::FontHelper*>(minikin_font.get());
  return font_helper->GetTypeface().get();
}
void Shaper_Minikin::ProcessBidirection(const TTParagraph* paragraph) {
  auto write_direction = paragraph->GetWriteDirection();
  auto& bidi_utils = BidiWrapper::GetInstance();
  bidi_utils.SetPara(paragraph->GetContent().ToString(), write_direction);
  const auto& levels = bidi_utils.GetLevels();
  const auto& v_order = bidi_utils.GetLogicalMap();
  char_direction_vec_.reserve(paragraph->GetCharCount());
  visual_order_.reserve(paragraph->GetCharCount());
  for (int k = 0; k < paragraph->GetCharCount(); k++) {
    char_direction_vec_.push_back(levels[k] % 2 == 1);
    visual_order_.push_back(v_order[k]);
  }
}
void Shaper_Minikin::PreProcessParagraph(const TTParagraph* paragraph) {
  auto& style_manager = paragraph->GetStyleManager();
  advances_.resize(paragraph->GetCharCount(), 0);
  char_to_glyphs_map_.resize(paragraph->GetCharCount(), -1);
  for (int run_id = 0; run_id < paragraph->GetRunCount(); run_id++) {
    auto* run = paragraph->GetRun(run_id);
    auto text_size = style_manager.GetTextSize(run->GetStartCharPos());
    auto text_scale = style_manager.GetTextScale(run->GetStartCharPos());
    auto rtl = paragraph->GetCharDirection(run->GetStartCharPos());
    if (run->GetCharCount() > 0) {
      auto run_text =
          paragraph->GetText(run->GetStartCharPos(), run->GetCharCount());
      auto layout_piece = LayoutPiece(run_text.Data(), run_text.Length(), rtl,
                                      text_size * text_scale);
      auto glyphs = layout_piece.glyphIds();
      auto advances = layout_piece.advances();
      auto indices = layout_piece.glyphIndices();
      auto font_indices = layout_piece.fontIndices();
      TTASSERT(glyphs.size() == advances.size() &&
               glyphs.size() == indices.size());
      if (rtl) {
        std::reverse(glyphs.begin(), glyphs.end());
        std::reverse(advances.begin(), advances.end());
        std::reverse(indices.begin(), indices.end());
        std::reverse(font_indices.begin(), font_indices.end());
      }
      AppendShapeResult(run->GetStartCharPos(), run->GetCharCount(),
                        layout_piece.glyphCount(), advances.data(),
                        indices.data());
      glyphs_.reserve(glyphs_.size() + layout_piece.glyphCount());
      glyph_advances_.reserve(glyph_advances_.size() +
                              layout_piece.glyphCount());
      const auto& fonts = layout_piece.fonts();
      for (auto k = 0; k < layout_piece.glyphCount(); k++) {
        glyphs_.push_back(glyphs[k]);
        glyph_advances_.push_back(advances[k]);
        auto& minikin_font = fonts[font_indices[k]].font->typeface();
        glyph_font_idx_.push_back(minikin_font->GetUniqueId());
      }
    }
  }
}
TTFontInfo Shaper_Minikin::CreateFontInfo(FontIdType font_idx,
                                          float text_size) {
  TTFontInfo font_info;
  auto font = font_collections_->GetFontById(font_idx);
  if (font != nullptr) font->GetFontInfo(font_info, text_size);
  return font_info;
}
minikin::LayoutPiece Shaper_Minikin::LayoutPiece(const char* str,
                                                 uint32_t length, bool dir,
                                                 float size) {
  auto u16vec = U8String::Utf8ToUtf16(str, length, false, nullptr);
  minikin::U16StringPiece piece((const uint16_t*)u16vec.c_str(),
                                (uint32_t)u16vec.length());
  paint_->size = size;
  LayoutAppendFunctor f;
  minikin::LayoutCache::getInstance().getOrCreate(
      piece, minikin::Range(0, u16vec.length()), *paint_, dir,
      minikin::StartHyphenEdit::NO_EDIT, minikin::EndHyphenEdit::NO_EDIT, f);
  return *f.layout_piece_;
}