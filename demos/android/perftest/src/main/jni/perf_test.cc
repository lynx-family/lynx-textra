// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <jni.h>
#include <textra/layout_drawer.h>

#include "tt_text_manager.h"

using namespace tttext;

void WarmICU() {
  // warm bidi functions
  char32_t character[] = {1, 0};
  uint32_t char_count = 1;
  uint32_t out_buffer_32;
  uint8_t out_buffer_8;
  ICUWrapper::GetInstance().BidiInit(character, char_count,
                                     WriteDirection::kAuto, &out_buffer_8,
                                     &out_buffer_32, &out_buffer_32);
}

extern "C" JNIEXPORT void JNICALL
Java_com_lynxtextra_TTText_nativeInitialCache(JNIEnv* env, jclass clazz) {
  WarmICU();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_lynx_textra_perftest_TTTextView_nativeCreateManager(JNIEnv* env,
                                                             jobject thiz) {
  return reinterpret_cast<jlong>(new TTTextManager);
}
extern "C" JNIEXPORT void JNICALL
Java_com_lynx_textra_perftest_TTTextView_nativeCreateParagraph(
    JNIEnv* env, jobject thiz, jlong manager_handler) {
  auto* manager = reinterpret_cast<TTTextManager*>(manager_handler);
  manager->CreateParagraph();
}
extern "C" JNIEXPORT void JNICALL
Java_com_lynx_textra_perftest_TTTextView_nativeAddText(
    JNIEnv* env, jobject thiz, jlong manager_handler, jstring content,
    jfloat font_size, jint color) {
  auto* manager = reinterpret_cast<TTTextManager*>(manager_handler);
  const char* cStr = env->GetStringUTFChars(content, nullptr);
  if (cStr == nullptr) {
    return;  // Memory allocation failed
  }
  manager->AddText(cStr, font_size, color);
  env->ReleaseStringUTFChars(content, cStr);
}

extern "C" JNIEXPORT void JNICALL
Java_com_lynx_textra_perftest_TTTextView_nativeDestroyManager(
    JNIEnv* env, jobject thiz, jlong manager_handler) {
  auto* manager = reinterpret_cast<TTTextManager*>(manager_handler);
  delete manager;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_lynx_textra_perftest_TTTextView_nativeLayoutContent(
    JNIEnv* env, jobject thiz, jlong manager_handler, jfloat height) {
  auto* manager = reinterpret_cast<TTTextManager*>(manager_handler);
  auto ret = manager->LayoutContent(height);
  return reinterpret_cast<jlong>(ret);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_lynx_textra_perftest_TTTextView_nativeGetFontManager(
    JNIEnv* env, jobject thiz, jlong manager_handler) {
  auto* manager = reinterpret_cast<TTTextManager*>(manager_handler);
  auto font_manager = manager->GetDefaultFontManager();
  auto java_font_manager =
      std::static_pointer_cast<JavaFontManager>(font_manager);
  return java_font_manager->GetInstance();
}
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_lynx_textra_perftest_TTTextView_nativeDrawContent(
    JNIEnv* env, jobject thiz, jobject canvas, jlong manager_handler,
    jlong region_handler) {
  auto* manager = reinterpret_cast<TTTextManager*>(manager_handler);
  auto* canvas_helper = manager->GetCanvas();
  LayoutDrawer drawer(reinterpret_cast<ICanvasHelper*>(canvas_helper));
  auto* region = manager->GetRetgion();
  if (region == nullptr) {
    return nullptr;
  }
  canvas_helper->ClearBuffer();
  drawer.DrawLayoutPage(region);
  auto& stream = canvas_helper->GetBuffer();
  auto data = stream.GetBuffer();
  auto length = stream.GetSize();
  auto array = env->NewByteArray(length);
  env->SetByteArrayRegion(array, 0, length, reinterpret_cast<jbyte*>(data));
  return array;
}
