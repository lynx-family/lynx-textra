// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "native_paragraph.h"

#include "arkui_base_node.h"
#include "harmony_run_delegate.h"
#include "js_value_helper.h"
#include "napi_init.h"
#include "paragraph_style.h"

namespace ttoffice {
namespace tttext {
void JsObjToParagraphStyle(napi_env env, napi_value& js_obj,
                           ParagraphStyle& paragraph_style) {
  uint32_t alignment = 0;
  JsValueHelperGetUint32Property(env, js_obj, "alignment", &alignment);
  paragraph_style.SetHorizontalAlign(
      static_cast<ParagraphHorizontalAlignment>(alignment));
}

void JsObjToRunStyle(napi_env env, napi_value& js_obj, Style& run_style) {
  napi_value js_value;
  napi_get_named_property(env, js_obj, "font_families", &js_value);
  FontDescriptor descriptor;
  JsValueHelperGetStringArrayValue(env, js_value, descriptor.font_family_list_);
  run_style.SetFontDescriptor(descriptor);

  double text_size;
  JSValueHelperGetDoubleProperty(env, js_obj, "text_size", &text_size);
  run_style.SetTextSize(text_size);
  uint32_t color;
  JsValueHelperGetUint32Property(env, js_obj, "text_color", &color);
  run_style.SetForegroundColor(TTColor(color));
}

napi_value NAPI_Global_bindNativeParagraph(napi_env env,
                                           napi_callback_info info) {
  auto* native_paragraph = new NativeParagraph();
  size_t argc = 1;
  napi_value args[1];
  napi_value this_arg;
  napi_get_cb_info(env, info, &argc, args, &this_arg, NULL);
  napi_status status =
      napi_wrap(env, args[0], native_paragraph, NAPI_Deref,
                reinterpret_cast<void*>(HintType::kParagraph), NULL);
  if (status != napi_ok) {
    delete native_paragraph;
  }

  return this_arg;
}

napi_value NAPI_Global_setParagraphStyle(napi_env env,
                                         napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr, nullptr};
  napi_value this_arg = nullptr;
  auto status = napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
  NativeParagraph* native_paragraph;
  napi_unwrap(env, args[0], reinterpret_cast<void**>(&native_paragraph));

  ParagraphStyle para_style;
  JsObjToParagraphStyle(env, args[1], para_style);
  native_paragraph->paragraph_->SetParagraphStyle(&para_style);
  return nullptr;
}

napi_value NAPI_Global_addTextRun(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value args[3] = {nullptr, nullptr};
  napi_value this_arg;
  napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
  NativeParagraph* native_paragraph;
  napi_unwrap(env, args[0], reinterpret_cast<void**>(&native_paragraph));

  std::string content;
  JsValueHelperGetStringValue(env, args[1], content);

  auto content_len = static_cast<uint32_t>(content.length());
  while (content[content_len - 1] == 0) content_len--;

  Style run_style;
  JsObjToRunStyle(env, args[2], run_style);
  native_paragraph->paragraph_->AddTextRun(&run_style, content.c_str(),
                                           content_len);
  return nullptr;
}

napi_value NAPI_Global_addFrameNode(napi_env env, napi_callback_info info) {
  size_t argc = 4;
  napi_value args[4] = {nullptr, nullptr, nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  NativeParagraph* native_paragraph;
  napi_unwrap(env, args[0], reinterpret_cast<void**>(&native_paragraph));

  ArkUI_NodeHandle contentHandle = 0;
  auto status =
      OH_ArkUI_GetNodeHandleFromNapiValue(env, args[1], &contentHandle);
  if (status != napi_ok) {
    return nullptr;
  }

  double width = 0, height = 0;
  napi_get_value_double(env, args[2], &width);
  napi_get_value_double(env, args[3], &height);

  auto delegate =
      std::make_shared<HarmonyRunDelegate>(contentHandle, width, height);
  native_paragraph->run_delegate_list_.push_back(delegate);
  native_paragraph->paragraph_->AddShapeRun(nullptr, delegate, false);
  return nullptr;
}
}  // namespace tttext
}  // namespace ttoffice
