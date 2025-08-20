// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;

public class CBufferInputStream extends DataInputStream {
  /**
   * Creates a DataInputStream that uses the specified
   * underlying InputStream.
   *
   * @param in the specified input stream
   */
  public CBufferInputStream(InputStream in) {
    super(in);
  }

  public String ReadCString() throws IOException {
    int tag_l = readInt();
    if (tag_l == 0)
      return "";
    byte[] tag_b = new byte[tag_l];
    if (read(tag_b, 0, tag_l) != -1) {
      return new String(tag_b, Charset.defaultCharset());
    }
    return "";
  }
}
