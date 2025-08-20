// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_JAVA_BUFFER_OUTPUT_STREAM_H_
#define PUBLIC_TEXTRA_PLATFORM_JAVA_BUFFER_OUTPUT_STREAM_H_

#include <cstdint>
#include <cstdlib>
#include <memory>

class BufferOutputStream {
 public:
  BufferOutputStream() : buffer_(nullptr), capacity_(0), size_(0) {
    EnlargeCapacity();
  }
  explicit BufferOutputStream(size_t capacity)
      : buffer_(reinterpret_cast<uint8_t*>(malloc(capacity))),
        capacity_(capacity),
        size_(0) {}

  ~BufferOutputStream() {
    if (buffer_ != nullptr) free(buffer_);
    buffer_ = nullptr;
  }

 public:
  void WriteString(const char* str, uint32_t len) {
    Write(len);
    WriteInternal(reinterpret_cast<uint8_t*>(const_cast<char*>(str)), len);
  }

  void WriteBuffer(uint8_t* buffer, uint32_t len) {
    WriteInternal(buffer, len);
  }

  template <typename T>
  void Write(T data) {
    WriteInternal(reinterpret_cast<uint8_t*>(&data), sizeof(T), true);
  }

  void WriteInt8(int8_t data) { Write((int8_t)data); }

  void WriteInt32(int32_t data) { Write((int32_t)data); }

  void WriteInt64(int64_t data) { Write((int64_t)data); }

  void WriteFloat(float data) { Write(static_cast<float>(data)); }

  void WriteBool(bool data) { Write(static_cast<bool>(data)); }

 public:
  uint32_t GetSize() const { return size_; }

  uint8_t* GetBuffer() { return buffer_; }
  void Clear() { size_ = 0; }

 private:
  void WriteInternal(uint8_t* buff, int len, bool little_endian = true) {
    while (size_ + len > capacity_) {
      if (!EnlargeCapacity()) return;
    }
    if (little_endian || len == 1) {
      memcpy(buffer_ + size_, buff, len);
    } else {
      for (int k = 0; k < len; k++) {
        buffer_[size_ + k] = buff[len - k - 1];
      }
    }
    size_ += len;
  }

  bool EnlargeCapacity() {
    capacity_ = capacity_ == 0 ? 1024 : 2 * capacity_;
    auto* new_buff = reinterpret_cast<uint8_t*>(malloc(capacity_));
    if (new_buff == nullptr) return false;
    if (buffer_ != nullptr) {
      memcpy(new_buff, buffer_, size_);
      free(buffer_);
    }
    buffer_ = new_buff;
    return true;
  }

 private:
  uint8_t* buffer_;
  uint32_t capacity_;
  uint32_t size_;
};

#endif  // PUBLIC_TEXTRA_PLATFORM_JAVA_BUFFER_OUTPUT_STREAM_H_
