// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_RUN_OBJECT_RUN_H_
#define SRC_TEXTLAYOUT_RUN_OBJECT_RUN_H_

#include <textra/macro.h>
#include <textra/run_delegate.h>

#include <memory>
#include <string>
#include <utility>

#include "src/textlayout/run/base_run.h"
namespace ttoffice {
namespace tttext {
class ObjectRun final : public BaseRun {
 public:
  ObjectRun(ParagraphImpl* paragraph, std::shared_ptr<RunDelegate> delegate,
            uint32_t start_char_pos, uint32_t end_char_pos, RunType type)
      : BaseRun(paragraph, Style(), start_char_pos, end_char_pos, type) {
    SetRunDelegate(std::move(delegate));
    boundary_type_ = BoundaryType::kLineBreakable;
  }
  ~ObjectRun() override = default;

 public:
  //  float GetWidth(uint32_t char_start_in_range, uint32_t char_count,
  //                 std::vector<float> *pos_array) const override;
  //  void Layout() override { delegate_->Layout(); }
  //  void Draw(ICanvasHelper *canvas, uint32_t char_pos_in_run,
  //            uint32_t char_count, float x_offset, float y_offset,
  //            Painter *painter) const override;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_RUN_OBJECT_RUN_H_
