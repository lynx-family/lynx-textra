// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_PARAGRAPH_H_
#define PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_PARAGRAPH_H_

#include <list>

#include "harmony_run_delegate.h"
#include "paragraph.h"

namespace ttoffice {
namespace tttext {
class NativeParagraph {
 public:
  NativeParagraph() : paragraph_(Paragraph::Create()) {}

  std::unique_ptr<Paragraph> paragraph_;
  std::list<std::shared_ptr<HarmonyRunDelegate>> run_delegate_list_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_PARAGRAPH_H_
