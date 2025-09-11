// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_NAPI_INIT_H_
#define DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_NAPI_INIT_H_

#include <node_api.h>

static void NAPI_Deref(napi_env env, void* data, void* hint);
napi_value NAPI_Global_initLayoutContext(napi_env env, napi_callback_info info);
napi_value NAPI_Global_buildCanvas(napi_env env, napi_callback_info info);
napi_value NAPI_Global_buildParagraph(napi_env env, napi_callback_info info);
napi_value NAPI_Global_layoutParagraph(napi_env env, napi_callback_info info);
napi_value NAPI_Global_drawParagraph(napi_env env, napi_callback_info info);
#endif  // DEMOS_HARMONY_PERFTEST_ENTRY_SRC_MAIN_CPP_NAPI_INIT_H_
