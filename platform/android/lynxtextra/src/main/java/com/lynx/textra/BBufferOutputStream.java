// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class BBufferOutputStream {
  private final ByteBuffer byteBuffer;

  public BBufferOutputStream(int size) {
    byteBuffer = ByteBuffer.allocate(size);
    byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
  }

  public BBufferOutputStream writeInt(int value) {
    byteBuffer.putInt(value);
    return this;
  }
  public BBufferOutputStream writeLong(long value) {
    byteBuffer.putLong(value);
    return this;
  }

  public BBufferOutputStream writeShort(short value) {
    byteBuffer.putShort(value);
    return this;
  }

  public BBufferOutputStream writeFloat(float value) {
    byteBuffer.putFloat(value);
    return this;
  }

  public BBufferOutputStream writeByte(byte value) {
    byteBuffer.put(value);
    return this;
  }

  public byte[] getByteArray() {
    return byteBuffer.array();
  }
}
