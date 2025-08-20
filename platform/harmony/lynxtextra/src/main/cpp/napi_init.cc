// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "napi_init.h"

#include <napi/native_api.h>

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
      {"createNativeTextNode", nullptr,
       ttoffice::tttext::NAPI_Global_createNativeTextNode, nullptr, nullptr,
       nullptr, napi_default, nullptr},
      {"destroyNativeTextNode", nullptr,
       ttoffice::tttext::NAPI_Global_destroyNativeTextNode, nullptr, nullptr,
       nullptr, napi_default, nullptr},
      {"bindNativeParagraph", nullptr,
       ttoffice::tttext::NAPI_Global_bindNativeParagraph, nullptr, nullptr,
       nullptr, napi_default, nullptr},
      {"setParagraphStyle", nullptr,
       ttoffice::tttext::NAPI_Global_setParagraphStyle, nullptr, nullptr,
       nullptr, napi_default, nullptr},
      {"addTextRun", nullptr, ttoffice::tttext::NAPI_Global_addTextRun, nullptr,
       nullptr, nullptr, napi_default, nullptr},
      {"addFrameNode", nullptr, ttoffice::tttext::NAPI_Global_addFrameNode,
       nullptr, nullptr, nullptr, napi_default, nullptr},
      {"appendParagraphToTextNode", nullptr,
       ttoffice::tttext::NAPI_Global_appendParagraphToTextNode, nullptr,
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
    .nm_modname = "lynxtextra",
    .nm_priv = ((void*)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNativeTextModule(void) {
  napi_module_register(&demoModule);
}
