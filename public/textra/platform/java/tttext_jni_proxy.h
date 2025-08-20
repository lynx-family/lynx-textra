// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_JAVA_TTTEXT_JNI_PROXY_H_
#define PUBLIC_TEXTRA_PLATFORM_JAVA_TTTEXT_JNI_PROXY_H_

#include <jni.h>
#include <textra/macro.h>
#include <textra/platform/java/java_font_manager.h>
#include <textra/platform/java/scoped_global_ref.h>

#include <memory>
namespace ttoffice {
namespace tttext {
class TTTextJNIProxy {
 private:
  TTTextJNIProxy();

 public:
  ~TTTextJNIProxy();

 public:
  void Initial();
  std::shared_ptr<JavaFontManager> GetDefaultFontManager();

 public:
  L_EXPORT static TTTextJNIProxy& GetInstance();
  static JavaVM* GetJavaVm();
  static JNIEnv* GetCurrentJNIEnv();
  static void InitialJNI(JNIEnv* env);
  static jstring CreateJavaStringFromU32(JNIEnv* env, const char32_t* str,
                                         uint32_t len);
  static jstring CreateJavaStringFromU16(JNIEnv* env, const char16_t* str,
                                         uint32_t len);
  static jstring CreateJavaStringFromU8(JNIEnv* env, const char* str,
                                        uint32_t len);

 private:
  std::shared_ptr<JavaFontManager> font_manager_;

 public:
  static JavaVM* java_vm_;
  static jfieldID JavaShapeResult_field_glyph_count_;
  static jfieldID JavaShapeResult_field_glyphs_;
  static jfieldID JavaShapeResult_field_advances_;
  static jfieldID JavaShapeResult_field_position_x_;
  static jfieldID JavaShapeResult_field_position_y_;
  static jfieldID JavaShapeResult_field_typeface_instance_;
  static std::unique_ptr<ScopedGlobalRef> JavaShaper_class_;
  static jmethodID JavaShaper_method_init_;
  static jmethodID JavaShaper_method_OnShapeText_;
  static jmethodID JavaShaper_method_OnShapeTextNew_;
  static std::unique_ptr<ScopedGlobalRef> JavaFontManager_class_;
  static jmethodID JavaFontManager_method_init_;
  static jmethodID JavaFontManager_method_onMatchFamilyStyle_;
  static jmethodID JavaFontManager_method_onMatchTypefaceIndex_;
  static jmethodID JavaTypeface_method_GetFontMetrics;
  static jmethodID JavaTypeface_method_GetTextBounds;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_JAVA_TTTEXT_JNI_PROXY_H_
