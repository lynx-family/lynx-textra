// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_TEXT_NODE_H_
#define PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_TEXT_NODE_H_

#include <list>

#include "arkui_node.h"
#include "i_document_view.h"
#include "i_font_manager.h"
#include "macro.h"
#include "native_paragraph.h"
#include "paragraph.h"
#include "text_layout.h"

using namespace NativeModule;
namespace ttoffice {
namespace tttext {

class L_EXPORT NativeTextNode : public ArkUINode {
 public:
  // Use custom component type ARKUI_NODE_CUSTOM to create a component.
  NativeTextNode();

  ~NativeTextNode() override;

  void SetDocumentView(std::unique_ptr<IDocumentView>& view) {
    document_view_ = std::move(view);
  }
  IDocumentView* GetDocumentView() const { return document_view_.get(); }
  float GetDensity() const { return density_; }
  void AttachToNodeContent(ArkUI_NodeContentHandle handle);
  void DetachFromNodeContent();
  void AppendParagraph(NativeParagraph* paragraph);

 private:
  static void OnStaticCustomEvent(ArkUI_NodeCustomEvent* event) {
    // Get the component instance object and call the related instance method.
    auto customNode = reinterpret_cast<NativeTextNode*>(
        OH_ArkUI_NodeCustomEvent_GetUserData(event));
    auto type = OH_ArkUI_NodeCustomEvent_GetEventType(event);
    switch (type) {
      case ARKUI_NODE_CUSTOM_EVENT_ON_MEASURE:
        customNode->OnMeasure(event);
        break;
      case ARKUI_NODE_CUSTOM_EVENT_ON_LAYOUT:
        customNode->OnLayout(event);
        break;
      case ARKUI_NODE_CUSTOM_EVENT_ON_DRAW:
        customNode->OnDraw(event);
        break;
      default:
        break;
    }
  }

 private:
  // Custom measurement logic.
  void OnMeasure(ArkUI_NodeCustomEvent* event);

  void OnLayout(ArkUI_NodeCustomEvent* event);

  void OnDraw(ArkUI_NodeCustomEvent* event);

 private:
  ArkUI_NodeContentHandle node_content_handle_ = 0;
  float density_ = 1.f;
  std::unique_ptr<tttext::TextLayout> layout_;
  std::unique_ptr<tttext::TTTextContext> context_;
  std::unique_ptr<tttext::LayoutRegion> region_;
  std::shared_ptr<tttext::IFontManager> font_manager_;
  std::list<std::unique_ptr<tttext::Paragraph>> paragraphs_list_;
  std::list<std::shared_ptr<HarmonyRunDelegate>> unmeasured_delegates_;
  std::unique_ptr<IDocumentView> document_view_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PLATFORM_HARMONY_LYNXTEXTRA_SRC_MAIN_CPP_NATIVE_TEXT_NODE_H_
