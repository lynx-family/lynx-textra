// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <jni.h>
#include <textra/fontmgr_collection.h>
#include <textra/i_font_manager.h>
#include <textra/platform_helper.h>

#include "paragraph_test.h"

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_lynx_textra_demo_PageView_getCppTestList(JNIEnv* env, jclass clazz) {
  auto test_cases = ParagraphTest::GetTestCaseIdAndName();

  jclass cppTestItemClass =
      env->FindClass("com/lynx/textra/demo/TestCaseListFragment$CppTestItem");
  jmethodID constructor =
      env->GetMethodID(cppTestItemClass, "<init>", "(ILjava/lang/String;)V");

  jobjectArray result =
      env->NewObjectArray(test_cases.size(), cppTestItemClass, nullptr);
  for (size_t i = 0; i < test_cases.size(); ++i) {
    jstring name = env->NewStringUTF(test_cases[i].second.c_str());
    jobject cppTestItem = env->NewObject(cppTestItemClass, constructor,
                                         test_cases[i].first, name);
    env->SetObjectArrayElement(result, i, cppTestItem);
    env->DeleteLocalRef(name);
    env->DeleteLocalRef(cppTestItem);
  }
  return result;
}

extern "C" JNIEXPORT void JNICALL
Java_com_lynx_textra_demo_PageView_drawCppTestCase(JNIEnv* env, jobject thiz,
                                                   jobject native_canvas,
                                                   jint case_id,
                                                   float width_px) {
  auto manager = PlatformHelper::CreateFontManager();
  auto canvas = PlatformHelper::CreateCanvasHelper(native_canvas, manager);
  if (!canvas) return;
  tttext::FontmgrCollection font_collection(manager);
  ParagraphTest paragraph_test(RenderingBackend::kAndroid, &font_collection);
  canvas->StartPaint();
  paragraph_test.TestWithId(canvas.get(), width_px, case_id);
  canvas->EndPaint();
}
