// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_ANDROID_APP_SRC_MAIN_JNI_BUFFER_INPUT_STREAM_H_
#define DEMOS_ANDROID_APP_SRC_MAIN_JNI_BUFFER_INPUT_STREAM_H_

#include <textra/macro.h>

#include <cstdint>
#include <memory>
#include <string>
class BufferInputStream {
 public:
  L_EXPORT BufferInputStream(const uint8_t* buff, size_t len,
                             bool little_endian = false);
  L_EXPORT ~BufferInputStream();

 public:
  int32_t L_EXPORT ReadInt() { return ReadInt32(); }
  int8_t ReadInt8();
  L_EXPORT int32_t ReadInt32();
  int64_t ReadInt64();
  L_EXPORT float ReadFloat();
  double ReadDouble();
  L_EXPORT bool ReadBool();
  const char* ReadString();
  L_EXPORT std::string ReadStdString();
  bool L_EXPORT Empty() {
    return ((size_t)(current_ - buffer_) >= buffer_len_);
  }
  void SetEndian(bool is_little) { little_endian_ = is_little; }
  bool IsLittleEndian() const { return little_endian_; }

 public:
  template <typename T>
  T ReadType();
  std::unique_ptr<char[]> CopyBuffer(int len);

 private:
  const uint8_t* buffer_;
  const uint8_t* current_;
  size_t buffer_len_;
  bool little_endian_ = false;
};

#endif  // DEMOS_ANDROID_APP_SRC_MAIN_JNI_BUFFER_INPUT_STREAM_H_
