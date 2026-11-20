// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_DARWIN_MACOS_SKITY_PNG_H_
#define DEMOS_DARWIN_MACOS_SKITY_PNG_H_

namespace skity {
class Bitmap;
}

namespace textra_demo {
// Encode a premultiplied RGBA test/demo bitmap using the system ImageIO codec.
bool WriteSkityPng(const skity::Bitmap& bitmap, const char* path);
}  // namespace textra_demo

#endif  // DEMOS_DARWIN_MACOS_SKITY_PNG_H_
