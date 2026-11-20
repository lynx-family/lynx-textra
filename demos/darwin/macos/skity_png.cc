// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "demos/darwin/macos/skity_png.h"

#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>

#include <cstring>
#include <skity/graphic/bitmap.hpp>

namespace textra_demo {
bool WriteSkityPng(const skity::Bitmap& bitmap, const char* path) {
  auto space = CGColorSpaceCreateDeviceRGB();
  auto context = CGBitmapContextCreate(
      bitmap.GetPixelAddr(), bitmap.Width(), bitmap.Height(), 8,
      bitmap.RowBytes(), space,
      kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(space);
  if (!context) return false;
  auto image = CGBitmapContextCreateImage(context);
  CGContextRelease(context);
  if (!image) return false;
  auto url = CFURLCreateFromFileSystemRepresentation(
      nullptr, reinterpret_cast<const UInt8*>(path), std::strlen(path), false);
  auto destination = url ? CGImageDestinationCreateWithURL(
                               url, CFSTR("public.png"), 1, nullptr)
                         : nullptr;
  bool result = false;
  if (destination) {
    CGImageDestinationAddImage(destination, image, nullptr);
    result = CGImageDestinationFinalize(destination);
    CFRelease(destination);
  }
  if (url) CFRelease(url);
  CGImageRelease(image);
  return result;
}
}  // namespace textra_demo
