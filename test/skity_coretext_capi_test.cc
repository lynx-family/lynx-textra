// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <gtest/gtest.h>
#include <textra/capi/platform/textra_capi_skity_coretext.h>
#include <textra/platform/skity/skity_font_manager_coretext_webkit.h>

#include "demos/capi/skity_capi_utils.hpp"

namespace {
namespace sc = tttext::skity_c;

TEST(SkityCppCoreTextTest, ImportedFontSurvivesCallerRelease) {
  auto native = CTFontCreateWithName(CFSTR("Helvetica"), 24, nullptr);
  ASSERT_NE(native, nullptr);
  const UniChar text = 'A';
  CGGlyph expected = 0;
  ASSERT_TRUE(CTFontGetGlyphsForCharacters(native, &text, &expected, 1));
  tttext::SkityFontManagerCoreText manager;
  auto face = manager.createTypefaceFromPlatformFont(native);
  CFRelease(native);
  ASSERT_NE(face, nullptr);
  EXPECT_EQ(face->UnicharToGlyph('A'), expected);
  EXPECT_GT(face->GetHorizontalAdvance(expected, 24), 0);
}

TEST(SkityCppCoreTextTest, WebkitLiteralFamilyMatchingKeepsSelectedStyle) {
  tttext::SkityFontManagerCoreTextWebkit manager;
  auto face = manager.legacyMakeTypeface("Helvetica", tttext::FontStyle());
  ASSERT_NE(face, nullptr);
  EXPECT_NE(face->UnicharToGlyph('A'), 0);
  auto other =
      manager.matchFamilyStyleLiteral("Helvetica", tttext::FontStyle());
  ASSERT_NE(other, nullptr);
  const auto glyph = face->UnicharToGlyph('A');
  EXPECT_EQ(glyph, other->UnicharToGlyph('A'));
  EXPECT_FLOAT_EQ(face->GetHorizontalAdvance(glyph, 24),
                  other->GetHorizontalAdvance(glyph, 24));
}

TEST(SkityCoreTextCapiTest, IncompleteNativeFontBridgeDoesNotAcquireInputs) {
  auto manager =
      sc::Take(skity_font_manager_ref_default(), skity_font_manager_destroy);
  ASSERT_TRUE(manager);
  const auto families = skity_font_manager_count_families(manager.get());
  int userdata = 0;
  textra_font_manager_options_t fonts{};
  fonts.struct_size = sizeof(fonts);
  fonts.font_manager = manager.get();
  fonts.userdata = &userdata;
  EXPECT_EQ(textra_skity_coretext_font_manager_callbacks_initialize(&fonts),
            TEXTRA_STATUS_UNSUPPORTED);
  EXPECT_EQ(fonts.font_manager, manager.get());
  EXPECT_EQ(fonts.userdata, &userdata);
  EXPECT_EQ(fonts.manager_callbacks, nullptr);
  EXPECT_EQ(fonts.typeface_callbacks, nullptr);
  EXPECT_EQ(skity_font_manager_count_families(manager.get()), families);

  auto native = CTFontCreateWithName(CFSTR("Helvetica"), 24, nullptr);
  textra_typeface_options_t face{};
  face.struct_size = sizeof(face);
  face.native_font = native;
  face.userdata = &userdata;
  EXPECT_EQ(textra_skity_coretext_typeface_callbacks_initialize(&face),
            TEXTRA_STATUS_UNSUPPORTED);
  EXPECT_EQ(face.native_font, native);
  EXPECT_EQ(face.userdata, &userdata);
  EXPECT_EQ(face.callbacks, nullptr);
  CFRelease(native);
}

TEST(SkityCoreTextCapiTest, CanvasRemainsBorrowedWithIncompleteFontBridge) {
  auto bitmap = sc::MakeBitmap(16, 16);
  auto canvas = sc::MakeCanvas(bitmap.get());
  ASSERT_TRUE(canvas);
  textra_canvas_options_t options{};
  options.struct_size = sizeof(options);
  options.native_canvas = canvas.get();
  ASSERT_EQ(textra_skity_coretext_canvas_callbacks_initialize(&options),
            TEXTRA_STATUS_OK);
  EXPECT_EQ(options.backend, TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT);
  EXPECT_EQ(options.native_canvas, canvas.get());
  ASSERT_EQ(options.callbacks->fill_rect(nullptr, canvas.get(), 0, 0, 16, 16,
                                         0xff123456),
            TEXTRA_STATUS_OK);
  options.callbacks->destroy(nullptr, canvas.get());
  skity_canvas_draw_color(canvas.get(), 0xff00ff00, SKITY_BLEND_MODE_SRC);
  const auto* pixels =
      static_cast<const uint8_t*>(skity_bitmap_get_pixels(bitmap.get()));
  EXPECT_EQ(pixels[1], 255);
  EXPECT_EQ(pixels[3], 255);
}
}  // namespace
