// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <textra/c/textra_buffer_output_stream.h>
#include <textra/c/textra_platform_helper.h>
#include <textra/platform/java/buffer_output_stream.h>

#if defined(ENABLE_JAVASHAPER)
#include <textra/platform/java/java_canvas_helper.h>
#endif

#include "src/capi/textra_capi_internal.h"

namespace {
using tttext::c_api_internal::Handle;
using tttext::c_api_internal::Raw;
}  // namespace

textra_status_t TEXTRA_CALL
textra_canvas_helper_create_java(textra_canvas_helper_t** out) noexcept {
#if defined(ENABLE_JAVASHAPER)
  *out = Handle<textra_canvas_helper_t>(
      static_cast<tttext::ICanvasHelper*>(new tttext::JavaCanvasHelper()));
  return TEXTRA_STATUS_OK;
#else
  *out = nullptr;
  return TEXTRA_STATUS_UNSUPPORTED;
#endif
}

textra_buffer_output_stream_t* TEXTRA_CALL
textra_canvas_helper_get_java_buffer(textra_canvas_helper_t* canvas) noexcept {
#if defined(ENABLE_JAVASHAPER)
  return Handle<textra_buffer_output_stream_t>(
      &static_cast<tttext::JavaCanvasHelper*>(
           Raw<tttext::ICanvasHelper>(canvas))
           ->GetBuffer());
#else
  return nullptr;
#endif
}

uint32_t TEXTRA_CALL textra_buffer_output_stream_get_size(
    const textra_buffer_output_stream_t* stream) noexcept {
  return Raw<BufferOutputStream>(stream)->GetSize();
}
const uint8_t* TEXTRA_CALL textra_buffer_output_stream_get_buffer(
    const textra_buffer_output_stream_t* stream) noexcept {
  return const_cast<BufferOutputStream*>(Raw<BufferOutputStream>(stream))
      ->GetBuffer();
}
void TEXTRA_CALL textra_buffer_output_stream_clear(
    textra_buffer_output_stream_t* stream) noexcept {
  Raw<BufferOutputStream>(stream)->Clear();
}
void TEXTRA_CALL textra_buffer_output_stream_write_int8(
    textra_buffer_output_stream_t* stream, int8_t value) noexcept {
  Raw<BufferOutputStream>(stream)->WriteInt8(value);
}
void TEXTRA_CALL textra_buffer_output_stream_write_int32(
    textra_buffer_output_stream_t* stream, int32_t value) noexcept {
  Raw<BufferOutputStream>(stream)->WriteInt32(value);
}
void TEXTRA_CALL textra_buffer_output_stream_write_int64(
    textra_buffer_output_stream_t* stream, int64_t value) noexcept {
  Raw<BufferOutputStream>(stream)->WriteInt64(value);
}
void TEXTRA_CALL textra_buffer_output_stream_write_float(
    textra_buffer_output_stream_t* stream, float value) noexcept {
  Raw<BufferOutputStream>(stream)->WriteFloat(value);
}
void TEXTRA_CALL textra_buffer_output_stream_write_bool(
    textra_buffer_output_stream_t* stream, uint32_t value) noexcept {
  Raw<BufferOutputStream>(stream)->WriteBool(value != 0);
}
void TEXTRA_CALL textra_buffer_output_stream_write_string(
    textra_buffer_output_stream_t* stream, const char* value,
    uint32_t length) noexcept {
  Raw<BufferOutputStream>(stream)->WriteString(value, length);
}
void TEXTRA_CALL textra_buffer_output_stream_write_buffer(
    textra_buffer_output_stream_t* stream, const uint8_t* value,
    uint32_t length) noexcept {
  Raw<BufferOutputStream>(stream)->WriteBuffer(const_cast<uint8_t*>(value),
                                               length);
}
