// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <jni.h>
#include <textra/fontmgr_collection.h>
#include <textra/icu_wrapper.h>
#include <textra/layout_drawer.h>
#include <textra/paragraph.h>
#include <textra/platform/java/java_canvas_helper.h>
#include <textra/platform/java/tttext_jni_proxy.h>
#include <textra/text_layout.h>

#include "examples/json_parser/json_parser.h"
#include "java_drawer_callback.h"
#include "src/textlayout/tt_shaper.h"

using namespace ttoffice::tttext;

namespace {
constexpr float kDefaultHeight = std::numeric_limits<float>::max();
constexpr LayoutMode kDefaultWidthMode = LayoutMode::kAtMost;
constexpr LayoutMode kDefaultHeightMode = LayoutMode::kAtMost;
}  // namespace

extern "C" JNIEXPORT jlong JNICALL
Java_com_lynx_textra_demo_JavaDrawerCallback_nativeCreateDrawerCallback(
    JNIEnv* env, jobject thiz) {
  return (uint64_t)(new JavaDrawerCallback(env, thiz));
}
extern "C" JNIEXPORT void JNICALL
Java_com_lynx_textra_demo_JavaDrawerCallback_nativeDestroyDrawerCallback(
    JNIEnv* env, jobject thiz, jlong instance) {
  ((JavaDrawerCallback*)instance)->SetJNIEnv(env);
  delete (JavaDrawerCallback*)instance;
}

extern "C" JNIEXPORT void JNICALL
Java_com_lynxtextra_TTText_nativeInitialCache(JNIEnv* env, jclass clazz) {}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_lynx_textra_demo_PageView_drawJsonTestCase(JNIEnv* env, jobject thiz,
                                                    jlong font_manager_handler,
                                                    jbyteArray jsonBytes,
                                                    jfloat width) {
  jsize length = env->GetArrayLength(jsonBytes);
  jbyte* bytes = env->GetByteArrayElements(jsonBytes, nullptr);
  std::string jsonString(reinterpret_cast<char*>(bytes), length);
  env->ReleaseByteArrayElements(jsonBytes, bytes, JNI_ABORT);

  auto jsonDoc = ParseJsonStringIntoTTTextObject(jsonString);
  auto& regions_from_json = jsonDoc->layout_regions_;
  if (regions_from_json.empty()) {
    regions_from_json.emplace_back(std::make_unique<LayoutRegion>(
        width, kDefaultHeight, kDefaultWidthMode, kDefaultHeightMode));
  } else {
    auto& old_region = regions_from_json[0];
    auto new_region = std::make_unique<LayoutRegion>(
        width, old_region->GetPageHeight(), old_region->GetWidthMode(),
        old_region->GetHeightMode());
    old_region = std::move(new_region);
  }

  auto java_font_manager =
      ((JavaFontManager*)font_manager_handler)->shared_from_this();
  tttext::FontmgrCollection font_collection(java_font_manager);
  auto region = LayoutJsonDocument(jsonDoc.get(), &font_collection,
                                   tttext::ShaperType::kSystem);

  JavaCanvasHelper helper;
  tttext::LayoutDrawer drawer(&helper);
  drawer.DrawLayoutPage(region.get());

  auto& buffer = helper.GetBuffer();
  auto* ret = env->NewByteArray(buffer.GetSize());
  env->SetByteArrayRegion(ret, 0, buffer.GetSize(),
                          (int8_t*)buffer.GetBuffer());
  return ret;
}
