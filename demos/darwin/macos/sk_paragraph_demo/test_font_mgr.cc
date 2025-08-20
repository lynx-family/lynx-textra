// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "demos/darwin/macos/sk_paragraph_demo/test_font_mgr.h"

#include <cassert>
#include <memory>

#include "include/skia/include/core/SkData.h"
#include "include/skia/include/core/SkTypeface.h"
#include "src/ports/platform/skia/skia_typeface_helper.h"

namespace ttoffice {
namespace tttext {
const char* TestFontStyleSet::font_path_ =
    "/Users/user/project.new/textlayout/thirdparty/minikin/tests/data/"
    "NotoSansCJK-Regular.ttc";
TestFontStyleSet::TestFontStyleSet() {
  typeface_ = SkTypeface::MakeFromFile(font_path_, 0);
}
int TestFontStyleSet::count() { return 0; }
void TestFontStyleSet::getStyle(int index, SkFontStyle* font_style,
                                SkString* style) {
  *font_style = SkFontStyle::Normal();
}
SkTypeface* TestFontStyleSet::createTypeface(int index) {
  return typeface_.get();
}
SkTypeface* TestFontStyleSet::matchStyle(const SkFontStyle& pattern) {
  return typeface_.get();
}

TestFontMgr::TestFontMgr() = default;
int TestFontMgr::onCountFamilies() const { return 1; }
void TestFontMgr::onGetFamilyName(int index, SkString* familyName) const {
  *familyName = "TestFont";
}
SkFontStyleSet* TestFontMgr::onCreateStyleSet(int index) const {
  return new TestFontStyleSet;
}
SkFontStyleSet* TestFontMgr::onMatchFamily(const char* familyName) const {
  return new TestFontStyleSet;
}
SkTypeface* TestFontMgr::onMatchFamilyStyle(const char* familyName,
                                            const SkFontStyle& style) const {
  return font_style_set_.typeface_.get();
}
SkTypeface* TestFontMgr::onMatchFamilyStyleCharacter(
    const char* familyName, const SkFontStyle& style, const char** bcp47,
    int bcp47Count, SkUnichar character) const {
  return font_style_set_.typeface_.get();
}
sk_sp<SkTypeface> TestFontMgr::onMakeFromData(sk_sp<SkData> sp,
                                              int ttcIndex) const {
  return SkTypeface::MakeFromData(sp, 0);
}
sk_sp<SkTypeface> TestFontMgr::onMakeFromStreamIndex(
    std::unique_ptr<SkStreamAsset> ptr, int ttcIndex) const {
  return SkTypeface::MakeFromFile(TestFontStyleSet::font_path_, 0);
}
sk_sp<SkTypeface> TestFontMgr::onMakeFromStreamArgs(
    std::unique_ptr<SkStreamAsset> ptr,
    const SkFontArguments& arguments) const {
  return SkTypeface::MakeFromFile(TestFontStyleSet::font_path_, 0);
}
sk_sp<SkTypeface> TestFontMgr::onMakeFromFile(const char* path,
                                              int ttcIndex) const {
  return SkTypeface::MakeFromFile(path, ttcIndex);
}
sk_sp<SkTypeface> TestFontMgr::onLegacyMakeTypeface(const char* familyName,
                                                    SkFontStyle style) const {
  return SkTypeface::MakeFromFile(TestFontStyleSet::font_path_, 0);
}
std::shared_ptr<IFontManager> IFontManager::RefDefault() {
  return std::make_shared<TestTTFontMgr>();
}
TestTTFontMgr::TestTTFontMgr() {
  typeface_ = std::make_shared<SkiaTypefaceHelper>(
      SkTypeface::MakeFromFile(TestFontStyleSet::font_path_, 0));
}
int TestTTFontMgr::countFamilies() const { return 1; }
TypefaceRef TestTTFontMgr::matchFamilyStyle(const char* familyName,
                                            const FontStyle& style) {
  return typeface_;
}
TypefaceRef TestTTFontMgr::matchFamilyStyleCharacter(const char* familyName,
                                                     const FontStyle& style,
                                                     const char** bcp47,
                                                     int bcp47Count,
                                                     uint32_t character) {
  return typeface_;
}
TypefaceRef TestTTFontMgr::makeFromFile(const char* path, int ttcIndex) {
  return typeface_;
}
TypefaceRef TestTTFontMgr::legacyMakeTypeface(const char* familyName,
                                              FontStyle style) const {
  return typeface_;
}
}  // namespace tttext
}  // namespace ttoffice
