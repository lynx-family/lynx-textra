// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "buffer_input_stream.h"

#include <cassert>
#include <cmath>
#include <memory>
#include <utility>

BufferInputStream::BufferInputStream(const uint8_t* buff, size_t len,
                                     bool little)
    : buffer_(buff), current_(buff), buffer_len_(len), little_endian_(little) {}

BufferInputStream::~BufferInputStream() = default;

float BufferInputStream::ReadFloat() {
  float ret = NAN;
  if (little_endian_) {
    memcpy(&ret, current_, sizeof(ret));
    current_ += sizeof(ret);
  } else {
    auto* bytes = (uint8_t*)&ret;
    for (int i = sizeof(ret) - 1; i >= 0; i--) {
      bytes[i] = *current_;
      current_++;
    }
  }
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}

bool BufferInputStream::ReadBool() {
  bool ret = 0;
  memcpy(&ret, current_, sizeof(ret));
  current_ += sizeof(ret);
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}

const char* BufferInputStream::ReadString() {
  int32_t len = ReadInt();
  if (len == 0) return nullptr;
  auto* ret = new char[len + 1];
  memcpy(ret, current_, len);
  ret[len] = 0;
  current_ += len;
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}

int32_t BufferInputStream::ReadInt32() {
  int32_t ret = 0;
  if (little_endian_) {
    memcpy(&ret, current_, sizeof(ret));
    current_ += sizeof(ret);
  } else {
    auto* bytes = (uint8_t*)&ret;
    for (int i = sizeof(ret) - 1; i >= 0; i--) {
      bytes[i] = *current_;
      current_++;
    }
  }
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}

int64_t BufferInputStream::ReadInt64() {
  int64_t ret = 0;
  if (little_endian_) {
    memcpy(&ret, current_, sizeof(ret));
    current_ += sizeof(ret);
  } else {
    auto* bytes = (uint8_t*)&ret;
    for (int i = sizeof(ret) - 1; i >= 0; i--) {
      bytes[i] = *current_;
      current_++;
    }
  }
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}

int8_t BufferInputStream::ReadInt8() {
  int8_t ret = 0;
  memcpy(&ret, current_, sizeof(ret));
  current_ += sizeof(ret);
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}

double BufferInputStream::ReadDouble() {
  double ret = NAN;
  if (little_endian_) {
    memcpy(&ret, current_, sizeof(ret));
    current_ += sizeof(ret);
  } else {
    auto* bytes = (uint8_t*)&ret;
    for (int i = sizeof(ret) - 1; i >= 0; i--) {
      bytes[i] = *current_;
      current_++;
    }
  }
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}

template <typename T>
T BufferInputStream::ReadType() {
  T ret;
  memcpy(&ret, current_, sizeof(ret));
  current_ += sizeof(ret);
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}

std::string BufferInputStream::ReadStdString() {
  int32_t len = ReadInt();
  if (len == 0) return {};
  std::string ret((char*)current_, len);
  current_ += len;
  return ret;
}

std::unique_ptr<char[]> BufferInputStream::CopyBuffer(int len) {
  auto ret = std::make_unique<char[]>(len);
  memcpy(ret.get(), current_, len);
  current_ += len;
  assert(current_ - buffer_ <= buffer_len_);
  return ret;
}
