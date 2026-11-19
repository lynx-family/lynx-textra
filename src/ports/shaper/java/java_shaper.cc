// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/java/java_shaper.h"

#include <textra/painter.h>

#include "src/ports/native/java/tttext_jni_proxy.h"
#include "src/ports/shaper/java/java_shaping_font_manager.h"
#ifdef USE_ICU
#include <textra/icu_wrapper.h>
#endif

#include <algorithm>
#include <climits>
#include <cmath>
#include <memory>

#include "src/ports/shaper/java/java_shaper_result.h"
#include "src/ports/shaper/java/java_utils.h"
#include "src/textlayout/run/base_run.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
namespace {
constexpr uint32_t kChineseFont = 1;
constexpr uint32_t kSupplementaryChineseFont = 2;

uint32_t GetChineseFontMask(char32_t ch) {
  // Han, CJK punctuation and fullwidth forms. This is a range-based estimate
  // for the system default fallback, not a lookup of the actual glyph font.
  if (base::IsCJKUnifiedIdeographBMP(ch) ||
      base::IsCJKCompatibilityIdeographBMP(ch) ||
      base::IsCJKSymbolsAndFullwidthForms(ch)) {
    return kChineseFont;
  }
  if (base::IsCJKSupplementaryIdeograph(ch)) {
    return kSupplementaryChineseFont;
  }
  return 0;
}
}  // namespace

std::map<FontKey, std::shared_ptr<JavaTypeface>> JavaShaper::font_lst_;
std::mutex JavaShaper::font_lst_lock_;
JavaShaper::JavaShaper(const FontmgrCollection& font_collection)
    : TTShaper(font_collection) {
  font_manager_ = JavaShapingFontManager::Create(font_collection);
  if (!font_manager_) return;
  auto native_manager = font_manager_->GetInstance();
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto* env = proxy.GetCurrentJNIEnv();
  if (env == nullptr) {
    return;
  }
  auto inst = env->NewObject(
      reinterpret_cast<jclass>(proxy.JavaShaper_class_->get()),
      proxy.JavaShaper_method_init_, static_cast<jobject>(native_manager));
  if (ClearException(env)) {
    if (inst != nullptr) {
      env->DeleteLocalRef(inst);
    }
    return;
  }
  java_instance_ = std::make_unique<ScopedGlobalRef>(env, inst);
  env->DeleteLocalRef(inst);
}

JavaShaper::~JavaShaper() = default;

void JavaShaper::OnShapeText(const ShapeKey& key, ShapeResult* result) const {
  auto& fd = key.style_.GetFontDescriptor();
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto* env = proxy.GetCurrentJNIEnv();
  if (!env || !java_instance_) {
    return;
  }
  JavaShapingResult reader;
  const bool android_metrics_round =
      context_ != nullptr && context_->IsAndroidMetricsRound();
  const bool use_fallback_metrics = android_metrics_round &&
                                    fd.platform_font_ == 0 &&
                                    fd.font_family_list_.empty();
  uint32_t chinese_fonts = 0;
  auto char_count = key.text_.length();
  reader.glyphs_.reserve(char_count * 2);
  reader.advances_.reserve(char_count * 2);
  auto& u16str = reader.glyphs_;
  auto& indexes = reader.indexes_;
  uint32_t char32_index = 0;
  for (auto utf32 : key.text_) {
    if (use_fallback_metrics) {
      chinese_fonts |= GetChineseFontMask(utf32);
    }
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
  if (u16str.size() > INT_MAX) {
    return;
  }
  auto* j_content = TTTextJNIProxy::CreateJavaStringFromU16(
      env, reinterpret_cast<char16_t*>(u16str.data()),
      static_cast<uint32_t>(u16str.size()));
  if (!j_content) {
    ClearException(env);

    return;
  }
  auto font =
      font_manager_->matchTypeface(fd, android_metrics_round, chinese_fonts);
  if (!font.typeface || !font.instance) {
    env->DeleteLocalRef(j_content);
    return;
  }
  reader.font_ = std::move(font.typeface);
  const float requested_font_size = key.style_.GetFontSize();
  const float font_size = android_metrics_round
                              ? std::floor(requested_font_size)
                              : requested_font_size;
  auto* info_list = (jfloatArray)env->CallObjectMethod(
      java_instance_->get(), proxy.JavaShaper_method_OnShapeText_, j_content,
      font.instance, font_size, false,
      fd.font_style_.GetSlant() == FontStyle::kItalic_Slant);
  auto glyph_count = u16str.size();
  reader.text_count_ = static_cast<uint32_t>(char_count);
  if (info_list == nullptr) {
    // maybe encounter java exception
    ClearException(env);

    env->DeleteLocalRef(j_content);
    return;
  }
  auto ret_length = env->GetArrayLength(info_list);
  if (static_cast<size_t>(ret_length) < glyph_count) {
    env->DeleteLocalRef(info_list);
    env->DeleteLocalRef(j_content);
    return;
  }
  auto* ret_advances = env->GetFloatArrayElements(info_list, nullptr);
  if (!ret_advances) {
    ClearException(env);

    env->DeleteLocalRef(info_list);
    env->DeleteLocalRef(j_content);
    return;
  }
  reader.advances_.insert(reader.advances_.begin(), ret_advances,
                          ret_advances + glyph_count);
  result->AppendPlatformShapingResult(reader);
  env->ReleaseFloatArrayElements(info_list, ret_advances, JNI_ABORT);
  env->DeleteLocalRef(info_list);
  env->DeleteLocalRef(j_content);
}

void JavaShaper::ProcessBidirection(
    const char32_t* text, uint32_t length,
    ttoffice::tttext::WriteDirection write_direction, uint32_t* visual_map,
    uint32_t* logical_map, uint8_t* dir_vec) {
#ifdef USE_ICU
  ICUWrapper::BidiInit(text, length, write_direction, dir_vec, visual_map,
                       logical_map);
#else
  TTShaper::ProcessBidirection(text, length, write_direction, visual_map,
                               logical_map, dir_vec);
#endif
}

}  // namespace tttext
}  // namespace ttoffice
