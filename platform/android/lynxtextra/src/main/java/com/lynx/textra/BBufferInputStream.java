// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;

public class BBufferInputStream {
  private final ByteBuffer byteBuffer;
  private final byte[] buffer;

  public BBufferInputStream(byte[] buf) {
    byteBuffer = ByteBuffer.wrap(buf);
    byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
    buffer = buf;
  }

  public int available() {
    return byteBuffer.limit() - byteBuffer.position();
  }

  public int readInt() {
    return byteBuffer.getInt();
  }

  public short readShort() {
    return byteBuffer.getShort();
  }

  public float readFloat() {
    return byteBuffer.getFloat();
  }

  public byte readByte() {
    return byteBuffer.get();
  }

  public long readLong() {
    return byteBuffer.getLong();
  }

  public void read(byte[] dst, int offset, int len) {
    byteBuffer.get(dst, offset, len);
  }

  public char ReadChar() {
    return byteBuffer.getChar();
  }

  public String ReadCString() throws IOException {
    int tag_l = readInt();
    if (tag_l == 0)
      return "";
    int pos = byteBuffer.position();
    byteBuffer.position(pos + tag_l);
    return new String(buffer, pos, tag_l, Charset.defaultCharset());
  }

  public int ReadVarInt32() throws IOException {
    int b = byteBuffer.get() & 0xFF;
    int ret = b & 0x7F;
    int shift = 7;
    while (b > 0x7F) {
      b = byteBuffer.get() & 0xFF;
      ret |= (b & 0x7F) << shift;
      shift += 7;
    }
    return ret;
  }
}
