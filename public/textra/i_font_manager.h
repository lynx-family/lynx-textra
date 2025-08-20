// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_I_FONT_MANAGER_H_
#define PUBLIC_TEXTRA_I_FONT_MANAGER_H_

#include <textra/i_typeface_helper.h>

#include <memory>

namespace ttoffice {
namespace tttext {
/**
 * @brief A platform-independent interface for font management.
 *
 * IFontManager provides a unified abstraction for font management operations
 * including:
 * - Font family enumeration
 * - Font matching by family name and style
 * - Character fallback for multilingual text
 * - Font file loading
 * - Conversion between platform-specific font representations and the common
 * typeface abstraction
 *
 * This interface allows the text layout system to work with fonts consistently
 * across different platforms (iOS, Android, Harmony). Implementations handle
 * the platform-specific details of font access, selection, and conversion.
 *
 * @note Copied from SkFontMgr.
 */
class IFontManager : public std::enable_shared_from_this<IFontManager> {
 public:
  virtual ~IFontManager() = default;

  virtual int countFamilies() const = 0;

  /**
   *  Find the closest matching typeface to the specified familyName and style
   *  and return a ref to it. The caller must call unref() on the returned
   *  object. Will return nullptr if no 'good' match is found.
   *
   *  Passing |nullptr| as the parameter for |familyName| will return the
   *  default system font.
   *
   *  It is possible that this will return a style set not accessible from
   *  createStyleSet(int) or matchFamily(const char[]) due to hidden or
   *  auto-activated fonts.
   */
  virtual TypefaceRef matchFamilyStyle(const char familyName[],
                                       const FontStyle&) = 0;

  /**
   *  Use the system fallback to find a typeface for the given character.
   *  Note that bcp47 is a combination of ISO 639, 15924, and 3166-1 codes,
   *  so it is fine to just pass a ISO 639 here.
   *
   *  Will return NULL if no family can be found for the character
   *  in the system fallback.
   *
   *  Passing |nullptr| as the parameter for |familyName| will return the
   *  default system font.
   *
   *  bcp47[0] is the least significant fallback, bcp47[bcp47Count-1] is the
   *  most significant. If no specified bcp47 codes match, any font with the
   *  requested character will be matched.
   */
  virtual TypefaceRef matchFamilyStyleCharacter(const char familyName[],
                                                const FontStyle&,
                                                const char* bcp47[],
                                                int bcp47Count,
                                                uint32_t character) = 0;

  /**
   *  Create a typeface for the specified fileName and TTC index
   *  (pass 0 for none) or NULL if the file is not found, or its contents are
   *  not recognized. The caller must call unref() on the returned object
   *  if it is not null.
   */
  virtual TypefaceRef makeFromFile(const char path[], int ttcIndex = 0) = 0;

  virtual TypefaceRef legacyMakeTypeface(const char familyName[],
                                         FontStyle style) const = 0;

  virtual TypefaceRef createTypefaceFromPlatformFont(
      const void* platform_font) {
    auto pf = const_cast<void*>(platform_font);
    return static_cast<ITypefaceHelper*>(pf)->shared_from_this();
  }

  virtual void* getPlatformFontFromTypeface(TypefaceRef typeface) {
    return typeface.get();
  }
};
}  // namespace tttext
}  // namespace ttoffice
using FontManagerRef = std::shared_ptr<tttext::IFontManager>;
#endif  // PUBLIC_TEXTRA_I_FONT_MANAGER_H_
