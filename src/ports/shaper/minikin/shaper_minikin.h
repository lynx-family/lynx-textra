// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef TEXTLAYOUT_FONT_MANAGER_MINIKIN_H
#define TEXTLAYOUT_FONT_MANAGER_MINIKIN_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "font/i_font_manager.h"
#include "lib/FamilyVariant.h"
#include "lib/Font.h"
#include "lib/FontCollection.h"
#include "lib/FontFamily.h"
#include "lib/FontStyle.h"
#include "lib/LayoutPieces.h"
#include "lib/LocaleList.h"
#include "lib/MinikinFont.h"
#include "lib/MinikinPaint.h"
#include "lib/Range.h"
#include "macro.h"
#include "tt_shaper.h"

using namespace ttoffice;
// TODO: performance issues need to be optimized
class LayoutAppendFunctor {
 public:
  LayoutAppendFunctor() {}
  std::unique_ptr<minikin::LayoutPiece> layout_piece_;
  void operator()(const minikin::LayoutPiece& layoutPiece,
                  const minikin::MinikinPaint& /* paint */) {
    layout_piece_ = std::make_unique<minikin::LayoutPiece>(layoutPiece);
  }
};
class Shaper_Minikin : public TTShaper {
 public:
  Shaper_Minikin();
  ~Shaper_Minikin();

 public:
  const ITypefaceHelper* GetFontById(uint32_t glyph_id) const override;
  void ProcessBidirection(const TTParagraph* paragraph) override;
  void PreProcessParagraph(const TTParagraph* paragraph) override;

 protected:
  TTFontInfo CreateFontInfo(FontIdType font_idx, float text_size) override;

 private:
  minikin::LayoutPiece LayoutPiece(const char* str, uint32_t length, bool dir,
                                   float size);

 protected:
  std::shared_ptr<TTFontManager> font_manager_;
  std::vector<const std::string> font_name_lst_ = {"serif"};
  //    {"Arial Unicode MS", "苹方-简", "Arial Hebrew", "Tahoma"};
  std::shared_ptr<minikin::FontCollection> font_collections_;
  std::unique_ptr<minikin::MinikinPaint> paint_;
};

#endif  // TEXTLAYOUT_FONT_MANAGER_MINIKIN_H
