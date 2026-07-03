// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/ark_graphics/shaper_ark_graphics.h"

#include <dlfcn.h>
#include <native_drawing/drawing_font.h>
#include <native_drawing/drawing_font_mgr.h>
#include <native_drawing/drawing_point.h>
#include <native_drawing/drawing_text_font_descriptor.h>
#include <native_drawing/drawing_text_line.h>
#include <native_drawing/drawing_text_lineTypography.h>
#include <native_drawing/drawing_text_run.h>
#include <native_drawing/drawing_text_typography.h>
#include <textra/i_typeface_helper.h>
#include <textra/platform/ark_graphics/ag_typeface_helper.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "src/textlayout/utils/tt_string.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {

// clang-format off
using OHOS_CreateLineTypography = OH_Drawing_LineTypography* (*)(OH_Drawing_TypographyCreate*);
using OHOS_LineTypographyCreateLine = OH_Drawing_TextLine* (*)(OH_Drawing_LineTypography*, size_t, size_t);
using OHOS_TextLineGetGlyphCount = double (*)(OH_Drawing_TextLine*);
using OHOS_TextLineGetGlyphRuns = OH_Drawing_Array* (*)(OH_Drawing_TextLine*);
using OHOS_GetDrawingArraySize = size_t (*)(OH_Drawing_Array*);
using OHOS_GetRunByIndex = OH_Drawing_Run* (*)(OH_Drawing_Array*, size_t);
using OHOS_GetRunGlyphCount = uint32_t (*)(OH_Drawing_Run*);
using OHOS_GetRunGlyphs = OH_Drawing_Array* (*)(OH_Drawing_Run*, int64_t, int64_t);
using OHOS_GetRunGlyphAdvances = OH_Drawing_Array* (*)(OH_Drawing_Run*, uint32_t, uint32_t);
using OHOS_GetRunPositions = OH_Drawing_Array* (*)(OH_Drawing_Run*, int64_t, int64_t);
using OHOS_GetRunStringIndices = OH_Drawing_Array* (*)(OH_Drawing_Run*, int64_t, int64_t);
using OHOS_GetRunFont = OH_Drawing_Font* (*)(OH_Drawing_Run*);
using OHOS_GetRunGlyphsByIndex = uint16_t (*)(OH_Drawing_Array*, size_t);
using OHOS_GetRunPositionsByIndex = OH_Drawing_Point* (*)(OH_Drawing_Array*, size_t);
using OHOS_GetRunStringIndicesByIndex = uint64_t (*)(OH_Drawing_Array*, size_t);
using OHOS_GetRunGlyphAdvanceByIndex = OH_Drawing_Point* (*)(OH_Drawing_Array*, size_t);
using OHOS_DestroyRunStringIndices = void (*)(OH_Drawing_Array*);
using OHOS_DestroyRunPositions = void (*)(OH_Drawing_Array*);
using OHOS_DestroyRunGlyphs = void (*)(OH_Drawing_Array*);
using OHOS_DestroyRuns = void (*)(OH_Drawing_Array*);
using OHOS_DestroyTextLine = void (*)(OH_Drawing_TextLine*);
using OHOS_DestroyLineTypography = void (*)(OH_Drawing_LineTypography*);
using OHOS_TypographyHandlerAddEncodedText = void (*)(OH_Drawing_TypographyCreate*, const void*, size_t, OH_Drawing_TextEncoding);
using OHOS_FontSetThemeFontFollowed = OH_Drawing_ErrorCode (*)(OH_Drawing_Font*, bool);
using OHOS_GetFontCollectionGlobalInstance = OH_Drawing_FontCollection* (*)();
using OHOS_TypographyGetTextLines = OH_Drawing_Array* (*)(OH_Drawing_Typography* typography);
using OHOS_DestroyTextLines = void (*)(OH_Drawing_Array* lines);
using OHOS_GetTextLineByIndex = OH_Drawing_TextLine* (*)(OH_Drawing_Array* lines, size_t index);
using OHOS_TypographyGetCharacterRangeForGlyphRangeWithBuffer = OH_Drawing_Range* (*)(OH_Drawing_Typography* typography, size_t glyphRangeStart, size_t glyphRangeEnd, OH_Drawing_Range** actualGlyphRange, OH_Drawing_TextEncoding textEncodingType);
using OHOS_ReleaseRangeBuffer = void (*)(OH_Drawing_Range* range);
// clang-format on

