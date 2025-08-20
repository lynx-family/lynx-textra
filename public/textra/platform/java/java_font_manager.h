// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_FONT_MANAGER_H_
#define PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_FONT_MANAGER_H_

#include <jni.h>
#include <textra/font_info.h>
#include <textra/i_font_manager.h>
#include <textra/macro.h>
#include <textra/platform/java/java_typeface.h>
#include <textra/platform/java/scoped_global_ref.h>

#include <list>
#include <memory>
#include <unordered_map>

namespace ttoffice {
namespace tttext {
class L_EXPORT JavaFontManager : public IFontManager {
 public:
  JavaFontManager();
  ~JavaFontManager() override;

 public:
  jobject GetInstance() const;
  void BindJavaHandler();
  int countFamilies() const override;
  TypefaceRef matchFamilyStyle(const char familyName[],
                               const FontStyle&) override;
  TypefaceRef matchFamilyStyleCharacter(const char familyName[],
                                        const FontStyle&, const char* bcp47[],
                                        int bcp47Count,
                                        uint32_t character) override;
  TypefaceRef makeFromFile(const char path[], int ttcIndex) override;
  TypefaceRef legacyMakeTypeface(const char familyName[],
                                 FontStyle style) const override;
  void* getPlatformFontFromTypeface(TypefaceRef typeface) override;

 public:
  std::shared_ptr<JavaTypeface> matchTypeface(const FontDescriptor& fd);
  std::shared_ptr<JavaTypeface> CreateNativeTypeface();

 private:
  std::unique_ptr<ScopedGlobalRef> java_instance_ = nullptr;
  std::list<std::shared_ptr<JavaTypeface>> typeface_list_;
  std::unordered_map<FontDescriptor, std::shared_ptr<JavaTypeface>,
                     FontDescriptor::Hasher>
      typeface_map_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_FONT_MANAGER_H_
