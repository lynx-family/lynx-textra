// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "native_text_node.h"

#include <native_drawing/drawing_canvas.h>
#include <window_manager/oh_display_manager.h>

#include "fontmgr_collection.h"
#include "layout_drawer.h"
#include "napi_init.h"
#include "native_paragraph.h"
#include "platform_helper.h"
#include "src/textlayout/layout_position.h"

namespace ttoffice {
namespace tttext {
napi_value NAPI_Global_createNativeTextNode(napi_env env,
                                            napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  // Get NodeContent
  ArkUI_NodeContentHandle contentHandle = 0;
  auto status =
      OH_ArkUI_GetNodeContentFromNapiValue(env, args[0], &contentHandle);

  // Create text list
  auto* node = new NativeTextNode();
  status = napi_wrap(env, args[0], reinterpret_cast<void*>(node), NAPI_Deref,
                     reinterpret_cast<void*>(HintType::kTextNode), NULL);
  if (status != napi_ok) {
    // Manually release memory
    delete node;
  }

  node->SetPercentWidth(1);
  node->SetPercentHeight(1);
  node->AttachToNodeContent(contentHandle);

  return nullptr;
}
napi_value NAPI_Global_destroyNativeTextNode(napi_env env,
                                             napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  NativeTextNode* node = nullptr;
  auto status = napi_unwrap(env, argv[0], reinterpret_cast<void**>(&node));
  if (status != napi_ok) {
    return nullptr;
  }
  node->DetachFromNodeContent();
  return nullptr;
}
napi_value NAPI_Global_appendParagraphToTextNode(napi_env env,
                                                 napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2] = {nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  NativeTextNode* node = nullptr;
  auto status = napi_unwrap(env, argv[0], reinterpret_cast<void**>(&node));
  if (status != napi_ok) {
    return nullptr;
  }
  NativeParagraph* native_paragraph = nullptr;
  status = napi_remove_wrap(env, argv[1],
                            reinterpret_cast<void**>(&native_paragraph));
  if (status != napi_ok) {
    return nullptr;
  }
  node->AppendParagraph(native_paragraph);
  node->MarkDirty();
  delete native_paragraph;
  return nullptr;
}
NativeTextNode::NativeTextNode()
    : ArkUINode((NativeModuleInstance::GetInstance()->GetNativeNodeAPI())
                    ->createNode(ARKUI_NODE_CUSTOM)) {
  // Register custom event listener.
  nativeModule_->addNodeCustomEventReceiver(handle_, OnStaticCustomEvent);
  // Declare custom event and pass itself as custom data.
  nativeModule_->registerNodeCustomEvent(
      handle_, ARKUI_NODE_CUSTOM_EVENT_ON_MEASURE, 0, this);
  nativeModule_->registerNodeCustomEvent(
      handle_, ARKUI_NODE_CUSTOM_EVENT_ON_LAYOUT, 0, this);
  nativeModule_->registerNodeCustomEvent(
      handle_, ARKUI_NODE_CUSTOM_EVENT_ON_DRAW, 0, this);

  OH_NativeDisplayManager_GetDefaultDisplayScaledDensity(&density_);

  font_manager_ =
      tttext::PlatformHelper::CreateFontManager(tttext::PlatformType::kSystem);
  tttext::FontmgrCollection fc(font_manager_);
  layout_ =
      std::make_unique<tttext::TextLayout>(&fc, tttext::ShaperType::kSystem);
  context_ = std::make_unique<tttext::TTTextContext>();
}
NativeTextNode::~NativeTextNode() {
  // Unregister custom event listener.
  nativeModule_->removeNodeCustomEventReceiver(handle_, OnStaticCustomEvent);
  // Cancel custom event declaration.
  nativeModule_->unregisterNodeCustomEvent(handle_,
                                           ARKUI_NODE_CUSTOM_EVENT_ON_MEASURE);
  nativeModule_->unregisterNodeCustomEvent(handle_,
                                           ARKUI_NODE_CUSTOM_EVENT_ON_LAYOUT);
  nativeModule_->unregisterNodeCustomEvent(handle_,
                                           ARKUI_NODE_CUSTOM_EVENT_ON_DRAW);
}
void NativeTextNode::AttachToNodeContent(ArkUI_NodeContentHandle handle) {
  node_content_handle_ = handle;
  OH_ArkUI_NodeContent_AddNode(node_content_handle_, GetHandle());
}
void NativeTextNode::DetachFromNodeContent() {
  OH_ArkUI_NodeContent_RemoveNode(node_content_handle_, GetHandle());
}
void NativeTextNode::AppendParagraph(NativeParagraph* native_paragraph) {
  paragraphs_list_.push_back(std::move(native_paragraph->paragraph_));
  for (auto& delegate : native_paragraph->run_delegate_list_) {
    auto& arkui_node = delegate->ark_ui_node_;
    AddChild(arkui_node);
  }
  unmeasured_delegates_.insert(unmeasured_delegates_.end(),
                               native_paragraph->run_delegate_list_.begin(),
                               native_paragraph->run_delegate_list_.end());
}

void NativeTextNode::OnMeasure(ArkUI_NodeCustomEvent* event) {
  auto layoutConstrain =
      OH_ArkUI_NodeCustomEvent_GetLayoutConstraintInMeasure(event);

  auto max_width = OH_ArkUI_LayoutConstraint_GetMaxWidth(layoutConstrain);
  auto min_width = OH_ArkUI_LayoutConstraint_GetMinWidth(layoutConstrain);

  auto max_height = OH_ArkUI_LayoutConstraint_GetMaxHeight(layoutConstrain);
  auto min_height = OH_ArkUI_LayoutConstraint_GetMinHeight(layoutConstrain);

  auto childLayoutConstrain = OH_ArkUI_LayoutConstraint_Copy(layoutConstrain);
  OH_ArkUI_LayoutConstraint_SetMaxHeight(childLayoutConstrain, max_height);
  OH_ArkUI_LayoutConstraint_SetMaxWidth(childLayoutConstrain, max_width);
  OH_ArkUI_LayoutConstraint_SetMinHeight(childLayoutConstrain, 0);
  OH_ArkUI_LayoutConstraint_SetMinWidth(childLayoutConstrain, 0);

  for (auto& delegate : unmeasured_delegates_) {
    auto node_handle = delegate->ark_ui_node_->GetHandle();
    nativeModule_->measureNode(node_handle, childLayoutConstrain);
    auto size = nativeModule_->getMeasuredSize(node_handle);
    delegate->ascent_ = -size.height;
    delegate->descent_ = 0;
    delegate->width_ = size.width;
  }
  context_->Reset();
  region_ = std::make_unique<LayoutRegion>(
      max_width, max_height, LayoutMode::kDefinite, LayoutMode::kDefinite);

  for (auto& paragraph : paragraphs_list_) {
    layout_->Layout(paragraph.get(), region_.get(), *context_);
    context_->ResetLayoutPosition({0, 0});
  }

  nativeModule_->setMeasuredSize(handle_, max_width, max_height);

  if (document_view_ != nullptr) {
    document_view_->OnLayout(max_width, max_height, LayoutMode::kDefinite,
                             LayoutMode::kDefinite);
  }
}
void NativeTextNode::OnLayout(ArkUI_NodeCustomEvent* event) {
  // auto position = OH_ArkUI_NodeCustomEvent_GetPositionInLayout(event);
  // nativeModule_->setLayoutPosition(handle_, position.x, position.y);

  for (auto& delegate : unmeasured_delegates_) {
    auto node_handle = delegate->ark_ui_node_->GetHandle();
    auto result = nativeModule_->layoutNode(node_handle, delegate->GetXOffset(),
                                            delegate->GetYOffset());
    if (result != napi_ok) {
      TTASSERT(false);
    }
  }
  unmeasured_delegates_.clear();
}

void NativeTextNode::OnDraw(ArkUI_NodeCustomEvent* event) {
  auto drawContext = OH_ArkUI_NodeCustomEvent_GetDrawContextInDraw(event);
  // Get the graphics drawing object.
  auto drawCanvas = reinterpret_cast<OH_Drawing_Canvas*>(
      OH_ArkUI_DrawContext_GetCanvas(drawContext));
  // Get the component size.
  auto canvas =
      PlatformHelper::CreateCanvasHelper(PlatformType::kSystem, drawCanvas);
  if (document_view_ != nullptr) {
    document_view_->OnDraw(*canvas);
  }
  LayoutDrawer drawer(canvas.get());
  drawer.DrawLayoutPage(region_.get());
}
}  // namespace tttext
}  // namespace ttoffice
