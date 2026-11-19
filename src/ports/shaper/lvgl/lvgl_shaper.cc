// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/lvgl/lvgl_shaper.h"

#include <utility>
#include <vector>

namespace ttoffice {
namespace tttext {
class LVGLShapingResult final : public PlatformShapingResultReader {
 public:
  ~LVGLShapingResult() override = default;
  uint32_t GlyphCount() const override {
    return static_cast<uint32_t>(glyphs_.size());
  }
  uint32_t TextCount() const override { return text_count_; }
  GlyphID ReadGlyphID(uint32_t idx) const override { return glyphs_[idx]; }
  float ReadAdvanceX(uint32_t idx) const override { return advances_[idx]; }
  uint32_t ReadIndices(uint32_t idx) const override { return indexes_[idx]; }
  TypefaceRef ReadFontId(uint32_t idx) const override { return font_lst_[idx]; }
  float ReadPositionX(uint32_t idx) const override {
    return advances_[idx] / 2;
  }
  std::vector<float> advances_;
  std::vector<GlyphID> glyphs_;
  std::vector<uint32_t> indexes_;
  uint32_t text_count_;
  std::vector<TypefaceRef> font_lst_;
};

LVGLShaper::LVGLShaper(FontmgrCollection font_collection) noexcept
    : TTShaper(font_collection) {}

void LVGLShaper::OnShapeText(const ShapeKey& key, ShapeResult* result) const {
  LVGLShapingResult reader;
  auto manager = font_collection_.GetDefaultFontManager();
  if (!manager) {
    return;
  }
  const auto& descriptor = key.style_.GetFontDescriptor();
  std::vector<TypefaceRef> faces;
  if (descriptor.platform_font_) {
    auto face = manager->createTypefaceFromPlatformFont(
        reinterpret_cast<const void*>(descriptor.platform_font_));
    if (!face) {
      return;
    }
    faces.push_back(std::move(face));
  } else {
    faces = font_collection_.findTypefaces(descriptor);
  }
  uint32_t index = 0;
  for (auto ch : key.text_) {
    if (ch > 0xffff) {
      return;
    }
    TypefaceRef face;
    GlyphID glyph = 0;
    if (ch < 32) {
      face = faces.empty()
                 ? manager->matchFamilyStyle(nullptr, descriptor.font_style_)
                 : faces.front();
      if (!face) {
        return;
      }
      reader.glyphs_.push_back(0);
      reader.indexes_.push_back(index++);
      reader.advances_.push_back(0);
      reader.font_lst_.push_back(std::move(face));
      continue;
    }
    for (auto& candidate : faces) {
      if (candidate && (glyph = candidate->UnicharToGlyph(ch)) != 0) {
        face = candidate;
        break;
      }
    }
    if (!face && font_collection_.fontFallbackEnabled()) {
      face = manager->matchFamilyStyleCharacter(
          descriptor.font_family_list_.empty()
              ? nullptr
              : descriptor.font_family_list_.front().c_str(),
          descriptor.font_style_, nullptr, 0, ch);
      if (face) {
        glyph = face->UnicharToGlyph(ch);
        faces.push_back(face);
      }
    }
    if (!face || !glyph) {
      return;
    }
    reader.glyphs_.push_back(glyph);
    reader.indexes_.push_back(index++);
    GlyphID pair[2]{glyph, 0};
    auto next = index < key.text_.length() ? key.text_[index] : 0;
    if (next <= 0xffff) pair[1] = static_cast<GlyphID>(next);
    float widths[2]{};
    face->GetHorizontalAdvances(pair, next ? 2 : 1, widths,
                                key.style_.GetFontSize());
    reader.advances_.push_back(widths[0]);
    reader.font_lst_.push_back(std::move(face));
  }
  reader.text_count_ = index;
  result->AppendPlatformShapingResult(reader);
}
}  // namespace tttext
}  // namespace ttoffice
