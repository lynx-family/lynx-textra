// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


#ifndef TEXTLAYOUT_SRC_TEXTLAYOUT_SHAPER_FONT_SK_FONT_MANAGER_H_
#define TEXTLAYOUT_SRC_TEXTLAYOUT_SHAPER_FONT_SK_FONT_MANAGER_H_

#include <unordered_map>

#include "core/SkFontStyle.h"
#include "core/SkRefCnt.h"
#include "i_font_manager.h"
class SkFontMgr;
class SkTypeface;
namespace ttoffice {
namespace tttext {
class SkFontManager : public IFontManager {
 public:
  SkFontManager();
  ~SkFontManager();

 public:
  int countFamilies() const override;
  std::shared_ptr<FontFamily> matchFamilyStyle(const char* familyName,
                                               const FontStyle& style) override;
  std::shared_ptr<FontFamily> matchFamilyStyleCharacter(
      const char* familyName, const FontStyle& style, const char** bcp47,
      int bcp47Count, uint32_t character) override;
  std::shared_ptr<FontFamily> makeFromFile(const char* path,
                                           int ttcIndex) override;

 public:
  SkFontManager(const SkFontManager&) = delete;
  SkFontManager operator==(const SkFontManager&) = delete;

 private:
  SkFontStyle ToSkFontStyle(const FontStyle&) const;
  std::shared_ptr<FontFamily> RegisterUniqueFont(sk_sp<SkTypeface>&,
                                                 const FontStyle& style);

 private:
  sk_sp<SkFontMgr> sk_font_mgr_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // TEXTLAYOUT_SRC_TEXTLAYOUT_SHAPER_FONT_SK_FONT_MANAGER_H_
