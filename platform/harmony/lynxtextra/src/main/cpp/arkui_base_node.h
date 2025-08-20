// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_ARKUI_BASE_NODE_H_
#define PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_ARKUI_BASE_NODE_H_

#include <list>
#include <memory>

#include "native_module.h"

namespace NativeModule {

class ArkUIBaseNode {
 public:
  explicit ArkUIBaseNode(ArkUI_NodeHandle handle)
      : handle_(handle),
        nativeModule_(NativeModuleInstance::GetInstance()->GetNativeNodeAPI()) {
  }

  virtual ~ArkUIBaseNode() {
    // Wrap destructor, implement child node removal function.
    if (!children_.empty()) {
      for (const auto& child : children_) {
        nativeModule_->removeChild(handle_, child->GetHandle());
      }
      children_.clear();
    }
    // Wrap destructor, unify node resource recovery.
    nativeModule_->disposeNode(handle_);
  }

  void AddChild(const std::shared_ptr<ArkUIBaseNode>& child) {
    children_.emplace_back(child);
    OnAddChild(child);
  }

  void RemoveChild(const std::shared_ptr<ArkUIBaseNode>& child) {
    children_.remove(child);
    OnRemoveChild(child);
  }

  void InsertChild(const std::shared_ptr<ArkUIBaseNode>& child, int32_t index) {
    if (index >= children_.size()) {
      AddChild(child);
    } else {
      auto iter = children_.begin();
      std::advance(iter, index);
      children_.insert(iter, child);
      OnInsertChild(child, index);
    }
  }

  ArkUI_NodeHandle GetHandle() const { return handle_; }

 protected:
  // For parent container subclasses, overload the following functions to
  // implement component mounting and unmounting.
  virtual void OnAddChild(const std::shared_ptr<ArkUIBaseNode>& child) {}
  virtual void OnRemoveChild(const std::shared_ptr<ArkUIBaseNode>& child) {}
  virtual void OnInsertChild(const std::shared_ptr<ArkUIBaseNode>& child,
                             int32_t index) {}

  ArkUI_NodeHandle handle_;
  ArkUI_NativeNodeAPI_1* nativeModule_ = nullptr;

 protected:
  std::list<std::shared_ptr<ArkUIBaseNode>> children_;
};
}  // namespace NativeModule

#endif  // PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_ARKUI_BASE_NODE_H_
