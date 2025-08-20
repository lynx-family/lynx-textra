// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_ARKUI_NODE_H_
#define PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_ARKUI_NODE_H_

#include <arkui/native_node.h>
#include <arkui/native_type.h>

#include <cassert>
#include <cstdint>

#include "arkui_base_node.h"
#include "macro.h"

namespace NativeModule {
class ArkUINode : public ArkUIBaseNode {
 public:
  explicit ArkUINode(ArkUI_NodeHandle handle) : ArkUIBaseNode(handle) {}

  ~ArkUINode() override {}

  // NDK related common attribute call encapsulation
  void SetWidth(float width) {
    assert(handle_);
    ArkUI_NumberValue value[] = {{.f32 = width}};
    ArkUI_AttributeItem item = {value, 1};
    nativeModule_->setAttribute(handle_, NODE_WIDTH, &item);
  }

  void SetPercentWidth(float percent) {
    assert(handle_);
    ArkUI_NumberValue value[] = {{.f32 = percent}};
    ArkUI_AttributeItem item = {value, 1};
    nativeModule_->setAttribute(handle_, NODE_WIDTH_PERCENT, &item);
  }

  void SetHeight(float height) {
    assert(handle_);
    ArkUI_NumberValue value[] = {{.f32 = height}};
    ArkUI_AttributeItem item = {value, 1};
    nativeModule_->setAttribute(handle_, NODE_HEIGHT, &item);
  }

  void SetPercentHeight(float percent) {
    assert(handle_);
    ArkUI_NumberValue value[] = {{.f32 = percent}};
    ArkUI_AttributeItem item = {value, 1};
    nativeModule_->setAttribute(handle_, NODE_HEIGHT_PERCENT, &item);
  }

  void SetBackgroundColor(uint32_t color) {
    assert(handle_);
    ArkUI_NumberValue value[] = {{.u32 = color}};
    ArkUI_AttributeItem item = {value, 1};
    nativeModule_->setAttribute(handle_, NODE_BACKGROUND_COLOR, &item);
  }

  void MarkDirty() {
    assert(handle_);
    nativeModule_->markDirty(handle_, NODE_NEED_MEASURE);
  }

 protected:
  // Component tree implementation
  void OnAddChild(const std::shared_ptr<ArkUIBaseNode>& child) override {
    auto result = nativeModule_->addChild(handle_, child->GetHandle());
    if (result != napi_ok) {
      TTASSERT(false);
    }
  }

  void OnRemoveChild(const std::shared_ptr<ArkUIBaseNode>& child) override {
    auto result = nativeModule_->removeChild(handle_, child->GetHandle());
    if (result != napi_ok) {
      TTASSERT(false);
    }
  }

  void OnInsertChild(const std::shared_ptr<ArkUIBaseNode>& child,
                     int32_t index) override {
    auto result =
        nativeModule_->insertChildAt(handle_, child->GetHandle(), index);
    if (result != napi_ok) {
      TTASSERT(false);
    }
  }
};
}  // namespace NativeModule

#endif  // PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_ARKUI_NODE_H_
