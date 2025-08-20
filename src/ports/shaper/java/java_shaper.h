// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_JAVA_JAVA_SHAPER_H_
#define SRC_PORTS_SHAPER_JAVA_JAVA_SHAPER_H_

#include <jni.h>
#include <textra/fontmgr_collection.h>
#include <textra/platform/java/java_typeface.h>
#include <textra/platform/java/scoped_global_ref.h>

#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "src/textlayout/tt_shaper.h"
namespace ttoffice {
namespace tttext {
using tttext::GlyphID;
struct FontKey {
  uint32_t font_id_;
  float font_size_;
  bool bold_;
  bool italic_;
  bool operator<(const FontKey& other) const {
    if (font_id_ != other.font_id_) {
      return font_id_ < other.font_id_;
    }
    if (font_size_ != other.font_size_) {
      return font_size_ < other.font_size_;
    }
    if (bold_ != other.bold_) {
      return bold_ < other.bold_;
    }
    return italic_ < other.italic_;
  }
};
class JavaShaper final : public TTShaper {
 public:
  JavaShaper() noexcept = delete;
  explicit JavaShaper(const FontmgrCollection& font_collection);
  ~JavaShaper() override = default;

  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override;

 private:
  static std::map<FontKey, std::shared_ptr<JavaTypeface>> font_lst_;
  static std::mutex font_lst_lock_;
  std::unique_ptr<ScopedGlobalRef> java_instance_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_JAVA_JAVA_SHAPER_H_
