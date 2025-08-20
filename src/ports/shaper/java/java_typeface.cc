// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/platform/java/java_typeface.h>
#include <textra/platform/java/tttext_jni_proxy.h>
namespace ttoffice {
namespace tttext {
JavaTypeface::JavaTypeface(uint32_t unique_id) : ITypefaceHelper(unique_id) {}
JavaTypeface::~JavaTypeface() = default;
jobject JavaTypeface::GetInstance() const { return java_instance_->get(); }
void JavaTypeface::BindJavaHandler(jobject handler, uint32_t unique_id) {
  auto env = TTTextJNIProxy::GetInstance().GetCurrentJNIEnv();
  java_instance_ = std::make_unique<ScopedGlobalRef>(env, handler);
  unique_id_ = unique_id;
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
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto env = proxy.GetCurrentJNIEnv();
  auto* metrics = (jfloatArray)env->CallObjectMethod(
      java_instance_->get(), proxy.JavaTypeface_method_GetFontMetrics,
      font_size);
  if (metrics == nullptr) {
    info->SetAscent(0);
    info->SetDescent(0);
    info->SetFontSize(font_size);
    return;
  }
  auto* jarray = env->GetFloatArrayElements(metrics, nullptr);
  TTASSERT(env->GetArrayLength(metrics) == 2);
  info->SetAscent(jarray[0]);
  info->SetDescent(jarray[1]);
  info->SetFontSize(font_size);
}
}  // namespace tttext
}  // namespace ttoffice
