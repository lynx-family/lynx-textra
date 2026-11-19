// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.

#include <jni.h>
#include <textra/platform_helper.h>

#include "demos/capi/textra_demo_cpp.hpp"

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_lynx_textra_demo_CapiDemoActivity_nativeRender(JNIEnv* env, jclass,
                                                        jobject native_canvas,
                                                        jint case_id,
                                                        jboolean capi) {
  using namespace ttoffice::tttext;
  if (case_id < 0 || static_cast<size_t>(case_id) >= TEXTRA_DEMO_CASE_COUNT * 3)
    return nullptr;
  const bool rounded = case_id >= static_cast<jint>(TEXTRA_DEMO_CASE_COUNT);
  const float font_size =
      case_id >= static_cast<jint>(TEXTRA_DEMO_CASE_COUNT * 2) ? 24.5f : 24.f;
  case_id %= static_cast<jint>(TEXTRA_DEMO_CASE_COUNT);
  textra_demo_metrics_t metrics{};
  textra_status_t status = TEXTRA_STATUS_OK;
  if (capi) {
    status = textra_android_runtime_initialize(env);
    textra_canvas_options_t canvas{
        sizeof(canvas), TEXTRA_RENDERING_BACKEND_ANDROID, 0, native_canvas};
    if (!status)
      status = textra_demo_render_config(
          TEXTRA_RENDERING_BACKEND_ANDROID, nullptr, nullptr, &canvas, case_id,
          320, 200, &metrics, rounded ? TEXTRA_TRUE : TEXTRA_FALSE, font_size);
    textra_text_layout_clear_shape_cache();
  } else {
    auto manager = PlatformHelper::CreateFontManager();
    auto canvas = PlatformHelper::CreateCanvasHelper(native_canvas, manager);
    if (!manager || !canvas) return nullptr;
    canvas->StartPaint();
    if (!textra_demo::RenderCpp(RenderingBackend::kAndroid, manager, nullptr,
                                canvas.get(), case_id, 320, 200, &metrics,
                                rounded, font_size))
      status = TEXTRA_STATUS_INTERNAL_ERROR;
    canvas->EndPaint();
    if (env->ExceptionCheck()) return nullptr;
  }
  if (status) {
    auto exception = env->FindClass("java/lang/IllegalStateException");
    char message[80];
    snprintf(message, sizeof(message), "Textra demo case %d: status %d",
             case_id, status);
    env->ThrowNew(exception, message);
    return nullptr;
  }
  jfloat values[]{metrics.width, metrics.height,
                  static_cast<float>(metrics.lines)};
  auto result = env->NewFloatArray(3);
  if (result) env->SetFloatArrayRegion(result, 0, 3, values);
  return result;
}
