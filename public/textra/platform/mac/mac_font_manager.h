// Copyright (c) 2022 The Bytedance, Inc . All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//
// Created by wenhe on 2022/10/9.
//

#ifndef PUBLIC_TEXTRA_PLATFORM_MAC_MAC_FONT_MANAGER_H_
#define PUBLIC_TEXTRA_PLATFORM_MAC_MAC_FONT_MANAGER_H_

#import <CoreText/CTFont.h>
#include <textra/i_font_manager.h>
#include <textra/macro.h>
#include <textra/platform/mac/typeface_coretext.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace ttoffice {
namespace tttext {
/**
 * @brief A font manager implementation backed by Apple's CoreText framework.
 */
class FontManagerCoreText : public IFontManager {
 public:
  FontManagerCoreText() = default;
  ~FontManagerCoreText() override = default;

 public:
  int32_t countFamilies() const override {
    return static_cast<int32_t>(font_family_map_.size());
  }
  TypefaceRef matchFamilyStyle(const char* familyName,
                               const FontStyle& style) override {
    auto iter = font_family_map_.find(familyName);
    if (iter == font_family_map_.end()) {
      return nullptr;
    }
    auto ct_font = CTFontCreateWithName(iter->second, 24, nullptr);
    return std::make_shared<TypefaceCoreText>(ct_font, id_++);
  }
  TypefaceRef matchFamilyStyleCharacter(const char* familyName,
                                        const FontStyle& style,
                                        const char** bcp47, int bcp47Count,
                                        uint32_t character) override {
    TTASSERT(false);
    return nullptr;
  }
  TypefaceRef makeFromFile(const char* path, int ttcIndex) override {
    TTASSERT(false);
    return nullptr;
  }
  TypefaceRef legacyMakeTypeface(const char* familyName,
                                 FontStyle style) const override {
    return nullptr;
  }

  TypefaceRef createTypefaceFromPlatformFont(
      const void* platform_font) override {
    CTFontRef font_ref = (CTFontRef)platform_font;
    return std::make_shared<TypefaceCoreText>(font_ref, id_++);
  }
  void* getPlatformFontFromTypeface(TypefaceRef typeface) override {
    auto ct_typeface_helper =
        std::static_pointer_cast<TypefaceCoreText>(typeface);
    return (void*)(ct_typeface_helper->GetFontRef());  // NOLINT
  }

  void RegisterFont(CGDataProviderRef data, const char* family_name) {
    CGFontRef cg_font = CGFontCreateWithDataProvider(data);
    CFStringRef post_script_name = CGFontCopyPostScriptName(cg_font);

    CFErrorRef error;
    if (!CTFontManagerRegisterGraphicsFont(cg_font, &error)) {
      //      NSLog(@"Typeface Register failed: %@", (__bridge NSError*)error);
      CFRelease(error);
      return;
    }

    font_family_map_[family_name] = post_script_name;
    //    NSLog(@"Typeface register success: %s:%@", family_name,
    //    post_script_name);
  }

 private:
  std::atomic<uint32_t> id_ = {0};
  std::unordered_map<std::string, CFStringRef> font_family_map_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_MAC_MAC_FONT_MANAGER_H_
