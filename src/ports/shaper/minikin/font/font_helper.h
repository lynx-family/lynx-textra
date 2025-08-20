// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


#ifndef SRC_PORTS_SHAPER_MINIKIN_FONT_HELPER_
#define SRC_PORTS_SHAPER_MINIKIN_FONT_HELPER_

#include "i_typeface_helper.h"
#include "shaper/minikin/lib/MinikinFont.h"

namespace minikin {
class FontHelper : public MinikinFont {
 public:
  explicit FontHelper(std::shared_ptr<ttoffice::ITypefaceHelper> typeface);
  ~FontHelper() override;

  // MinikinFont overrides.
  float GetHorizontalAdvance(uint32_t glyph_id, const MinikinPaint& paint,
                             const FontFakery& fakery) const override;
  void GetBounds(MinikinRect* bounds, uint32_t glyph_id,
                 const MinikinPaint& paint,
                 const FontFakery& fakery) const override;
  void GetFontExtent(MinikinExtent* extent, const MinikinPaint& paint,
                     const FontFakery& fakery) const override;

  const void* GetFontData() const override { return typeface_->GetFontData(); }
  size_t GetFontSize() const override { return typeface_->GetFontSize(); }
  int GetFontIndex() const override { return typeface_->GetFontIndex(); }
  const std::vector<minikin::FontVariation>& GetAxes() const override {
    return axes_;
  }

 public:
  std::shared_ptr<ttoffice::ITypefaceHelper> GetTypeface() const;
  uint16_t UnicodeToGlyph(uint32_t codepoint,
                          uint32_t variationSelector) const override;
  void GetFontInfo(TTFontInfo& info, float font_size) const override;

 private:
  std::shared_ptr<ttoffice::ITypefaceHelper> typeface_;
  std::vector<minikin::FontVariation> axes_;
};
}  // namespace minikin

#endif  // SRC_PORTS_SHAPER_MINIKIN_FONT_HELPER_
