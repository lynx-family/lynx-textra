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

extern "C" JNIEXPORT void JNICALL
Java_com_lynx_textra_perftest_TTTextView_nativeDrawContent(
    JNIEnv* env, jobject thiz, jobject target, jlong manager_handler,
    jlong region_handler) {
  auto* manager = reinterpret_cast<TTTextManager*>(manager_handler);
  auto* region = manager->GetRetgion();
  if (!region) return;
  auto canvas = PlatformHelper::CreateCanvasHelper(
      target, manager->GetDefaultFontManager());
  if (!canvas) return;
  canvas->StartPaint();
  LayoutDrawer drawer(canvas.get());
  drawer.DrawLayoutPage(region);
  canvas->EndPaint();
}
