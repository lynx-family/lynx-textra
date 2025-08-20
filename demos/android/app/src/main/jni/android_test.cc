// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <jni.h>
#include <textra/fontmgr_collection.h>
#include <textra/i_font_manager.h>
#include <textra/platform/java/java_canvas_helper.h>
#include <textra/platform/java/java_font_manager.h>

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

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_lynx_textra_demo_PageView_drawCppTestCase(JNIEnv* env, jobject thiz,
                                                   jlong font_manager_handler,
                                                   jint case_id, float width) {
  auto java_font_manager =
      ((JavaFontManager*)font_manager_handler)->shared_from_this();
  JavaCanvasHelper canvas_helper;
  tttext::FontmgrCollection font_collection(java_font_manager);
  ParagraphTest paragraph_test(kSystem, &font_collection);
  paragraph_test.TestWithId(&canvas_helper, width, case_id);
  auto& buffer = canvas_helper.GetBuffer();
  auto* data = buffer.GetBuffer();
  auto length = buffer.GetSize();
  auto array = env->NewByteArray(length);
  env->SetByteArrayRegion(array, 0, length, reinterpret_cast<jbyte*>(data));
  return array;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_lynx_textra_demo_PageView_nativeCreateFontManager(JNIEnv* env,
                                                           jobject thiz) {
  auto java_font_manager =
      TTTextJNIProxy::GetInstance().GetDefaultFontManager();
  return java_font_manager->GetInstance();
}
