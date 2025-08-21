// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/java/java_shaper.h"

#include <textra/painter.h>
#include <textra/platform/java/java_font_manager.h>
#include <textra/platform/java/tttext_jni_proxy.h>

#include <memory>

#include "src/ports/shaper/java/java_shaper_result.h"
#include "src/ports/shaper/java/java_utils.h"
#include "src/textlayout/run/base_run.h"
#include "src/textlayout/style/style_manager.h"

namespace ttoffice {
namespace tttext {
std::map<FontKey, std::shared_ptr<JavaTypeface>> JavaShaper::font_lst_;
std::mutex JavaShaper::font_lst_lock_;
JavaShaper::JavaShaper(const FontmgrCollection& font_collection)
    : TTShaper(font_collection) {
  auto font_mgr = std::static_pointer_cast<JavaFontManager>(
      font_collection.GetDefaultFontManager());
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto* env = proxy.GetCurrentJNIEnv();
  auto inst =
      env->NewObject(reinterpret_cast<jclass>(proxy.JavaShaper_class_->get()),
                     proxy.JavaShaper_method_init_, font_mgr->GetInstance());
  if (ClearException(env)) {
    return;
  }
  java_instance_ = std::make_unique<ScopedGlobalRef>(env, inst);
}
void JavaShaper::OnShapeText(const ShapeKey& key, ShapeResult* result) const {
  auto& fd = key.style_.GetFontDescriptor();
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto* env = proxy.GetCurrentJNIEnv();
  JavaShapingResult reader;
  auto char_count = key.text_.length();
  reader.glyphs_.reserve(char_count * 2);
  reader.advances_.reserve(char_count * 2);
  auto& u16str = reader.glyphs_;
  auto& indexes = reader.indexes_;
  uint32_t char32_index = 0;
  for (auto utf32 : key.text_) {
    if (utf32 <= 0xD7FF || (utf32 >= 0xE000 && utf32 <= 0xFFFF)) {
      u16str.push_back(static_cast<GlyphID>(utf32));
      indexes.push_back(char32_index);
    } else if (utf32 >= 0x10000 && utf32 <= 0x10FFFF) {
      utf32 -= 0x10000;
      u16str.push_back(static_cast<char16_t>(0xD800 | ((utf32 >> 10) & 0x3FF)));
      u16str.push_back(static_cast<char16_t>(0xDC00 | (utf32 & 0x3FF)));
      indexes.push_back(char32_index);
      indexes.push_back(char32_index);
    } else {
      LogUtil::E("U32StringToU16 Invalid UTF-32 encoding");
    }
    char32_index++;
  }
  auto* j_content = TTTextJNIProxy::CreateJavaStringFromU16(
      env, reinterpret_cast<char16_t*>(u16str.data()), u16str.size());
  auto java_fontmanager = std ::static_pointer_cast<JavaFontManager>(
      font_collection_.GetDefaultFontManager());
  auto java_typeface = java_fontmanager->matchTypeface(fd);
  auto* info_list = (jfloatArray)env->CallObjectMethod(
      java_instance_->get(), proxy.JavaShaper_method_OnShapeText_, j_content,
      java_typeface->GetInstance(), key.style_.GetFontSize(), false,
      fd.font_style_.GetSlant() == FontStyle::kItalic_Slant);
  auto glyph_count = u16str.size();
  reader.text_count_ = char_count;
  if (info_list == nullptr) {
    // maybe encounter java exception
    ClearException(env);
    std::vector<float> advances(glyph_count, 0);
    reader.advances_.insert(reader.advances_.begin(), advances.begin(),
                            advances.end());
    reader.font_ = java_typeface;
    result->AppendPlatformShapingResult(reader);
    return;
  }
  auto* ret_advances = env->GetFloatArrayElements(info_list, nullptr);
  size_t ret_length = env->GetArrayLength(info_list);
  TTASSERT(ret_length == glyph_count);
  reader.advances_.insert(reader.advances_.begin(), ret_advances,
                          ret_advances + glyph_count);
  reader.font_ = java_typeface;
  result->AppendPlatformShapingResult(reader);
  env->ReleaseFloatArrayElements(info_list, ret_advances, 0);
  env->DeleteLocalRef(info_list);
  env->DeleteLocalRef(j_content);
}
}  // namespace tttext
}  // namespace ttoffice
