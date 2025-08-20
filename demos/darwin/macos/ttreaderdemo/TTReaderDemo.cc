// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "demos/darwin/macos/ttreaderdemo/TTReaderDemo.h"

#include <textra/macro.h>
#include <textra/platform/skia/skia_canvas_helper.h>
#include <textra/platform/skia/skia_font_manager.h>

#include <memory>

#include "demos/darwin/macos/skia_app_demo/SkiaAdaptor.h"
#include "demos/darwin/macos/ttreaderdemo/paragraph_test.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/encode/SkEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "modules/skparagraph/include/TTText.h"

int MainWork(const char* path, const char* password, const char* output,
             int width, int height, uint32_t test_id, bool count = false) {
  SkPictureRecorder recoder;
  auto recorder_canvas = recoder.beginRecording(width, height);
  SkiaCanvasHelper skia_canvas_helper(recorder_canvas);

  FontmgrCollection font_collection_(
      std::make_shared<SkiaFontManager>(skia_font_mgr));
  ParagraphTest paragraph_test(kSelfRendering, &font_collection_);
  paragraph_test.TestWithId(&skia_canvas_helper, width, test_id);
  auto skpicture = recoder.finishRecordingAsPicture();
  SkBitmap bitmap;
  SkImageInfo imageInfo =
      SkImageInfo::Make(width, height, SkColorType::kRGBA_8888_SkColorType,
                        kUnpremul_SkAlphaType);
  if (!bitmap.tryAllocPixels(imageInfo)) return -1;
  SkCanvas canvas(bitmap);
  canvas.drawPicture(skpicture);
  SkFILEWStream dst("layout.png");
  auto ret = SkPngEncoder::Encode(&dst, bitmap.pixmap(), {});
  return ret;
}
void printUsage() {
  printf(
      "usage: TTReaderDemo <file> [options]\n"
      "\t-p <password>\t\t\t\tpassword of the file\n"
      "\t-o <dir>\t\t\t\t\tspecify the EXISTING dump folder\n"
      "\t-w <width>\t\t\t\t\twidth of output images\n"
      "\t-h <height>\t\t\t\t\theight of output images\n"
      "\t-c\t\t\t\t\t\tshow count of pages\n");
}
int main(int argc, char* argv[]) {
  /* Default values */
  const char* output = ".";
  const char* pwd = "";
  int width = 1920;
  int height = 1200;
  uint32_t test_id = 0;
  bool count = false;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-o") == 0) {
      /* *
       * The output path requires to be resolved already,
       * i.e. no relative ".."
       * */
      output = argv[++i];
    } else if (strcmp(argv[i], "-p") == 0) {
      pwd = argv[++i];
    } else if (strcmp(argv[i], "-w") == 0) {
      width = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-h") == 0) {
      height = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-c") == 0) {
      count = true;
    } else if (strcmp(argv[i], "-t") == 0) {
      test_id = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--help") == 0) {
      printUsage();
      return 0;
    } else {
      printf("Unknown parameter: %s\n", argv[i]);
      return -1;
    }
  }
  int c = MainWork(argv[1], pwd, output, width, height, test_id, count);
  return count ? c : 0;
}
