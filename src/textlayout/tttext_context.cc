// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/layout_region.h>
#include <textra/tttext_context.h>

#include <memory>

#include "src/textlayout/layout_position.h"

namespace ttoffice {
namespace tttext {

TTTextContext::TTTextContext()
    : position_(std::make_unique<LayoutPosition>(0, 0)) {}

TTTextContext::~TTTextContext() = default;

void TTTextContext::Reset() {
  *position_ = LayoutPosition{0, 0};
  layout_bottom_ = 0;
}

void TTTextContext::ResetLayoutPosition(const LayoutPosition& position) {
  *position_ = position;
}

void TTTextContext::UpdateContextSpace(TextLine* line) {
  SetLayoutBottom(line->GetLineBottom());
}
}  // namespace tttext
}  // namespace ttoffice