struct OHOS_ShapingFuncPtr {
  OHOS_CreateLineTypography CreateLineTypography_;
  OHOS_LineTypographyCreateLine LineTypographyCreateLine_;
  OHOS_TextLineGetGlyphCount TextLineGetGlyphCount_;
  OHOS_TextLineGetGlyphRuns TextLineGetGlyphRuns_;
  OHOS_GetDrawingArraySize GetDrawingArraySize_;
  OHOS_GetRunByIndex GetRunByIndex_;
  OHOS_GetRunGlyphCount GetRunGlyphCount_;
  OHOS_GetRunGlyphs GetRunGlyphs_;
  OHOS_GetRunGlyphAdvances GetRunGlyphAdvances_;
  OHOS_GetRunPositions GetRunPositions_;
  OHOS_GetRunStringIndices GetRunStringIndices_;
  OHOS_GetRunFont GetRunFont_;
  OHOS_GetRunGlyphsByIndex GetRunGlyphsByIndex_;
  OHOS_GetRunPositionsByIndex GetRunPositionsByIndex_;
  OHOS_GetRunStringIndicesByIndex GetRunStringIndicesByIndex_;
  OHOS_GetRunGlyphAdvanceByIndex GetRunGlyphAdvanceByIndex_;
  OHOS_DestroyRunStringIndices DestroyRunStringIndices_;
  OHOS_DestroyRunPositions DestroyRunPositions_;
  OHOS_DestroyRunGlyphs DestroyRunGlyphs_;
  OHOS_DestroyRuns DestroyRuns_;
  OHOS_DestroyTextLine DestroyTextLine_;
  OHOS_DestroyLineTypography DestroyLineTypography_;
  OHOS_TypographyHandlerAddEncodedText TypographyHandlerAddEncodedText_;
  OHOS_FontSetThemeFontFollowed FontSetThemeFontFollowed_;
  OHOS_GetFontCollectionGlobalInstance GetFontCollectionGlobalInstance_;
  OHOS_TypographyGetTextLines TypographyGetTextLines_;
  OHOS_DestroyTextLines DestroyTextLines_;
  OHOS_GetTextLineByIndex GetTextLineByIndex_;
  OHOS_TypographyGetCharacterRangeForGlyphRangeWithBuffer
      TypographyGetCharacterRangeForGlyphRangeWithBuffer_;
  OHOS_ReleaseRangeBuffer ReleaseRangeBuffer_;
};
static std::unique_ptr<OHOS_ShapingFuncPtr> ohos_shaping_funcs_ = nullptr;
class AGShapingResult : public PlatformShapingResultReader {
 public:
  AGShapingResult() = default;
  ~AGShapingResult() override = default;

 public:
  uint32_t GlyphCount() const override {
    return static_cast<uint32_t>(ct_glyphs_.size());
  }
  uint32_t TextCount() const override { return text_length_; }
  GlyphID ReadGlyphID(uint32_t idx) const override { return ct_glyphs_[idx]; }
  float ReadAdvanceX(uint32_t idx) const override {
    return ct_advances_[idx][0];
  }
  float ReadAdvanceY(uint32_t idx) const override {
    return ct_advances_[idx][1];
  }
  float ReadPositionX(uint32_t idx) const override {
    return ct_position_[idx][0];
  }
  float ReadPositionY(uint32_t idx) const override {
    return ct_position_[idx][1];
  }
  uint32_t ReadIndices(uint32_t idx) const override {
    return static_cast<uint32_t>(ct_indices_[idx]);
  }
  TypefaceRef ReadFontId(uint32_t idx) const override { return typeface_[idx]; }

