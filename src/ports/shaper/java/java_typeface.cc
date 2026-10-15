// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/platform/java/java_typeface.h>
#include <textra/platform/java/tttext_jni_proxy.h>

#include <algorithm>
#include <cmath>

#include "src/ports/shaper/java/java_utils.h"
namespace ttoffice {
namespace tttext {
JavaTypeface::JavaTypeface(uint32_t unique_id) : ITypefaceHelper(unique_id) {}
JavaTypeface::~JavaTypeface() = default;
jobject JavaTypeface::GetInstance() const { return java_instance_->get(); }
std::shared_ptr<JavaTypeface> JavaTypeface::CopyWithFallbackMetrics(
    float ascent_ratio, float descent_ratio) const {
  auto typeface = std::make_shared<JavaTypeface>(unique_id_);
  typeface->java_instance_ = java_instance_;
  typeface->font_style_ = font_style_;
  typeface->font_ascent_ratio_ = std::min(font_ascent_ratio_, ascent_ratio);
  typeface->font_descent_ratio_ = std::max(font_descent_ratio_, descent_ratio);
  typeface->round_font_metrics_ = round_font_metrics_;
  typeface->floor_font_size_ = floor_font_size_;
  return typeface;
}
void JavaTypeface::BindJavaHandler(JNIEnv* env, jobject handler,
                                   uint32_t unique_id, float font_ascent_ratio,
                                   float font_descent_ratio) {
  java_instance_ = std::make_shared<ScopedGlobalRef>(env, handler);
  unique_id_ = unique_id;
  font_ascent_ratio_ = font_ascent_ratio;
  font_descent_ratio_ = font_descent_ratio;
}
float JavaTypeface::GetHorizontalAdvance(GlyphID glyph_id,
                                         float font_size) const {
  return 0;
}
void JavaTypeface::GetHorizontalAdvances(GlyphID* glyph_ids, uint32_t count,
                                         float* widths, float font_size) const {
}
void JavaTypeface::GetWidthBound(float* rect_f, GlyphID glyph_id,
                                 float font_size) const {}
void JavaTypeface::GetWidthBounds(float* rect_ltrb, GlyphID glyphs[],
                                  uint32_t glyph_count, float font_size) {
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto env = proxy.GetCurrentJNIEnv();
  auto jchar_array = env->NewCharArray(glyph_count);
  env->SetCharArrayRegion(jchar_array, 0, glyph_count,
                          reinterpret_cast<const jchar*>(glyphs));
  jobject jrect_ltrb = env->CallObjectMethod(
      java_instance_->get(), proxy.JavaTypeface_method_GetTextBounds,
      jchar_array, font_size);
  if (jrect_ltrb == nullptr) {
    ClearException(env);
    std::memset(rect_ltrb, 0, 4 * sizeof(float));
    return;
  }
  auto* jarray = env->GetFloatArrayElements((jfloatArray)jrect_ltrb, nullptr);
  std::memcpy(rect_ltrb, jarray, 4 * sizeof(float));
  env->ReleaseFloatArrayElements((jfloatArray)jrect_ltrb, jarray, 0);
  env->DeleteLocalRef(jchar_array);
  env->DeleteLocalRef(jrect_ltrb);
}
const void* JavaTypeface::GetFontData() const { return nullptr; }
size_t JavaTypeface::GetFontDataSize() const { return 0; }
int JavaTypeface::GetFontIndex() const { return 0; }
uint32_t JavaTypeface::GetUnitsPerEm() const { return 0; }
uint16_t JavaTypeface::UnicharToGlyph(Unichar codepoint,
                                      uint32_t variationSelector) const {
  return 0;
}
void JavaTypeface::UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                                    GlyphID* glyphs) const {}
void JavaTypeface::OnCreateFontInfo(FontInfo* info, float font_size) const {
  const float metrics_size =
      floor_font_size_ ? std::floor(font_size) : font_size;
  const float ascent = font_ascent_ratio_ * metrics_size;
  const float descent = font_descent_ratio_ * metrics_size;
  const bool round_metrics = round_font_metrics_;
  info->SetAscent(round_metrics ? std::floor(ascent + 0.5f) : ascent);
  info->SetDescent(round_metrics ? std::floor(descent + 0.5f) : descent);
  info->SetFontSize(font_size);
}
}  // namespace tttext
}  // namespace ttoffice
