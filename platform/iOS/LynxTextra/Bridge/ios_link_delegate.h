// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ios_link_delegate_h
#define ios_link_delegate_h
#include <memory>

#include "TTTextRunDelegate.h"
#include "link_delegate.h"
class IOSLinkDelegate : public ttoffice::LinkDelegate {
 public:
  ~IOSLinkDelegate() = default;
  explicit IOSLinkDelegate(id<TTTextLink> link);

 public:
  ttoffice::textlayout::DecorationType GetDecorateType() override;

  id<TTTextLink> GetDelegate();
  bool Equals(LinkDelegate* other) override;
  std::unique_ptr<ttoffice::textlayout::TextAttachment> Clone() override;

 private:
  id<TTTextLink> link_delegate_;
};

#endif /* ios_link_delegate_h */
