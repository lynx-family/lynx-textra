/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MINIKIN_TEST_FREE_TYPE_MINIKIN_FONT_FOR_TEST_H
#define MINIKIN_TEST_FREE_TYPE_MINIKIN_FONT_FOR_TEST_H

#include <string>

#include "freetype/include/ft2build.h"
#include "minikin/MinikinFont.h"
#include FT_FREETYPE_H

#include "minikin/Macros.h"

namespace minikin {

class FreeTypeMinikinFont : public MinikinFont {
 public:
  FreeTypeMinikinFont(const std::string& font_path, int index);
  explicit FreeTypeMinikinFont(const std::string& font_path)
      : FreeTypeMinikinFont(font_path, 0) {}
  ~FreeTypeMinikinFont() override;

  // MinikinFont overrides.
  float GetHorizontalAdvance(uint32_t glyph_id, const MinikinPaint& paint,
                             const FontFakery& fakery) const override;
  void GetBounds(MinikinRect* bounds, uint32_t glyph_id,
                 const MinikinPaint& paint,
                 const FontFakery& fakery) const override;
  void GetFontExtent(MinikinExtent* extent, const MinikinPaint& paint,
                     const FontFakery& fakery) const override;

  const std::string& fontPath() const { return mFontPath; }

  const void* GetFontData() const override { return mFontData; }
  size_t GetFontSize() const override { return mFontSize; }
  int GetFontIndex() const override { return mFontIndex; }
  const std::vector<minikin::FontVariation>& GetAxes() const override {
    return mAxes;
  }

 private:
  const std::string mFontPath;
  const int mFontIndex;
  void* mFontData;
  size_t mFontSize;
  std::vector<minikin::FontVariation> mAxes;

  FT_Library mFtLibrary;
  FT_Face mFtFace;

  MINIKIN_PREVENT_COPY_AND_ASSIGN(FreeTypeMinikinFont);
};

}  // namespace minikin

#endif  // MINIKIN_TEST_FREE_TYPE_MINIKIN_FONT_FOR_TEST_H
