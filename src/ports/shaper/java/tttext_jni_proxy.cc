// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <jni.h>
#include <textra/icu_wrapper.h>
#include <textra/platform/java/java_typeface.h>
#include <textra/platform/java/tttext_jni_proxy.h>

#include "utils/u_8_string.h"

namespace ttoffice {
namespace tttext {

constexpr char CLASS_JAVA_TYPEFACE[] = "com/lynx/textra/JavaTypeface";
constexpr char CLASS_JAVA_SHAPERESULT[] = "com/lynx/textra/JavaShapeResult";
constexpr char CLASS_JAVA_SHAPER[] = "com/lynx/textra/JavaShaper";
constexpr char CLASS_JAVA_FONTMANAGER[] = "com/lynx/textra/JavaFontManager";
JavaVM* TTTextJNIProxy::java_vm_ = nullptr;
jfieldID TTTextJNIProxy::JavaShapeResult_field_glyph_count_ = nullptr;
jfieldID TTTextJNIProxy::JavaShapeResult_field_glyphs_ = nullptr;
jfieldID TTTextJNIProxy::JavaShapeResult_field_advances_ = nullptr;
jfieldID TTTextJNIProxy::JavaShapeResult_field_position_x_ = nullptr;
jfieldID TTTextJNIProxy::JavaShapeResult_field_position_y_ = nullptr;
jfieldID TTTextJNIProxy::JavaShapeResult_field_typeface_instance_ = nullptr;
std::unique_ptr<ScopedGlobalRef> TTTextJNIProxy::JavaShaper_class_ = nullptr;
jmethodID TTTextJNIProxy::JavaShaper_method_init_ = nullptr;
jmethodID TTTextJNIProxy::JavaShaper_method_OnShapeText_ = nullptr;
jmethodID TTTextJNIProxy::JavaShaper_method_OnShapeTextNew_ = nullptr;
std::unique_ptr<ScopedGlobalRef> TTTextJNIProxy::JavaFontManager_class_ =
    nullptr;
jmethodID TTTextJNIProxy::JavaFontManager_method_init_ = nullptr;
jmethodID TTTextJNIProxy::JavaFontManager_method_onMatchFamilyStyle_ = nullptr;
jmethodID TTTextJNIProxy::JavaFontManager_method_onMatchTypefaceIndex_ =
    nullptr;
jmethodID TTTextJNIProxy::JavaTypeface_method_GetFontMetrics = nullptr;
jmethodID TTTextJNIProxy::JavaTypeface_method_GetTextBounds = nullptr;

TTTextJNIProxy::TTTextJNIProxy() = default;
TTTextJNIProxy::~TTTextJNIProxy() = default;
void TTTextJNIProxy::Initial() {
  font_manager_ = std::make_shared<JavaFontManager>();
}
std::shared_ptr<JavaFontManager> TTTextJNIProxy::GetDefaultFontManager() {
  return font_manager_;
}
TTTextJNIProxy& TTTextJNIProxy::GetInstance() {
  static TTTextJNIProxy instance;
  return instance;
}
JavaVM* TTTextJNIProxy::GetJavaVm() { return java_vm_; }
JNIEnv* TTTextJNIProxy::GetCurrentJNIEnv() {
  thread_local JNIEnv* env = nullptr;
  if (env == nullptr) {
    java_vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  }
  return env;
}
void TTTextJNIProxy::InitialJNI(JNIEnv* env) {
  env->GetJavaVM(&java_vm_);
  auto clzz_java_shape_result = env->FindClass(CLASS_JAVA_SHAPERESULT);
  JavaShapeResult_field_glyph_count_ =
      env->GetFieldID(clzz_java_shape_result, "glyph_count_", "I");
  JavaShapeResult_field_glyphs_ =
      env->GetFieldID(clzz_java_shape_result, "glyphs_", "[S");
  JavaShapeResult_field_advances_ =
      env->GetFieldID(clzz_java_shape_result, "advance_", "[F");
  JavaShapeResult_field_position_x_ =
      env->GetFieldID(clzz_java_shape_result, "position_x_", "[F");
  JavaShapeResult_field_position_y_ =
      env->GetFieldID(clzz_java_shape_result, "position_y_", "[F");
  JavaShapeResult_field_typeface_instance_ =
      env->GetFieldID(clzz_java_shape_result, "typeface_instance_", "[J");
  env->DeleteLocalRef(clzz_java_shape_result);
  auto clzz_java_shaper = env->FindClass(CLASS_JAVA_SHAPER);
  JavaShaper_class_ = std::make_unique<ScopedGlobalRef>(env, clzz_java_shaper);
  JavaShaper_method_init_ = env->GetMethodID(
      clzz_java_shaper, "<init>", "(Lcom/lynx/textra/JavaFontManager;)V");
  JavaShaper_method_OnShapeText_ = env->GetMethodID(
      clzz_java_shaper, "OnShapeText",
      "(Ljava/lang/String;Lcom/lynx/textra/JavaTypeface;FZZ)[F");
  JavaShaper_method_OnShapeTextNew_ = env->GetMethodID(
      clzz_java_shaper, "OnShapeTextNew",
      "(Ljava/lang/String;Lcom/lynx/textra/JavaTypeface;FZZ)Lcom/"
      "lynx/textra/JavaShapeResult;");
  env->DeleteLocalRef(clzz_java_shaper);
  auto clzz_font_manager = env->FindClass(CLASS_JAVA_FONTMANAGER);
  JavaFontManager_class_ =
      std::make_unique<ScopedGlobalRef>(env, clzz_font_manager);
  JavaFontManager_method_init_ =
      env->GetMethodID(clzz_font_manager, "<init>", "(J)V");
  JavaFontManager_method_onMatchFamilyStyle_ = env->GetMethodID(
      clzz_font_manager, "onMatchFamilyStyle", "(Ljava/lang/String;IZJ)J");
  JavaFontManager_method_onMatchTypefaceIndex_ =
      env->GetMethodID(clzz_font_manager, "onMatchTypefaceIndex", "(J)J");
  env->DeleteLocalRef(clzz_font_manager);
  auto clzz_JavaTypeface = env->FindClass(CLASS_JAVA_TYPEFACE);
  JavaTypeface_method_GetFontMetrics =
      env->GetMethodID(clzz_JavaTypeface, "GetFontMetrics", "(F)[F");
  JavaTypeface_method_GetTextBounds =
      env->GetMethodID(clzz_JavaTypeface, "GetTextBounds", "([CF)[F");
  env->DeleteLocalRef(clzz_JavaTypeface);
}
jstring TTTextJNIProxy::CreateJavaStringFromU32(JNIEnv* env,
                                                const char32_t* str,
                                                uint32_t len) {
  auto u16string = base::U32StringToU16(str, len);
  return env->NewString(reinterpret_cast<const jchar*>(u16string.c_str()),
                        u16string.length());
}
jstring TTTextJNIProxy::CreateJavaStringFromU16(JNIEnv* env,
                                                const char16_t* str,
                                                uint32_t len) {
  return env->NewString(reinterpret_cast<const jchar*>(str), len);
}
jstring TTTextJNIProxy::CreateJavaStringFromU8(JNIEnv* env, const char* str,
                                               uint32_t len) {
  auto u16string = base::U8StringToU16(str, len);
  return env->NewString(reinterpret_cast<const jchar*>(u16string.c_str()),
                        u16string.length());
}
}  // namespace tttext
}  // namespace ttoffice

