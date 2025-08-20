// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_TYPEFACE_H_
#define PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_TYPEFACE_H_
#include <jni.h>
#include <textra/i_typeface_helper.h>
#include <textra/platform/java/scoped_global_ref.h>

#include <memory>
namespace ttoffice {
namespace tttext {
using tttext::GlyphID;
using tttext::Unichar;
class JavaTypeface final : public ITypefaceHelper {
 public:
  explicit JavaTypeface(uint32_t unique_id);
  ~JavaTypeface() override;

 public:
  void BindJavaHandler(jobject handler, uint32_t unique_id);

 public:
  jobject GetInstance() const;
  float GetHorizontalAdvance(GlyphID glyph_id, float font_size) const override;
  void GetHorizontalAdvances(GlyphID* glyph_ids, uint32_t count, float* widths,
                             float font_size) const override;
  void GetWidthBound(float* rect_f, GlyphID glyph_id,
                     float font_size) const override;
  void GetWidthBounds(float* rect_ltrb, GlyphID glyphs[], uint32_t glyph_count,
                      float font_size) override;
  const void* GetFontData() const override;
  size_t GetFontDataSize() const override;
  int GetFontIndex() const override;
  uint32_t GetUnitsPerEm() const override;
  uint16_t UnicharToGlyph(Unichar codepoint,
                          uint32_t variationSelector = 0) const override;
  void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                        GlyphID* glyphs) const override;

 protected:
  void OnCreateFontInfo(FontInfo* info, float font_size) const override;

 private:
  std::unique_ptr<ScopedGlobalRef> java_instance_ = nullptr;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_TYPEFACE_H_
