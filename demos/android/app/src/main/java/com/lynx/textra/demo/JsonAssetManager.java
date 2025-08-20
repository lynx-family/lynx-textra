// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import android.content.Context;
import android.content.res.AssetManager;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Responsible for reading JSON files from assets/json_data directory
 */
public class JsonAssetManager {
  private static final String JSON_DATA_PATH = "json_data";

  private final AssetManager assetManager;
  private final List<String> files = new ArrayList<>();

  public JsonAssetManager(Context context) {
    this.assetManager = context.getAssets();
    try {
      for (String file : assetManager.list(JSON_DATA_PATH)) {
        if (file.endsWith(".json")) {
          files.add(file);
        }
      }
    } catch (IOException e) {
      android.util.Log.w("JsonAssetManager", "Failed to read JSON files: " + e.getMessage());
    }
  }

  /**
   * Get all available JSON file names
   */
  public List<String> getFiles() {
    return Collections.unmodifiableList(files);
  }

  /**
   * Read the specified JSON file (returns byte array)
   *
   * @param fileName file name
   * @return byte array of file content
   * @throws IOException if reading file fails
   */
  public byte[] readFile(String fileName) throws IOException {
    try (InputStream inputStream = assetManager.open(JSON_DATA_PATH + "/" + fileName);
         ByteArrayOutputStream os = new ByteArrayOutputStream()) {
      byte[] buffer = new byte[1024];
      int nRead;
      while ((nRead = inputStream.read(buffer, 0, buffer.length)) != -1) {
        os.write(buffer, 0, nRead);
      }
      return os.toByteArray();
    }
  }
}
