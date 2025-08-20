// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ios_layout_drawer_listener_h
#define ios_layout_drawer_listener_h

#import "TTTextDrawerDelegate.h"
#include "layout_definition.h"
#include "layout_drawer_listener.h"
#include "tt_color.h"
using namespace ttoffice::textlayout;
class IOSLayoutDrawerListener
    : public ttoffice::textlayout::LayoutDrawerListener {
 public:
  IOSLayoutDrawerListener(id<TTTextDrawerDelegate> listener)
      : listener_(listener) {}

 public:
  virtual uint32_t FetchThemeColor(ThemeCategory type,
                                   const TTColor& run_color) override;

 private:
  __weak id<TTTextDrawerDelegate> listener_;
};
#endif /* ios_layout_drawer_listener_h */