void WarmICU() {
  // warm bidi functions
  char32_t character[] = {1, 0};
  uint32_t char_count = 1;
  uint32_t out_buffer_32;
  uint8_t out_buffer_8;
  tttext::ICUWrapper::GetInstance().BidiInit(
      character, char_count, tttext::WriteDirection::kAuto, &out_buffer_8,
      &out_buffer_32, &out_buffer_32);
}

extern "C" JNIEXPORT void JNICALL
JavaTypefaceBindNativeInstance(JNIEnv* env, jclass clazz, long native_handler,
                               jobject java_instance, jint index) {
  auto typeface = reinterpret_cast<tttext::JavaTypeface*>(native_handler);
  typeface->BindJavaHandler(java_instance, index);
}

extern "C" JNIEXPORT void JNICALL JavaFontManagerBindNativeInstance(
    JNIEnv* env, jclass clazz, long native_handler, jobject java_instance) {
  auto font_manager =
      reinterpret_cast<tttext::JavaFontManager*>(native_handler);
  font_manager->BindJavaHandler();
}

extern "C" JNIEXPORT jlong JNICALL JavaFontManagerCreateNativeTypeface(
    JNIEnv* env, jclass clazz, long native_handler) {
  auto font_manager =
      reinterpret_cast<tttext::JavaFontManager*>(native_handler);
  return (jlong)font_manager->CreateNativeTypeface().get();
}