 public:
  std::vector<GlyphID> ct_glyphs_;
  std::vector<std::array<float, 2>> ct_advances_;
  std::vector<uint32_t> ct_indices_;
  std::vector<std::array<float, 2>> ct_position_;
  std::vector<TypefaceRef> typeface_;
  uint32_t text_length_{0};
};
ShaperArkGraphics::ShaperArkGraphics(
    const FontmgrCollection& font_collections) noexcept
    : TTShaper(font_collections),
      shared_font_collection_(OH_Drawing_CreateSharedFontCollection()) {
  typography_style_ = OH_Drawing_CreateTypographyStyle();

  if (ohos_shaping_funcs_ == nullptr) {
    ohos_shaping_funcs_ = std::make_unique<OHOS_ShapingFuncPtr>();
    void* handle = dlopen("libnative_drawing.so", RTLD_LAZY);
    if (handle) {
      // clang-format off
      ohos_shaping_funcs_->CreateLineTypography_ = (OHOS_CreateLineTypography)dlsym(handle, "OH_Drawing_CreateLineTypography");
      ohos_shaping_funcs_->LineTypographyCreateLine_ = (OHOS_LineTypographyCreateLine)dlsym(handle, "OH_Drawing_LineTypographyCreateLine");
      ohos_shaping_funcs_->TextLineGetGlyphCount_ = (OHOS_TextLineGetGlyphCount)dlsym(handle, "OH_Drawing_TextLineGetGlyphCount");
      ohos_shaping_funcs_->TextLineGetGlyphRuns_ = (OHOS_TextLineGetGlyphRuns)dlsym(handle, "OH_Drawing_TextLineGetGlyphRuns");
      ohos_shaping_funcs_->GetDrawingArraySize_ = (OHOS_GetDrawingArraySize)dlsym(handle, "OH_Drawing_GetDrawingArraySize");
      ohos_shaping_funcs_->GetRunByIndex_ = (OHOS_GetRunByIndex)dlsym(handle, "OH_Drawing_GetRunByIndex");
      ohos_shaping_funcs_->GetRunGlyphCount_ = (OHOS_GetRunGlyphCount)dlsym(handle, "OH_Drawing_GetRunGlyphCount");
      ohos_shaping_funcs_->GetRunGlyphs_ = (OHOS_GetRunGlyphs)dlsym(handle, "OH_Drawing_GetRunGlyphs");
      ohos_shaping_funcs_->GetRunGlyphAdvances_ = (OHOS_GetRunGlyphAdvances)dlsym(handle, "OH_Drawing_GetRunGlyphAdvances");
      ohos_shaping_funcs_->GetRunPositions_ = (OHOS_GetRunPositions)dlsym(handle, "OH_Drawing_GetRunPositions");
      ohos_shaping_funcs_->GetRunStringIndices_ = (OHOS_GetRunStringIndices)dlsym(handle, "OH_Drawing_GetRunStringIndices");
      ohos_shaping_funcs_->GetRunFont_ = (OHOS_GetRunFont)dlsym(handle, "OH_Drawing_GetRunFont");
      ohos_shaping_funcs_->GetRunGlyphsByIndex_ = (OHOS_GetRunGlyphsByIndex)dlsym(handle, "OH_Drawing_GetRunGlyphsByIndex");
      ohos_shaping_funcs_->GetRunPositionsByIndex_ = (OHOS_GetRunPositionsByIndex)dlsym(handle, "OH_Drawing_GetRunPositionsByIndex");
      ohos_shaping_funcs_->GetRunStringIndicesByIndex_ = (OHOS_GetRunStringIndicesByIndex)dlsym(handle, "OH_Drawing_GetRunStringIndicesByIndex");
      ohos_shaping_funcs_->GetRunGlyphAdvanceByIndex_ = (OHOS_GetRunGlyphAdvanceByIndex)dlsym(handle, "OH_Drawing_GetRunGlyphAdvanceByIndex");
      ohos_shaping_funcs_->DestroyRunStringIndices_ = (OHOS_DestroyRunStringIndices)dlsym(handle, "OH_Drawing_DestroyRunStringIndices");
      ohos_shaping_funcs_->DestroyRunPositions_ = (OHOS_DestroyRunPositions)dlsym(handle, "OH_Drawing_DestroyRunPositions");
      ohos_shaping_funcs_->DestroyRunGlyphs_ = (OHOS_DestroyRunGlyphs)dlsym(handle, "OH_Drawing_DestroyRunGlyphs");
      ohos_shaping_funcs_->DestroyRuns_ = (OHOS_DestroyRuns)dlsym(handle, "OH_Drawing_DestroyRuns");
      ohos_shaping_funcs_->DestroyTextLine_ = (OHOS_DestroyTextLine)dlsym(handle, "OH_Drawing_DestroyTextLine");
      ohos_shaping_funcs_->DestroyLineTypography_ = (OHOS_DestroyLineTypography)dlsym(handle, "OH_Drawing_DestroyLineTypography");
      ohos_shaping_funcs_->TypographyHandlerAddEncodedText_ = (OHOS_TypographyHandlerAddEncodedText)dlsym(handle, "OH_Drawing_TypographyHandlerAddEncodedText");
      ohos_shaping_funcs_->FontSetThemeFontFollowed_ = (OHOS_FontSetThemeFontFollowed)dlsym(handle, "OH_Drawing_FontSetThemeFontFollowed");
      ohos_shaping_funcs_->GetFontCollectionGlobalInstance_ = (OHOS_GetFontCollectionGlobalInstance)dlsym(handle, "OH_Drawing_GetFontCollectionGlobalInstance");
      ohos_shaping_funcs_->TypographyGetTextLines_ = (OHOS_TypographyGetTextLines)dlsym(handle, "OH_Drawing_TypographyGetTextLines");
      ohos_shaping_funcs_->DestroyTextLines_ = (OHOS_DestroyTextLines)dlsym(handle, "OH_Drawing_DestroyTextLines");
      ohos_shaping_funcs_->GetTextLineByIndex_ = (OHOS_GetTextLineByIndex)dlsym(handle, "OH_Drawing_GetTextLineByIndex");
      ohos_shaping_funcs_->TypographyGetCharacterRangeForGlyphRangeWithBuffer_ = (OHOS_TypographyGetCharacterRangeForGlyphRangeWithBuffer)dlsym(handle, "OH_Drawing_TypographyGetCharacterRangeForGlyphRangeWithBuffer");
      ohos_shaping_funcs_->ReleaseRangeBuffer_ = (OHOS_ReleaseRangeBuffer)dlsym(handle, "OH_Drawing_ReleaseRangeBuffer");
      // clang-format on
    }
    if (ohos_shaping_funcs_->GetFontCollectionGlobalInstance_) {
      shared_font_collection_ =
          ohos_shaping_funcs_->GetFontCollectionGlobalInstance_();
    }
  }
}
ShaperArkGraphics::~ShaperArkGraphics() {
  OH_Drawing_DestroyTypographyStyle(typography_style_);
  OH_Drawing_DestroyFontCollection(shared_font_collection_);
}
void ShaperArkGraphics::OnShapeText(const ShapeKey& key,
                                    ShapeResult* result) const {
  bool is_only_space_char = true;
  for (const auto& ch : key.text_) {
    if (!base::IsSpaceChar(ch)) {
      is_only_space_char = false;
      break;
    }
  }
  if (!context_->IsHarmonyShaperForceLowAPI() &&
      ohos_shaping_funcs_
              ->TypographyGetCharacterRangeForGlyphRangeWithBuffer_ !=
          nullptr &&
      !is_only_space_char) {
    ShapingTextWithHighAPILevel(key, result);
  } else {
    ShapingTextWithLowAPILevel(key, result);
  }
}

