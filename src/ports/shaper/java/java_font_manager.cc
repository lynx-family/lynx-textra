// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/platform/java/java_font_manager.h>
#include <textra/platform/java/java_typeface.h>
#include <textra/platform/java/tttext_jni_proxy.h>

namespace ttoffice {
namespace tttext {
JavaFontManager::JavaFontManager() {
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto* env = proxy.GetCurrentJNIEnv();
  auto inst = env->NewObject(
      static_cast<jclass>(proxy.JavaFontManager_class_->get()),
      proxy.JavaFontManager_method_init_, reinterpret_cast<uint64_t>(this));
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
  typeface_list_.push_back(std::make_shared<JavaTypeface>(0));
  return typeface_list_.back();
}
std::shared_ptr<JavaTypeface> JavaFontManager::matchTypeface(
    const FontDescriptor& fd) {
  auto iter = typeface_map_.find(fd);
  if (iter != typeface_map_.end()) {
    return iter->second;
  }
  auto& proxy = TTTextJNIProxy::GetInstance();
  auto env = proxy.GetCurrentJNIEnv();
  long raw_pointer = 0;
  std::shared_ptr<JavaTypeface> obj = nullptr;
  if (fd.platform_font_ != 0) {
    raw_pointer = env->CallLongMethod(
        java_instance_->get(),
        proxy.JavaFontManager_method_onMatchTypefaceIndex_, fd.platform_font_);
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
        java_instance_->get(), proxy.JavaFontManager_method_onMatchFamilyStyle_,
        j_fontfamily, fd.font_style_.GetWeight(),
        fd.font_style_.GetSlant() != FontStyle::kUpright_Slant,
        reinterpret_cast<uint64_t>(obj.get()));
  }
  if (raw_pointer != (jlong)obj.get()) {
    auto typeface =
        (reinterpret_cast<JavaTypeface*>(raw_pointer))->shared_from_this();
    obj = std::static_pointer_cast<JavaTypeface>(typeface);
  }
  typeface_map_[fd] = obj;
  return obj;
}
}  // namespace tttext
}  // namespace ttoffice
