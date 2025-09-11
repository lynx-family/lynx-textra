// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "napi_init.h"

#include <hilog/log.h>
#include <napi/native_api.h>
#include <native_drawing/drawing_bitmap.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_path.h>
#include <native_drawing/drawing_pen.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_types.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>

#include <cinttypes>
#include <string>

#include "oh_drawing_newapi_test.h"
#include "oh_drawing_test.h"
#include "textra_test.h"

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
      {"buildParagraph", nullptr, NAPI_Global_buildParagraph, nullptr, nullptr,
       nullptr, napi_default, nullptr},
      {"layoutParagraph", nullptr, NAPI_Global_layoutParagraph, nullptr,
       nullptr, nullptr, napi_default, nullptr},
      {"drawParagraph", nullptr, NAPI_Global_drawParagraph, nullptr, nullptr,
       nullptr, napi_default, nullptr},
      {"buildCanvas", nullptr, NAPI_Global_buildCanvas, nullptr, nullptr,
       nullptr, napi_default, nullptr},
      {"initLayoutContext", nullptr, NAPI_Global_initLayoutContext, nullptr,
       nullptr, nullptr, napi_default, nullptr},
  };

  napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
  return exports;
}

EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "perftest",
    .nm_priv = ((void*)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNativeTextModule(void) {
  napi_module_register(&demoModule);
}

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3200     // 全局domain宏，标识业务领域
#define LOG_TAG "NativeText"  // 全局tag宏，标识模块日志tag
void OHBuildParagraph(OHContext* context, const std::string& content);
void OHLayoutParagraph(OHContext* context, double width);
void OHDrawParagraph(OHContext* context, OH_Drawing_Canvas* canvas);
using BuildParagraphFunc = void (*)(void* context, const std::string& content);
using LayoutParagraphFunc = void (*)(void* context, double width);
using DrawParagraphFunc = void (*)(void* context, OH_Drawing_Canvas* canvas);

struct LayoutContext {
  OH_Drawing_Bitmap* bitmap_ = nullptr;
  std::unique_ptr<OHContext> oh_context_ = nullptr;
  std::unique_ptr<TextraContext> textra_context_ = nullptr;
  std::unique_ptr<OHNewContext> oh_new_context_ = nullptr;
  void* context_ = nullptr;
  BuildParagraphFunc build_paragraph_func_ = nullptr;
  LayoutParagraphFunc layout_paragraph_func_ = nullptr;
  DrawParagraphFunc draw_paragraph_func_ = nullptr;
};

static void NAPI_Deref(napi_env env, void* data, void* hint) {
  OH_LOG_INFO(LOG_APP, "LayoutCost NAPI_Deref");
  auto context = static_cast<LayoutContext*>(data);
  if (context && context->bitmap_) {
    OH_Drawing_BitmapDestroy(context->bitmap_);
  }
  delete context;
}

napi_value NAPI_Global_initLayoutContext(napi_env env,
                                         napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2] = {nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  int32_t mode;
  napi_get_value_int32(env, argv[1], &mode);
  auto* context = new LayoutContext;
  if (mode == 0) {
    context->textra_context_ = std::make_unique<TextraContext>();
    context->context_ = context->textra_context_.get();
    context->build_paragraph_func_ = &TextraBuildParagraph;
    context->layout_paragraph_func_ = &TextraLayoutParagraph;
    context->draw_paragraph_func_ = &TextraDrawParagraph;
  } else if (mode == 1) {
    context->oh_context_ = std::make_unique<OHContext>();
    context->context_ = context->oh_context_.get();
    context->build_paragraph_func_ = &OHBuildParagraph;
    context->layout_paragraph_func_ = &OHLayoutParagraph;
    context->draw_paragraph_func_ = &OHDrawParagraph;
  } else if (mode == 2) {
    context->oh_new_context_ = std::make_unique<OHNewContext>();
    context->context_ = context->oh_new_context_.get();
    context->build_paragraph_func_ = &OHNewBuildParagraph;
    context->layout_paragraph_func_ = &OHNewLayoutParagraph;
    context->draw_paragraph_func_ = &OHNewDrawParagraph;
  }
  napi_wrap(env, argv[0], context, NAPI_Deref, nullptr, nullptr);
  return nullptr;
}

napi_value NAPI_Global_buildCanvas(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value argv[3] = {nullptr, nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  void* temp = nullptr;
  auto status = napi_unwrap(env, argv[0], &temp);
  if (status != napi_ok) {
    return nullptr;
  }
  auto* context = reinterpret_cast<LayoutContext*>(temp);

  double width = 0, height = 0;
  napi_get_value_double(env, argv[1], &width);
  napi_get_value_double(env, argv[2], &height);

  // 创建一个位图对象
  OH_Drawing_Bitmap* bitmap = OH_Drawing_BitmapCreate();
  OH_Drawing_BitmapFormat cFormat{COLOR_FORMAT_RGBA_8888, ALPHA_FORMAT_PREMUL};
  OH_Drawing_BitmapBuild(bitmap, width, height, &cFormat);
  context->bitmap_ = bitmap;
  return nullptr;
}

napi_value NAPI_Global_buildParagraph(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2] = {nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  void* temp = nullptr;
  auto status = napi_unwrap(env, argv[0], &temp);
  if (status != napi_ok) {
    return nullptr;
  }
  auto* context = reinterpret_cast<LayoutContext*>(temp);

  size_t str_len = 0;
  napi_get_value_string_utf8(env, argv[1], nullptr, 0, &str_len);
  std::string test_content;
  test_content.resize(str_len + 1);
  napi_get_value_string_utf8(env, argv[1], test_content.data(), str_len,
                             nullptr);
  test_content[str_len] = 0;

  if (context && context->build_paragraph_func_)
    context->build_paragraph_func_(context->context_, test_content);
  return nullptr;
}

napi_value NAPI_Global_layoutParagraph(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2] = {nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  void* temp = nullptr;
  auto status = napi_unwrap(env, argv[0], &temp);
  if (status != napi_ok) {
    return nullptr;
  }
  auto* context = reinterpret_cast<LayoutContext*>(temp);

  double layout_width;
  status = napi_get_value_double(env, argv[1], &layout_width);

  if (context && context->layout_paragraph_func_)
    context->layout_paragraph_func_(context->context_, layout_width);
  return nullptr;
}

napi_value NAPI_Global_drawParagraph(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value argv[4] = {nullptr, nullptr, nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  void* temp = nullptr;
  auto status = napi_unwrap(env, argv[0], &temp);
  LayoutContext* context = nullptr;
  auto* target_canvas = reinterpret_cast<OH_Drawing_Canvas*>(temp);
  status = napi_unwrap(env, argv[1], &temp);
  context = reinterpret_cast<LayoutContext*>(temp);
  if (target_canvas == nullptr) {
    target_canvas = OH_Drawing_CanvasCreate();
    OH_Drawing_CanvasBind(target_canvas, context->bitmap_);
  }

  double x = 0, y = 0;
  napi_get_value_double(env, argv[2], &x);
  napi_get_value_double(env, argv[3], &y);
  if (!context) return nullptr;

  if (context->draw_paragraph_func_)
    context->draw_paragraph_func_(context->context_, target_canvas);
  return nullptr;
}
