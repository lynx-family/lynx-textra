// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NAPI_INIT_H_
#define PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NAPI_INIT_H_

#include <node_api.h>
namespace ttoffice {
namespace tttext {
enum class HintType : uint8_t { kTextNode = 1, kParagraph, kTextStorage };

static void NAPI_Deref(napi_env env, void* data, void* hint) {
  auto type = static_cast<HintType*>(hint);
}
napi_value NAPI_Global_createNativeTextNode(napi_env env,
                                            napi_callback_info info);

napi_value NAPI_Global_destroyNativeTextNode(napi_env env,
                                             napi_callback_info info);
napi_value NAPI_Global_bindNativeParagraph(napi_env env,
                                           napi_callback_info info);
napi_value NAPI_Global_setParagraphStyle(napi_env env, napi_callback_info info);
napi_value NAPI_Global_addTextRun(napi_env env, napi_callback_info info);
napi_value NAPI_Global_addFrameNode(napi_env env, napi_callback_info info);
napi_value NAPI_Global_appendParagraphToTextNode(napi_env env,
                                                 napi_callback_info info);
}  // namespace tttext
}  // namespace ttoffice

#endif  // PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NAPI_INIT_H_