// define the methods to be registered
static JNINativeMethod java_typeface_methods[] = {
    {"BindNativeHandler", "(JLcom/lynx/textra/JavaTypeface;I)V",
     reinterpret_cast<void*>(JavaTypefaceBindNativeInstance)},
};

static JNINativeMethod java_font_manager_methods[] = {
    {"BindNativeInstance", "(JLcom/lynx/textra/JavaFontManager;)V",
     reinterpret_cast<void*>(JavaFontManagerBindNativeInstance)},
    {"CreateNativeTypeface", "(J)J",
     reinterpret_cast<void*>(JavaFontManagerCreateNativeTypeface)},
};

extern "C" JNIEXPORT void JNICALL TTTextNativeInitial(JNIEnv* env,
                                                      jclass clazz) {
  tttext::TTTextJNIProxy::GetInstance().Initial();
  WarmICU();
}

extern "C" JNIEXPORT jobject JNICALL
TTTextNativeGetDefaultFontManager(JNIEnv* env, jclass clazz) {
  auto font_manager =
      tttext::TTTextJNIProxy::GetInstance().GetDefaultFontManager();
  return font_manager->GetInstance();
}

static JNINativeMethod tttext_methods[] = {
    {"nativeInitialCache", "()V", reinterpret_cast<void*>(TTTextNativeInitial)},
    {"nativeGetDefaultFontManager", "()Lcom/lynx/textra/JavaFontManager;",
     reinterpret_cast<void*>(TTTextNativeGetDefaultFontManager)},
};

int RegisterJNIMethods(JNIEnv* env, const char* class_name,
                       JNINativeMethod* methods, jint num_methods) {
  jclass clazz = env->FindClass(class_name);
  if (clazz == NULL) {
    return -1;
  }

  // register native methods
  if (env->RegisterNatives(clazz, methods, num_methods)) {
    return -1;
  }
  return 0;
}
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  // this function maybe call multi times by System.loadlibrary()
  // ignore subsequent calls.
  if (tttext::TTTextJNIProxy::java_vm_ != nullptr) {
    return JNI_VERSION_1_6;
  }
  tttext::TTTextJNIProxy::java_vm_ = vm;
  JNIEnv* env = NULL;

  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return -1;
  }

  RegisterJNIMethods(env, "com/lynx/textra/JavaTypeface", java_typeface_methods,
                     std::size(java_typeface_methods));

  RegisterJNIMethods(env, "com/lynx/textra/JavaFontManager",
                     java_font_manager_methods,
                     std::size(java_font_manager_methods));

  RegisterJNIMethods(env, "com/lynx/textra/TTText", tttext_methods,
                     std::size(tttext_methods));

  tttext::TTTextJNIProxy::InitialJNI(env);
  // return the used JNI version
  return JNI_VERSION_1_6;
}
