// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_JS_VALUE_HELPER_H_
#define PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_JS_VALUE_HELPER_H_

#include <string>

#include "napi/native_api.h"
namespace ttoffice {
namespace tttext {
bool JsValueHelperGetUint32Property(napi_env env, napi_value js_obj,
                                    const char* prop_name, uint32_t* value) {
  napi_value js_value = nullptr;
  napi_get_named_property(env, js_obj, prop_name, &js_value);
  auto status = napi_get_value_uint32(env, js_value, value);
  return status == napi_ok;
}
bool JSValueHelperGetDoubleProperty(napi_env env, napi_value js_obj,
                                    const char* prop_name, double* value) {
  napi_value js_value;
  napi_get_named_property(env, js_obj, prop_name, &js_value);
  auto status = napi_get_value_double(env, js_value, value);
  return status == napi_ok;
}
bool JsValueHelperGetStringValue(napi_env env, napi_value js_obj,
                                 std::string& str) {
  size_t str_len = 0;
  auto status = napi_get_value_string_utf8(env, js_obj, nullptr, 0,
                                           &str_len);  // Get string length
  if (status != napi_ok) {
    return false;
  }
  str.resize(str_len + 1);
  status = napi_get_value_string_utf8(env, js_obj, str.data(), str_len + 1,
                                      &str_len);
  return status == napi_ok;
}

bool JsValueHelperGetStringArrayValue(napi_env env, napi_value js_obj,
                                      std::vector<std::string>& str_array) {
  bool is_array = false;
  napi_is_array(env, js_obj, &is_array);
  if (!is_array) {
    return false;
  }

  uint32_t array_len;
  auto status = napi_get_array_length(env, js_obj, &array_len);
  if (status != napi_ok) {
    return false;
  }
  str_array.resize(array_len);
  for (auto k = 0; k < array_len; ++k) {
    napi_value element;
    status = napi_get_element(env, js_obj, k, &element);
    if (status != napi_ok) {
      return false;
    }
    JsValueHelperGetStringValue(env, js_obj, str_array[k]);
  }
  return true;
}
}  // namespace tttext
}  // namespace ttoffice
#endif  // PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_JS_VALUE_HELPER_H_
