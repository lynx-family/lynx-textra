// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_canvas_proxy.h"

namespace textra_capi {
namespace {
textra_status_t CheckParagraph(const textra_paragraph* p,
                               const textra_text_layout* engine,
                               uint32_t backend) {
  if (engine->backend != backend) return TEXTRA_STATUS_BACKEND_MISMATCH;
  auto status = CheckFonts(p, engine);
  if (status) return status;
  for (const auto& d : p->delegates)
    if (!d->callbacks.draw) return TEXTRA_STATUS_UNSUPPORTED;
  if (p->style && p->style->ellipsis &&
      !p->style->ellipsis->impl->callbacks.draw)
    return TEXTRA_STATUS_UNSUPPORTED;
  return TEXTRA_STATUS_OK;
}
}  // namespace
textra_status_t Draw(textra_layout_drawer* drawer,
                     const textra_layout_region* region,
                     const textra_paragraph* p, uint32_t index,
                     const textra_text_range_t* range, float x, float y) {
  if (!drawer || (!region && !p) || !Finite(x) || !Finite(y))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || drawer->active || drawer->canvas->active ||
      (region && region->active))
    return TEXTRA_STATUS_BUSY;
  tt::TextLine* line = nullptr;
  if (p) {
    if (p->impl->GetType() != tt::ParagraphType::kSingleLine)
      return TEXTRA_STATUS_UNSUPPORTED;
    if (!p->valid_layout || !p->single_engine)
      return TEXTRA_STATUS_INVALID_STATE;
    auto status =
        CheckParagraph(p, p->single_engine.get(), drawer->canvas->backend);
    if (status) return status;
  } else {
    if (index != UINT32_MAX) {
      line = region->impl->GetLine(index);
      if (!line) return TEXTRA_STATUS_OUT_OF_RANGE;
      if (range &&
          (range->start > range->end || range->end > line->GetCharCount()))
        return TEXTRA_STATUS_OUT_OF_RANGE;
    }
    for (const auto& dep : region->dependencies) {
      auto status = CheckParagraph(dep->paragraph.get(), dep->engine.get(),
                                   drawer->canvas->backend);
      if (status) return status;
    }
  }
  bool unused_region_flag = false;
  Active a(drawer->active), b(drawer->canvas->active),
      r(region ? const_cast<textra_layout_region*>(region)->active
               : unused_region_flag);
  OperationScope scope(drawer->canvas.get());
  CanvasProxy canvas(drawer->canvas->impl.get());
  tt::LayoutDrawer impl(&canvas);
  impl.SetListener(&drawer->listener);
  canvas.StartPaint();
  if (p) {
    if (!impl.DrawSingleLine(*p->impl, x, y)) {
      canvas.EndPaint();
      return TEXTRA_STATUS_INVALID_STATE;
    }
  } else if (line) {
    impl.DrawTextLine(line, range ? range->start : 0,
                      range ? range->end : line->GetCharCount());
  } else {
    impl.DrawLayoutPage(region->impl.get());
  }
  canvas.EndPaint();
  return scope.local.error;
}
}  // namespace textra_capi