void ShaperArkGraphics::ShapingTextWithHighAPILevel(const ShapeKey& key,
                                                    ShapeResult* result) const {
  auto typography_handler = OH_Drawing_CreateTypographyHandler(
      OH_Drawing_CreateTypographyStyle(), shared_font_collection_);
  auto text_style = OH_Drawing_CreateTextStyle();
  OH_Drawing_SetTextStyleFontSize(text_style, key.style_.GetFontSize());
  auto& fd = key.style_.GetFontDescriptor();
  const char** families = new const char*[fd.font_family_list_.size()];
  for (int32_t k = 0; k < static_cast<int32_t>(fd.font_family_list_.size());
       k++) {
    families[k] = fd.font_family_list_[k].c_str();
  }
  OH_Drawing_SetTextStyleFontFamilies(
      text_style, static_cast<int32_t>(fd.font_family_list_.size()), families);
  if (fd.font_style_.GetSlant() != FontStyle::kUpright_Slant)
    OH_Drawing_SetTextStyleFontStyle(text_style, FONT_STYLE_ITALIC);
  int weight = fd.font_style_.GetWeight() / 100 - 1;
  if (weight < 0) weight = 0;
  if (weight > 8) weight = 8;
  OH_Drawing_SetTextStyleFontWeight(text_style, weight);
  OH_Drawing_TypographyHandlerPushTextStyle(typography_handler, text_style);
  auto u32_text(key.text_);
  for (auto& ch : u32_text) {
    if (ch == '\n') {
      ch = '\r';
    }
  }

  ohos_shaping_funcs_->TypographyHandlerAddEncodedText_(
      typography_handler, u32_text.c_str(), u32_text.length() * 4,
      TEXT_ENCODING_UTF32);
  auto typo = OH_Drawing_CreateTypography(typography_handler);
  OH_Drawing_TypographyLayout(typo, 1e9);
  auto line_count = OH_Drawing_TypographyGetLineCount(typo);
  if (line_count == 0) {
    return;
  }
  auto lines = ohos_shaping_funcs_->TypographyGetTextLines_(typo);
  auto line = ohos_shaping_funcs_->GetTextLineByIndex_(lines, 0);

  uint32_t glyph_count =
      static_cast<uint32_t>(ohos_shaping_funcs_->TextLineGetGlyphCount_(line));
  AGShapingResult shaping_result;
  shaping_result.ct_advances_.resize(glyph_count);
  shaping_result.ct_glyphs_.resize(glyph_count);
  shaping_result.ct_indices_.resize(glyph_count);
  shaping_result.ct_position_.resize(glyph_count, {0, 0});
  shaping_result.typeface_.resize(glyph_count);

  auto glyph_runs = ohos_shaping_funcs_->TextLineGetGlyphRuns_(line);
  auto glyph_runs_count = ohos_shaping_funcs_->GetDrawingArraySize_(glyph_runs);
  auto glyph_idx = 0;
  for (auto k = 0; k < static_cast<int32_t>(glyph_runs_count); k++) {
    auto* run = ohos_shaping_funcs_->GetRunByIndex_(glyph_runs, k);
    auto glyph_cnt = ohos_shaping_funcs_->GetRunGlyphCount_(run);
    auto glyph_array = ohos_shaping_funcs_->GetRunGlyphs_(run, 0, glyph_cnt);
    auto advance_array =
        ohos_shaping_funcs_->GetRunGlyphAdvances_(run, 0, glyph_cnt);
    auto position_array =
        ohos_shaping_funcs_->GetRunPositions_(run, 0, glyph_cnt);
    auto indices_array =
        ohos_shaping_funcs_->GetRunStringIndices_(run, 0, glyph_cnt);
    auto font = ohos_shaping_funcs_->GetRunFont_(run);
    auto tf_helper = std::make_shared<AGTypefaceHelper>(font);

    for (auto j = 0; j < static_cast<int32_t>(glyph_cnt); j++) {
      shaping_result.ct_glyphs_[glyph_idx + j] =
          ohos_shaping_funcs_->GetRunGlyphsByIndex_(glyph_array, j);
      auto pos =
          ohos_shaping_funcs_->GetRunPositionsByIndex_(position_array, j);
      auto& ppos = shaping_result.ct_position_[glyph_idx + j];
      OH_Drawing_PointGetX(pos, &ppos[0]);
      OH_Drawing_PointGetY(pos, &ppos[1]);
      auto adv =
          ohos_shaping_funcs_->GetRunGlyphAdvanceByIndex_(advance_array, j);
      auto& aadv = shaping_result.ct_advances_[glyph_idx + j];
      OH_Drawing_PointGetX(adv, &aadv[0]);
      shaping_result.typeface_[glyph_idx + j] = tf_helper;
    }
    glyph_idx += glyph_cnt;
    ohos_shaping_funcs_->DestroyRunStringIndices_(indices_array);
    ohos_shaping_funcs_->DestroyRunPositions_(position_array);
    ohos_shaping_funcs_->DestroyRunGlyphs_(glyph_array);
  }
  delete[] families;
  const auto text_length = static_cast<uint32_t>(key.text_.length());
  TTString string(key.text_.c_str(), text_length);

  for (auto i = 0u; i < glyph_count; i++) {
    auto* char_range =
        ohos_shaping_funcs_
            ->TypographyGetCharacterRangeForGlyphRangeWithBuffer_(
                typo, i, i + 1, nullptr, TEXT_ENCODING_UTF8);
    if (char_range == nullptr) {
      continue;
    }
    auto u8_start = OH_Drawing_GetStartFromRange(char_range);
    auto char_start = string.Utf8PosToCharPos(static_cast<uint32_t>(u8_start));
    shaping_result.ct_indices_[i] = char_start;
    ohos_shaping_funcs_->ReleaseRangeBuffer_(char_range);
  }

  ohos_shaping_funcs_->DestroyRuns_(glyph_runs);
  ohos_shaping_funcs_->DestroyTextLines_(lines);
  OH_Drawing_DestroyTypography(typo);
  OH_Drawing_DestroyTypographyHandler(typography_handler);

  shaping_result.text_length_ = static_cast<uint32_t>(key.text_.length());
  result->AppendPlatformShapingResult(shaping_result);
}

