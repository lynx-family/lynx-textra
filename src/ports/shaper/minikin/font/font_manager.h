// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


#ifndef SRC_PORTS_SHAPER_MINIKIN_FONT_MANAGER_
#define SRC_PORTS_SHAPER_MINIKIN_FONT_MANAGER_

#include "i_font_manager.h"
#include "i_typeface_helper.h"

namespace ttoffice {
namespace tttext {

class FontManager : public IFontManager {
 public:
  explicit FontManager();
  ~FontManager();

  int countFamilies() const override;
  std::shared_ptr<minikin::FontFamily> matchFamilyStyle(
      const char* familyName, const minikin::FontStyle& style) override;
  std::shared_ptr<minikin::FontFamily> matchFamilyStyleCharacter(
      const char* familyName, const minikin::FontStyle& style,
      const char** bcp47, int bcp47Count, uint32_t character) override;
  std::shared_ptr<minikin::FontFamily> makeFromFile(const char* path,
                                                    int ttcIndex) override;

  FontManager(const FontManager&) = delete;
  FontManager operator==(const FontManager&) = delete;

 private:
  // SkFontStyle ToSkFontStyle(const FontStyle &) const;
  std::shared_ptr<minikin::FontFamily> RegisterUniqueFont(
      std::shared_ptr<ITypefaceHelper>, const minikin::FontStyle& style);

 private:
  std::shared_ptr<ITypefaceHelper> typeface_;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_MINIKIN_FONT_MANAGER_
