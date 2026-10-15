// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/java/java_shaper.h"

#include <textra/painter.h>
#include <textra/platform/java/java_font_manager.h>
#include <textra/platform/java/tttext_jni_proxy.h>
#ifdef USE_ICU
#include <textra/icu_wrapper.h>
#endif

#include <algorithm>
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
  auto font_mgr = std::static_pointer_cast<JavaFontManager>(
      font_collection.GetDefaultFontManager());
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto* env = proxy.GetCurrentJNIEnv();
  if (env == nullptr) {
    return;
  }
  auto inst =
      env->NewObject(reinterpret_cast<jclass>(proxy.JavaShaper_class_->get()),
                     proxy.JavaShaper_method_init_, font_mgr->GetInstance());
  if (ClearException(env)) {
    if (inst != nullptr) {
      env->DeleteLocalRef(inst);
    }
    return;
  }
  java_instance_ = std::make_unique<ScopedGlobalRef>(env, inst);
  env->DeleteLocalRef(inst);
}

void JavaShaper::OnShapeText(const ShapeKey& key, ShapeResult* result) const {
  auto& fd = key.style_.GetFontDescriptor();
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto* env = proxy.GetCurrentJNIEnv();
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
  auto* j_content = TTTextJNIProxy::CreateJavaStringFromU16(
      env, reinterpret_cast<char16_t*>(u16str.data()), u16str.size());
  auto java_fontmanager = std ::static_pointer_cast<JavaFontManager>(
      font_collection_.GetDefaultFontManager());
  auto java_typeface = java_fontmanager->matchTypeface(fd);
  java_typeface->SetStaticLayoutMetricsEnabled(android_metrics_round);
  reader.font_ = java_typeface;
  const float requested_font_size = key.style_.GetFontSize();
  const float font_size = android_metrics_round
                              ? std::floor(requested_font_size)
                              : requested_font_size;
  auto* info_list = (jfloatArray)env->CallObjectMethod(
      java_instance_->get(), proxy.JavaShaper_method_OnShapeText_, j_content,
      java_typeface->GetInstance(), font_size, false,
      fd.font_style_.GetSlant() == FontStyle::kItalic_Slant);
  if (chinese_fonts != 0) {
    auto default_manager = proxy.GetDefaultFontManager();
    const auto& chinese_font =
        default_manager->GetDefaultChineseTypeface(false);
    if (chinese_font) {
      float ascent_ratio = java_typeface->GetAscentRatio();
      float descent_ratio = java_typeface->GetDescentRatio();
      const auto merge_ratios = [&](const std::shared_ptr<JavaTypeface>& font) {
        ascent_ratio = std::min(ascent_ratio, font->GetAscentRatio());
        descent_ratio = std::max(descent_ratio, font->GetDescentRatio());
      };
      if (chinese_fonts & kChineseFont) {
        merge_ratios(chinese_font);
      }
      if (chinese_fonts & kSupplementaryChineseFont) {
        merge_ratios(default_manager->GetDefaultChineseTypeface(true));
      }
      if (ascent_ratio != java_typeface->GetAscentRatio() ||
          descent_ratio != java_typeface->GetDescentRatio()) {
        reader.font_ =
            java_typeface->CopyWithFallbackMetrics(ascent_ratio, descent_ratio);
      }
    }
  }
  auto glyph_count = u16str.size();
  reader.text_count_ = char_count;
  if (info_list == nullptr) {
    // maybe encounter java exception
    ClearException(env);
    reader.advances_.resize(glyph_count, 0);
    result->AppendPlatformShapingResult(reader);
    env->DeleteLocalRef(j_content);
    return;
  }
  auto* ret_advances = env->GetFloatArrayElements(info_list, nullptr);
  auto ret_length = env->GetArrayLength(info_list);
  TTASSERT(ret_length >= static_cast<jsize>(glyph_count));
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
