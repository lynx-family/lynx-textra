// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/platform/java/java_font_manager.h>
#include <textra/platform/java/java_typeface.h>
#include <textra/platform/java/tttext_jni_proxy.h>

#include <memory>

#include "src/ports/shaper/java/java_utils.h"

namespace ttoffice {
namespace tttext {
JavaFontManager::JavaFontManager() {
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto* env = proxy.GetCurrentJNIEnv();
  auto inst = env->NewObject(
      static_cast<jclass>(proxy.JavaFontManager_class_->get()),
      proxy.JavaFontManager_method_init_, reinterpret_cast<jlong>(this));
  if (ClearException(env)) {
    return;
  }
  java_instance_ = std::make_unique<ScopedGlobalRef>(env, inst);
  BindJavaHandler();
  env->DeleteLocalRef(inst);
}
JavaFontManager::~JavaFontManager() = default;
jobject JavaFontManager::GetInstance() const { return java_instance_->get(); }
void JavaFontManager::BindJavaHandler() {
  auto ret = matchFamilyStyle(nullptr, FontStyle::Normal());
  TTASSERT(ret != nullptr);
  ret = matchFamilyStyle(nullptr, FontStyle::Bold());
  TTASSERT(ret != nullptr);
  ret = matchFamilyStyle(nullptr, FontStyle::Italic());
  TTASSERT(ret != nullptr);
  ret = matchFamilyStyle(nullptr, FontStyle::BoldItalic());
  TTASSERT(ret != nullptr);
}
void JavaFontManager::BindDefaultFallbackMetrics(
    float ascent_ratio, float descent_ratio, float supplementary_ascent_ratio,
    float supplementary_descent_ratio) {
  auto font = std::static_pointer_cast<JavaTypeface>(
      matchFamilyStyle(nullptr, FontStyle::Normal()));
  auto* env = TTTextJNIProxy::GetCurrentJNIEnv();
  default_chinese_typeface_ =
      std::make_shared<JavaTypeface>(font->GetUniqueId());
  default_chinese_typeface_->BindJavaHandler(env, font->GetInstance(),
                                             font->GetUniqueId(), ascent_ratio,
                                             descent_ratio);
  default_chinese_typeface_->SetStaticLayoutMetricsEnabled(true, true);
  default_supplementary_chinese_typeface_ =
      std::make_shared<JavaTypeface>(font->GetUniqueId());
  default_supplementary_chinese_typeface_->BindJavaHandler(
      env, font->GetInstance(), font->GetUniqueId(), supplementary_ascent_ratio,
      supplementary_descent_ratio);
  default_supplementary_chinese_typeface_->SetStaticLayoutMetricsEnabled(true,
                                                                         true);
}
int JavaFontManager::countFamilies() const { return 0; }
TypefaceRef JavaFontManager::matchFamilyStyle(const char familyName[],
                                              const FontStyle& font_style) {
  FontDescriptor fd;
  if (familyName != nullptr) {
    fd.font_family_list_.emplace_back(familyName);
  }
  fd.font_style_ = font_style;
  fd.platform_font_ = 0;
  return matchTypeface(fd);
}
TypefaceRef JavaFontManager::matchFamilyStyleCharacter(const char familyName[],
                                                       const FontStyle&,
                                                       const char* bcp47[],
                                                       int bcp47Count,
                                                       uint32_t character) {
  TTASSERT(false);
  return nullptr;
}
TypefaceRef JavaFontManager::makeFromFile(const char path[], int ttcIndex) {
  TTASSERT(false);
  return nullptr;
}
TypefaceRef JavaFontManager::legacyMakeTypeface(const char familyName[],
                                                FontStyle style) const {
  TTASSERT(false);
  return nullptr;
}
void* JavaFontManager::getPlatformFontFromTypeface(TypefaceRef typeface) {
  return reinterpret_cast<void*>(typeface->GetFontIndex());
}

std::shared_ptr<JavaTypeface> JavaFontManager::CreateNativeTypeface() {
  auto typeface = std::make_shared<JavaTypeface>(0);
  std::lock_guard<std::mutex> lock(typeface_list_mutex_);
  typeface_list_.push_back(typeface);
  return typeface;
}

std::shared_ptr<JavaTypeface> JavaFontManager::matchTypeface(
    const FontDescriptor& fd) {
  {
    std::lock_guard<std::mutex> lock(typeface_map_mutex_);
    auto iter = typeface_map_.find(fd);
    if (iter != typeface_map_.end()) {
      return iter->second;
    }
  }
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto env = proxy.GetCurrentJNIEnv();
  jstring j_settings = nullptr;
  if (!fd.font_feature_settings_.empty()) {
    std::string settings;
    for (const auto& [tag, value] : fd.font_feature_settings_) {
      if (!settings.empty()) {
        settings += ",";
      }
      settings += "'";
      settings.append(tag, 4);
      settings += "' " + std::to_string(value);
    }
    j_settings = TTTextJNIProxy::CreateJavaStringFromU8(env, settings.data(),
                                                        settings.size());
  }
  std::shared_ptr<JavaTypeface> obj = nullptr;
  jlong raw_pointer = 0;
  if (fd.platform_font_ != 0) {
    raw_pointer = env->CallLongMethod(
        java_instance_->get(), proxy.JavaFontManager_method_onMatchTypeface_,
        nullptr, static_cast<jint>(0), static_cast<jboolean>(false),
        static_cast<jlong>(fd.platform_font_), j_settings,
        static_cast<jlong>(0));
  } else {
    jstring j_fontfamily = nullptr;
    if (!fd.font_family_list_.empty()) {
      const char* familyName = fd.font_family_list_[0].c_str();
      if (strlen(familyName) > 0) {
        j_fontfamily = TTTextJNIProxy::CreateJavaStringFromU8(
            env, familyName, strlen(familyName));
      }
    }
    obj = std::make_shared<JavaTypeface>(0);
    raw_pointer = env->CallLongMethod(
        java_instance_->get(), proxy.JavaFontManager_method_onMatchTypeface_,
        j_fontfamily, static_cast<jint>(fd.font_style_.GetWeight()),
        static_cast<jboolean>(fd.font_style_.GetSlant() !=
                              FontStyle::kUpright_Slant),
        static_cast<jlong>(0), j_settings, reinterpret_cast<jlong>(obj.get()));
    if (j_fontfamily != nullptr) {
      env->DeleteLocalRef(j_fontfamily);
    }
  }
  if (j_settings != nullptr) {
    env->DeleteLocalRef(j_settings);
  }
  if (ClearException(env) || raw_pointer == 0) {
    return nullptr;
  }
  if (raw_pointer != reinterpret_cast<jlong>(obj.get())) {
    obj = std::static_pointer_cast<JavaTypeface>(
        reinterpret_cast<JavaTypeface*>(raw_pointer)->shared_from_this());
  }
  {
    std::lock_guard<std::mutex> lock(typeface_map_mutex_);
    auto [iter, inserted] = typeface_map_.emplace(fd, obj);
    if (!inserted) {
      return iter->second;
    }
  }
  return obj;
}
}  // namespace tttext
}  // namespace ttoffice
