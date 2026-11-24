// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <napi/native_api.h>
#include <napi/native_node_api.h>

#include <string>

#include "test/perf/shaping_benchmark.h"

namespace {
napi_value Run(napi_env env, napi_callback_info info) {
  size_t count = 2;
  napi_value args[2];
  if (napi_get_cb_info(env, info, &count, args, nullptr, nullptr) != napi_ok ||
      count != 2)
    return nullptr;
  std::string inputs[2];
  for (size_t i = 0; i < 2; ++i) {
    size_t length = 0;
    if (napi_get_value_string_utf8(env, args[i], nullptr, 0, &length) !=
        napi_ok)
      return nullptr;
    inputs[i].resize(length + 1);
    if (napi_get_value_string_utf8(env, args[i], inputs[i].data(), length + 1,
                                   &length) != napi_ok)
      return nullptr;
    inputs[i].resize(length);
  }
  std::string response;
  if (RunShapingBenchmark(inputs[0], inputs[1], &response)) {
    napi_throw_error(env, nullptr, "Textra benchmark failed");
    return nullptr;
  }
  napi_value result;
  return napi_create_string_utf8(env, response.c_str(), response.size(),
                                 &result) == napi_ok
             ? result
             : nullptr;
}
napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor method = {"run",   nullptr, Run,          nullptr,
                                     nullptr, nullptr, napi_default, nullptr};
  return napi_define_properties(env, exports, 1, &method) == napi_ok ? exports
                                                                     : nullptr;
}
napi_module module = {1, 0, nullptr, Init, "textra_perf", nullptr, {nullptr}};
__attribute__((constructor)) void Register() { napi_module_register(&module); }
}  // namespace
