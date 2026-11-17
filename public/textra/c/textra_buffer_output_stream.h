// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_C_TEXTRA_BUFFER_OUTPUT_STREAM_H_
#define PUBLIC_TEXTRA_C_TEXTRA_BUFFER_OUTPUT_STREAM_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

/* Borrowed from JavaCanvasHelper; never destroy this stream. Writes use the
 * existing BufferOutputStream representation and copy their input data.
 * get_buffer returns a borrowed byte view, invalidated by further writes or
 * clear. String lengths count UTF-8 bytes; WriteString prefixes the length
 * and does not add a terminating NUL. This is the existing Java command stream,
 * not a versioned serialization format for persistent storage. */
TEXTRA_API uint32_t TEXTRA_CALL textra_buffer_output_stream_get_size(
    const textra_buffer_output_stream_t* stream) TEXTRA_NOEXCEPT;
TEXTRA_API const uint8_t* TEXTRA_CALL textra_buffer_output_stream_get_buffer(
    const textra_buffer_output_stream_t* stream) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_buffer_output_stream_clear(
    textra_buffer_output_stream_t* stream) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_buffer_output_stream_write_int8(
    textra_buffer_output_stream_t* stream, int8_t value) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_buffer_output_stream_write_int32(
    textra_buffer_output_stream_t* stream, int32_t value) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_buffer_output_stream_write_int64(
    textra_buffer_output_stream_t* stream, int64_t value) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_buffer_output_stream_write_float(
    textra_buffer_output_stream_t* stream, float value) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_buffer_output_stream_write_bool(
    textra_buffer_output_stream_t* stream, uint32_t value) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_buffer_output_stream_write_string(
    textra_buffer_output_stream_t* stream, const char* value,
    uint32_t length) TEXTRA_NOEXCEPT;
TEXTRA_API void TEXTRA_CALL textra_buffer_output_stream_write_buffer(
    textra_buffer_output_stream_t* stream, const uint8_t* value,
    uint32_t length) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END
#endif  // PUBLIC_TEXTRA_C_TEXTRA_BUFFER_OUTPUT_STREAM_H_
