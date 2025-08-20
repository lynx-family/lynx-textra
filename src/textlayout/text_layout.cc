// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/fontmgr_collection.h>
#include <textra/macro.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include "src/textlayout/text_layout_impl.h"
#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {

TextLayout::TextLayout(FontmgrCollection* font_collection, ShaperType type)
    : TextLayout(TTShaper::CreateShaper(font_collection, type)) {}

TextLayout::TextLayout(std::unique_ptr<TTShaper> shaper)
    : shaper_(std::move(shaper)) {}

TextLayout::~TextLayout() = default;

LayoutResult TextLayout::LayoutEx(Paragraph* para, LayoutRegion* page,
                                  TTTextContext& context) const {
  TTASSERT(para);
  TTASSERT(page);
  return TextLayoutImpl::LayoutEx(para, page, context, shaper_.get());
}

}  // namespace tttext
}  // namespace ttoffice
