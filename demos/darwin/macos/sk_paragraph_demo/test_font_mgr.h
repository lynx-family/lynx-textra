// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_DARWIN_MACOS_SK_PARAGRAPH_DEMO_TEST_FONT_MGR_H_
#define DEMOS_DARWIN_MACOS_SK_PARAGRAPH_DEMO_TEST_FONT_MGR_H_

#include <textra/i_font_manager.h>

#include <memory>

#include "include/skia/include/core/SkFontMgr.h"
namespace ttoffice {
namespace tttext {

class TestFontMgr;
class TestFontStyleSet : public SkFontStyleSet {
  friend TestFontMgr;

 public:
  TestFontStyleSet();
  static const char* font_path_;

 private:
  int count() override;
  void getStyle(int index, SkFontStyle* font_style, SkString* style) override;
  SkTypeface* createTypeface(int index) override;
  SkTypeface* matchStyle(const SkFontStyle& pattern) override;

  sk_sp<SkTypeface> typeface_;
};
class TestFontMgr : public SkFontMgr {
 public:
  TestFontMgr();

 private:
  int onCountFamilies() const override;
  void onGetFamilyName(int index, SkString* familyName) const override;
  SkFontStyleSet* onCreateStyleSet(int index) const override;
  SkFontStyleSet* onMatchFamily(const char* familyName) const override;
  SkTypeface* onMatchFamilyStyle(const char* familyName,
                                 const SkFontStyle& style) const override;
  SkTypeface* onMatchFamilyStyleCharacter(const char* familyName,
                                          const SkFontStyle& style,
                                          const char** bcp47, int bcp47Count,
                                          SkUnichar character) const override;
  sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> sp,
                                   int ttcIndex) const override;
  sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> ptr,
                                          int ttcIndex) const override;
  sk_sp<SkTypeface> onMakeFromStreamArgs(
      std::unique_ptr<SkStreamAsset> ptr,
      const SkFontArguments& arguments) const override;
  sk_sp<SkTypeface> onMakeFromFile(const char* path,
                                   int ttcIndex) const override;
  sk_sp<SkTypeface> onLegacyMakeTypeface(const char* familyName,
                                         SkFontStyle style) const override;

 private:
  TestFontStyleSet font_style_set_;
};

class TestTTFontMgr : public TTFontManager {
 public:
  TestTTFontMgr();
  int countFamilies() const override;
  TypefaceRef matchFamilyStyle(const char* familyName,
                               const FontStyle& style) override;
  TypefaceRef matchFamilyStyleCharacter(const char* familyName,
                                        const FontStyle& style,
                                        const char** bcp47, int bcp47Count,
                                        uint32_t character) override;
  TypefaceRef makeFromFile(const char* path, int ttcIndex) override;
  TypefaceRef legacyMakeTypeface(const char* familyName,
                                 FontStyle style) const override;
  std::shared_ptr<TTTypeface> typeface_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // DEMOS_DARWIN_MACOS_SK_PARAGRAPH_DEMO_TEST_FONT_MGR_H_