void ShaperArkGraphics::ShapingTextWithLowAPILevel(const ShapeKey& key,
                                                   ShapeResult* result) const {
  auto font_mgr = font_collection_.GetDefaultFontManager();
  auto& font_desc = key.style_.GetFontDescriptor();
  auto font_family = font_desc.font_family_list_.empty()
                         ? ""
                         : font_desc.font_family_list_[0].c_str();
  auto tf_typeface =
      font_mgr->matchFamilyStyle(font_family, font_desc.font_style_);
  auto default_typeface = tf_typeface;
  auto text_len = static_cast<uint32_t>(key.text_.length());
  AGShapingResult shaping_result;
  shaping_result.ct_advances_.resize(text_len);
  shaping_result.ct_glyphs_.resize(text_len);
  shaping_result.ct_indices_.resize(text_len);
  shaping_result.ct_position_.resize(text_len, {0, 0});
  shaping_result.typeface_.resize(text_len);

  auto shaping_piece = [&](RunPiece& piece, OH_Drawing_Font* oh_font) {
    OH_Drawing_Font* drawing_font = oh_font;
    auto piece_length = piece.length_;
    if (drawing_font == nullptr) {
      tf_typeface = font_mgr->matchFamilyStyleCharacter(
          font_family, font_desc.font_style_, nullptr, 0,
          key.text_[piece.offset_]);
      auto ag_typeface_helper =
          std::static_pointer_cast<AGTypefaceHelper>(tf_typeface);
      drawing_font = ag_typeface_helper->GetTypeface();
      OH_Drawing_FontSetTextSize(drawing_font, key.style_.GetFontSize());
      if (ohos_shaping_funcs_ &&
          ohos_shaping_funcs_->FontSetThemeFontFollowed_) {
        ohos_shaping_funcs_->FontSetThemeFontFollowed_(drawing_font, true);
      }
    }
    auto* glyphs = shaping_result.ct_glyphs_.data() + piece.offset_;
    std::vector<float> glyph_advances(piece_length);
    auto glyph_count = OH_Drawing_FontTextToGlyphs(
        drawing_font, key.text_.data() + piece.offset_, piece_length * 4,
        TEXT_ENCODING_UTF32, glyphs, piece_length);
    TTASSERT(glyph_count == piece_length);
    OH_Drawing_FontGetWidths(drawing_font, glyphs, glyph_count,
                             glyph_advances.data());

    bool encounter_zero = false;
    uint32_t zero_start = 0;
    for (auto k = 0u; k < piece_length; k++) {
      if (glyphs[k] == 0) {
        if (!encounter_zero) {
          encounter_zero = true;
          zero_start = k;
        }
      } else {
        if (encounter_zero) {
          encounter_zero = false;
          RunPiece new_piece(piece.offset_ + zero_start, k - zero_start);
          run_pieces_.emplace_back(new_piece);
        }
        shaping_result.ct_advances_[piece.offset_ + k][0] = glyph_advances[k];
        shaping_result.ct_advances_[piece.offset_ + k][1] = 0;
        shaping_result.typeface_[piece.offset_ + k] = tf_typeface;
      }
    }
    if (encounter_zero) {
      if (zero_start != 0 || oh_font != nullptr) {
        run_pieces_.emplace_back(
            RunPiece(piece.offset_ + zero_start, piece_length - zero_start));
      }
    }
  };
  run_pieces_.emplace_back(
      RunPiece{0, static_cast<uint32_t>(key.text_.length())});
  do {
    auto run_piece = run_pieces_.front();
    run_pieces_.pop_front();
    OH_Drawing_Font* drawing_font = nullptr;
    if (tf_typeface) {
      auto ag_typeface_helper =
          std::static_pointer_cast<AGTypefaceHelper>(tf_typeface);
      drawing_font = ag_typeface_helper->GetTypeface();
      OH_Drawing_FontSetTextSize(drawing_font, key.style_.GetFontSize());
      if (ohos_shaping_funcs_ &&
          ohos_shaping_funcs_->FontSetThemeFontFollowed_) {
        ohos_shaping_funcs_->FontSetThemeFontFollowed_(drawing_font, true);
      }
    }
    shaping_piece(run_piece, drawing_font);
    tf_typeface = nullptr;
  } while (!run_pieces_.empty());

  auto advance_x = 0.f;
  for (auto k = 0u; k < text_len; k++) {
    shaping_result.ct_position_[k][0] = advance_x;
    shaping_result.ct_position_[k][1] = 0;
    advance_x += shaping_result.ct_advances_[k][0];
    shaping_result.ct_indices_[k] = k;
    if (shaping_result.typeface_[k] == nullptr) {
      shaping_result.typeface_[k] = default_typeface;
    }
  }

  shaping_result.text_length_ = text_len;
  result->AppendPlatformShapingResult(shaping_result);
}
}  // namespace tttext
}  // namespace ttoffice
