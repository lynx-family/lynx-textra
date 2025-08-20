// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_HARMONY_RUN_DELEGATE_H_
#define PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_HARMONY_RUN_DELEGATE_H_

#include "arkui_node.h"
#include "run_delegate.h"

namespace ttoffice {
namespace tttext {
class HarmonyRunDelegate : public RunDelegate {
 public:
  explicit HarmonyRunDelegate(ArkUI_NodeHandle handle, double width,
                              double height)
      : ark_ui_node_(std::make_shared<NativeModule::ArkUINode>(handle)),
        ascent_(-height),
        width_(width) {}
  ~HarmonyRunDelegate() override {}
  float GetAscent() const override { return ascent_; }
  float GetDescent() const override { return 0; }
  float GetAdvance() const override { return width_; }
  void Layout() override {}
  void Draw(ICanvasHelper* canvas, float x, float y) override {}

 public:
  std::shared_ptr<NativeModule::ArkUINode> ark_ui_node_;
  float ascent_ = 0;
  float descent_ = 0;
  float width_ = 0;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_HARMONY_RUN_DELEGATE_H_
