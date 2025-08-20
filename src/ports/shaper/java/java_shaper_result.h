// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_JAVA_JAVA_SHAPER_RESULT_H_
#define SRC_PORTS_SHAPER_JAVA_JAVA_SHAPER_RESULT_H_

#include <textra/platform/java/tttext_jni_proxy.h>

#include <memory>
#include <vector>

#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
class JavaShapingResult final : public PlatformShapingResultReader {
 public:
  ~JavaShapingResult() override = default;
  uint32_t GlyphCount() const override { return glyphs_.size(); }
  uint32_t TextCount() const override { return text_count_; }
  GlyphID ReadGlyphID(uint32_t idx) const override { return glyphs_[idx]; }
  float ReadAdvanceX(uint32_t idx) const override { return advances_[idx]; }
  uint32_t ReadIndices(uint32_t idx) const override { return indexes_[idx]; }
  TypefaceRef ReadFontId(uint32_t idx) const override { return font_; }
  std::vector<float> advances_;
  std::vector<GlyphID> glyphs_;
  std::vector<uint32_t> indexes_;
  uint32_t text_count_;
  TypefaceRef font_;
};

class JavaShapingResultNew final : public PlatformShapingResultReader {
 public:
  explicit JavaShapingResultNew(jobject java_instance) {
    auto* env = TTTextJNIProxy::GetInstance().GetCurrentJNIEnv();
    auto& proxy = TTTextJNIProxy::GetInstance();

    jint glyph_count = env->GetIntField(
        java_instance, proxy.JavaShapeResult_field_glyph_count_);
    glyphs_.resize(glyph_count);
    auto glyphs_array = (jshortArray)env->GetObjectField(
        java_instance, proxy.JavaShapeResult_field_glyphs_);
    env->GetShortArrayRegion(glyphs_array, 0, glyph_count,
                             reinterpret_cast<jshort*>(glyphs_.data()));
    advances_.resize(glyph_count);
    auto advances_array = (jfloatArray)env->GetObjectField(
        java_instance, proxy.JavaShapeResult_field_advances_);
    env->GetFloatArrayRegion(advances_array, 0, glyph_count, advances_.data());
    position_y_.resize(glyph_count);
    auto position_y_array = (jfloatArray)env->GetObjectField(
        java_instance, proxy.JavaShapeResult_field_position_y_);
    env->GetFloatArrayRegion(position_y_array, 0, glyph_count,
                             position_y_.data());
    auto typeface_instance_array = (jlongArray)env->GetObjectField(
        java_instance, proxy.JavaShapeResult_field_typeface_instance_);
    auto long_array = std::make_unique<jlong[]>(glyph_count);
    env->GetLongArrayRegion(typeface_instance_array, 0, glyph_count,
                            long_array.get());
    for (auto k = 0; k < glyph_count; ++k) {
      typeface_instance_.emplace_back(
          reinterpret_cast<JavaTypeface*>(long_array[k])->shared_from_this());
    }
  }
  ~JavaShapingResultNew() override = default;
  uint32_t GlyphCount() const override { return glyphs_.size(); }
  uint32_t TextCount() const override { return glyphs_.size(); }
  GlyphID ReadGlyphID(uint32_t idx) const override { return glyphs_[idx]; }
  float ReadAdvanceX(uint32_t idx) const override { return advances_[idx]; }
  uint32_t ReadIndices(uint32_t idx) const override { return idx; }
  TypefaceRef ReadFontId(uint32_t idx) const override {
    return typeface_instance_[idx];
  }
  std::vector<GlyphID> glyphs_{};
  std::vector<float> advances_;
  std::vector<float> position_y_;
  std::vector<TypefaceRef> typeface_instance_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_JAVA_JAVA_SHAPER_RESULT_H_
