// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "ios_link_delegate.h"

#import <Foundation/Foundation.h>
IOSLinkDelegate::IOSLinkDelegate(id<TTTextLink> link)
    : ttoffice::LinkDelegate((void*)this), link_delegate_(link) {}

ttoffice::textlayout::DecorationType IOSLinkDelegate::GetDecorateType() {
  if ([link_delegate_ respondsToSelector:@selector(linkStyle)]) {
    auto style = link_delegate_.linkStyle;
    if (style == kUnderLine) {
      return ttoffice::textlayout::DecorationType::kUnderLine;
    }
  }
  return ttoffice::textlayout::DecorationType::kNone;
}

id<TTTextLink> IOSLinkDelegate::GetDelegate() { return link_delegate_; }

bool IOSLinkDelegate::Equals(LinkDelegate* other) {
  return link_delegate_ == ((IOSLinkDelegate*)other)->link_delegate_;
}

std::unique_ptr<ttoffice::textlayout::TextAttachment> IOSLinkDelegate::Clone() {
  return std::make_unique<IOSLinkDelegate>(link_delegate_);
}
