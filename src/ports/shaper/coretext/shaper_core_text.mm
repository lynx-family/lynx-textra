// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "shaper_core_text.h"

#ifndef MIN
#import <objc/NSObjCRuntime.h>
#endif

#ifdef TTTEXT_OS_IOS
#import <UIKit/UIKit.h>
#endif

#ifdef TTTEXT_OS_MAC
#import <AppKit/AppKit.h>
#endif

#include <textra/font_info.h>
#include <textra/fontmgr_collection.h>
#include <textra/i_canvas_helper.h>
#include <textra/style.h>

#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
ShaperCoreText::ShaperCoreText(FontmgrCollection& font_collection) noexcept
    : TTShaper(font_collection), default_font_desc_(nullptr){};

ShaperCoreText::~ShaperCoreText() {
  if (default_font_desc_ != nullptr) {
    CFRelease(default_font_desc_);
  }
}
void ShaperCoreText::ProcessBidirection(const char32_t* text, uint32_t length,
                                        WriteDirection write_direction,
                                        uint32_t* visual_map,
                                        uint32_t* logical_map,
                                        uint8_t* dir_vec) {
  auto u16_str = ttoffice::base::U32StringToU16(text, length);
  uint32_t u16_to_u32_map[u16_str.length() + 1];
  CTWritingDirection direction = kCTWritingDirectionNatural;
  if (write_direction == WriteDirection::kLTR ||
      write_direction == WriteDirection::kTTB) {
    direction = kCTWritingDirectionLeftToRight;
  } else if (write_direction == WriteDirection::kRTL ||
             write_direction == WriteDirection::kBTT) {
    direction = kCTWritingDirectionRightToLeft;
  }
  CFMutableDictionaryRef dict =
      CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks);
  CTParagraphStyleRef para_style = NULL;
  if (direction != kCTWritingDirectionNatural) {
    CTParagraphStyleSetting setting;
    setting.spec = kCTParagraphStyleSpecifierBaseWritingDirection;
    setting.valueSize = sizeof(direction);
    setting.value = &direction;
    para_style = CTParagraphStyleCreate(&setting, 1);
    CFDictionaryAddValue(dict, kCTParagraphStyleAttributeName, para_style);
  }
  auto attr_text = GenerateAttributeString(
      u16_str.data(), static_cast<uint32_t>(u16_str.length()), u16_to_u32_map,
      dict);
  auto line = CTLineCreateWithAttributedString(attr_text);
  CFArrayRef array_ref = CTLineGetGlyphRuns(line);
  auto array_count = CFArrayGetCount(array_ref);

  int order = 0;
  for (int run_idx = 0; run_idx < array_count; run_idx++) {
    CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(array_ref, run_idx);
    CFRange range = CTRunGetStringRange(run);
    auto char_start = u16_to_u32_map[range.location];
    auto char_end = u16_to_u32_map[range.location + range.length];
    auto char_count = char_end - char_start;
    auto run_status = CTRunGetStatus(run);
    bool is_rtl = static_cast<bool>(run_status & kCTRunStatusRightToLeft);
    if (is_rtl) {
      for (long k = char_count - 1; k >= 0; k--) {
        dir_vec[char_start + k] = 1;
        visual_map[char_start + k] = static_cast<unsigned int>(order++);
      }
    } else {
      for (long k = 0; k < char_count; k++) {
        dir_vec[char_start + k] = 0;
        visual_map[char_start + k] = static_cast<unsigned int>(order++);
      }
    }
  }
  CFRelease(attr_text);
  CFRelease(dict);
  CFRelease(line);
  if (para_style != NULL) {
    CFRelease(para_style);
  }
}

class CTShapingResult : public PlatformShapingResultReader {
 public:
  CTShapingResult(){};

 public:
  uint32_t GlyphCount() const override {
    return static_cast<uint32_t>(ct_glyphs_.size());
  }
  uint32_t TextCount() const override { return text_length_; }
  GlyphID ReadGlyphID(uint32_t idx) const override { return ct_glyphs_[idx]; }
  float ReadAdvanceX(uint32_t idx) const override {
    return ct_advances_[idx].width;
  }
  float ReadAdvanceY(uint32_t idx) const override {
    return ct_advances_[idx].height;
  }
  float ReadPositionX(uint32_t idx) const override {
    return ct_position_[idx].x;
  }
  float ReadPositionY(uint32_t idx) const override {
    return ct_position_[idx].y;
  }
  uint32_t ReadIndices(uint32_t idx) const override {
    return static_cast<uint32_t>(ct_indices_[idx]);
  }
  TypefaceRef ReadFontId(uint32_t idx) const override { return typeface_[idx]; }

  void AppendResult(TypefaceRef typeface, CGGlyph* glyphs, CGSize* advances,
                    CFIndex* indices, CGPoint* position, int glyph_count,
                    int char_count, bool is_rtl) {
    if (is_rtl) {
      ct_glyphs_.insert(ct_glyphs_.begin(), glyphs, glyphs + glyph_count);
      ct_advances_.insert(ct_advances_.begin(), advances,
                          advances + glyph_count);
      ct_indices_.insert(ct_indices_.begin(), indices, indices + glyph_count);
      typeface_.insert(typeface_.begin(), glyph_count, typeface);
    } else {
      ct_glyphs_.insert(ct_glyphs_.end(), glyphs, glyphs + glyph_count);
      ct_advances_.insert(ct_advances_.end(), advances, advances + glyph_count);
      ct_indices_.insert(ct_indices_.end(), indices, indices + glyph_count);
      typeface_.insert(typeface_.end(), glyph_count, typeface);
    }
    ct_position_.insert(ct_position_.end(), position, position + glyph_count);
    text_length_ += char_count;
  }

 public:
  std::vector<CGGlyph> ct_glyphs_;
  std::vector<CGSize> ct_advances_;
  std::vector<CFIndex> ct_indices_;
  std::vector<CGPoint> ct_position_;
  std::vector<TypefaceRef> typeface_;
  uint32_t text_length_{0};
};
void ShaperCoreText::OnShapeText(const ShapeKey& key,
                                 ShapeResult* result) const {
  CFDictionaryRef attributes = GenerateAttributes(key);
  auto u16_str = base::U32StringToU16(
      key.text_.data(), static_cast<uint32_t>(key.text_.length()));
  uint32_t u16_char_map[u16_str.length() + 1];
  auto cfa = GenerateAttributeString(u16_str.data(),
                                     static_cast<uint32_t>(u16_str.length()),
                                     u16_char_map, attributes);
  auto line = CTLineCreateWithAttributedString(cfa);
  CFArrayRef array_ref = CTLineGetGlyphRuns(line);
  auto array_count = CFArrayGetCount(array_ref);

  CTShapingResult ct_result;
  CTFontRef prev_font = nullptr;
  for (auto k = 0; k < array_count; k++) {
    auto run = (CTRunRef)CFArrayGetValueAtIndex(array_ref, k);
    auto run_status = CTRunGetStatus(run);
    bool is_rtl = run_status & kCTRunStatusRightToLeft;
    auto glyph_count = CTRunGetGlyphCount(run);

    CFRange range = CTRunGetStringRange(run);
    auto char32_start = u16_char_map[range.location];
    auto char32_end = u16_char_map[range.location + range.length];
    auto char_count = char32_end - char32_start;
    CTFontRef font = (CTFontRef)CFDictionaryGetValue(
        CTRunGetAttributes(run), (__bridge CFStringRef)NSFontAttributeName);
    if (prev_font == nullptr || font != prev_font) {
      auto typeface = font_collection_.GetDefaultFontManager()
                          ->createTypefaceFromPlatformFont(font);
      ct_font_lst_.push_back(typeface);
      prev_font = font;
    }
    TTASSERT(!ct_font_lst_.empty());

    std::vector<CGGlyph> ct_glyphs(glyph_count);
    std::vector<CGSize> ct_advances(glyph_count);
    std::vector<CFIndex> ct_indices(glyph_count);
    std::vector<CGPoint> ct_position(glyph_count);
    CFRange glyph_range = CFRangeMake(0, glyph_count);
    CTRunGetGlyphs(run, glyph_range, ct_glyphs.data());
    CTRunGetAdvances(run, glyph_range, ct_advances.data());
    CTRunGetStringIndices(run, glyph_range, ct_indices.data());
    CTRunGetPositions(run, glyph_range, ct_position.data());
    if (glyph_count > 1 && is_rtl) {
      std::reverse(ct_glyphs.begin(), ct_glyphs.end());
      std::reverse(ct_advances.begin(), ct_advances.end());
      std::reverse(ct_indices.begin(), ct_indices.end());
      std::reverse(ct_position.begin(), ct_position.end());
    }
    for (auto k = 0; k < glyph_count; k++) {
      ct_indices[k] = u16_char_map[ct_indices[k]];
    }
    /* Copy From hb-coretext.cc */
    if (glyph_count > 1 && (run_status & kCTRunStatusNonMonotonic)) {
      auto cluster = ct_indices[glyph_count - 1];
      for (unsigned int i = static_cast<uint32_t>(glyph_count - 1); i > 0;
           i--) {
        cluster = std::min(cluster, ct_indices[i - 1]);
        ct_indices[i - 1] = cluster;
      }
    }
    ct_result.AppendResult(ct_font_lst_.back(), ct_glyphs.data(),
                           ct_advances.data(), ct_indices.data(),
                           ct_position.data(),
                           static_cast<uint32_t>(glyph_count),
                           static_cast<int32_t>(char_count), is_rtl);
  }
  result->AppendPlatformShapingResult(ct_result);
  CFRelease(attributes);
  CFRelease(cfa);
  CFRelease(line);
}

CFAttributedStringRef ShaperCoreText::GenerateAttributeString(
    const char16_t* content, uint32_t length, uint32_t* u16char_map,
    CFDictionaryRef attribute) const {
  CFStringRef cf_string =
      CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)content,
                              length * 2, kCFStringEncodingUTF16LE, false);
  TTASSERT(length == CFStringGetLength(cf_string));
  auto attr_text =
      CFAttributedStringCreate(kCFAllocatorDefault, cf_string, attribute);

  // Given NSString internally uses utf16 encoding, in the sentence
  // “给，也是在这条街，珍𤨩楼！”, the character *𤨩* will produce a surrogate
  // character, causing the number of characters and the number of internal
  // utf32 characters to be inconsistent, resulting in array out-of-bounds read
  // and write. Here, we first calculate the mapping from utf16 to utf32.
  uint32_t u32_id = 0;
  uint32_t u16_id = 0;
  while (u16_id < length) {
    auto ch = content[u16_id];
    u16char_map[u16_id++] = u32_id;
    if (!ttoffice::base::IsLeadSurrogate(ch)) {
      u32_id++;
    }
  }
  u16char_map[u16_id] = u32_id;
  CFRelease(cf_string);
  return attr_text;
}
static float ConvertFontWeight(FontStyle::Weight weight) {
#ifdef TTTEXT_OS_MAC
  switch (weight) {
    case FontStyle::Weight::kThin_Weight:
      return NSFontWeightUltraLight;
    case FontStyle::Weight::kExtraLight_Weight:
      return NSFontWeightThin;
    case FontStyle::Weight::kLight_Weight:
      return NSFontWeightLight;
    case FontStyle::Weight::kNormal_Weight:
      return NSFontWeightRegular;
    case FontStyle::Weight::kMedium_Weight:
      return NSFontWeightMedium;
    case FontStyle::Weight::kSemiBold_Weight:
      return NSFontWeightSemibold;
    case FontStyle::Weight::kBold_Weight:
      return NSFontWeightBold;
    case FontStyle::Weight::kExtraBold_Weight:
      return NSFontWeightHeavy;
    case FontStyle::Weight::kBlack_Weight:
      return NSFontWeightBlack;
    default:
      return NSFontWeightRegular;
  }
#endif
#ifdef TTTEXT_OS_IOS
  switch (weight) {
    case FontStyle::Weight::kThin_Weight:
      return UIFontWeightUltraLight;
    case FontStyle::Weight::kExtraLight_Weight:
      return UIFontWeightThin;
    case FontStyle::Weight::kLight_Weight:
      return UIFontWeightLight;
    case FontStyle::Weight::kNormal_Weight:
      return UIFontWeightRegular;
    case FontStyle::Weight::kMedium_Weight:
      return UIFontWeightMedium;
    case FontStyle::Weight::kSemiBold_Weight:
      return UIFontWeightSemibold;
    case FontStyle::Weight::kBold_Weight:
      return UIFontWeightBold;
    case FontStyle::Weight::kExtraBold_Weight:
      return UIFontWeightHeavy;
    case FontStyle::Weight::kBlack_Weight:
      return UIFontWeightBlack;
    default:
      return UIFontWeightRegular;
  }
#endif
}

CFDictionaryRef ShaperCoreText::GenerateAttributes(const ShapeKey& key) const {
  CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(
      kCFAllocatorDefault, 5, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);
  auto style = key.style_;
  auto text_size = style.GetFontSize();
  CTFontRef ct_font = nullptr;
  auto& fd = style.GetFontDescriptor();
  auto platform_font = fd.platform_font_;
  if (platform_font != 0) {
#ifdef TTTEXT_OS_MAC
    ct_font = CTFontCreateWithFontDescriptor(CTFontCopyFontDescriptor(ct_font),
                                             text_size, nullptr);
#endif
#ifdef TTTEXT_OS_IOS
    UIFont* ui_font = (__bridge UIFont*)((void*)platform_font);
    if (ui_font != nullptr) {
      ct_font = (__bridge_retained CTFontRef)
          [UIFont fontWithDescriptor:ui_font.fontDescriptor size:text_size];
    }
#endif
  } else if (!fd.font_family_list_.empty()) {
    bool has_valid_font_family = false;
    for (auto& font_family : fd.font_family_list_) {
      if (!font_family.empty()) {
        has_valid_font_family = true;
        break;
      }
    }
    if (has_valid_font_family) {
      auto typefaces = font_collection_.findTypefaces(fd);
      if (typefaces.size() > 0) {
        // We take the first typeface to shape text, but the first one is not
        // always the most suitable. Therefore, we finally iterate all typefaces
        // to find the most suitable one as skia does. If so, we should let
        // ctshaper be a sub class of skshaper so that we could share lots of
        // codes in onelineshaper.
        auto ct_typeface = static_cast<CTFontRef>(
            font_collection_.GetDefaultFontManager()
                ->getPlatformFontFromTypeface(typefaces[0]));
        ct_font = CTFontCreateCopyWithAttributes(ct_typeface, text_size,
                                                 nullptr, nullptr);
      }
    }
  }
  if (ct_font == nullptr) {
    if (default_font_desc_ == nullptr) {
#ifdef TTTEXT_OS_MAC
      //    ct_font =
      //    CTFontCreateWithFontDescriptor(CTFontCopyFontDescriptor(ct_font),
      //                                             text_size, nullptr);
      if (style.FakeBold()) {
        ct_font =
            CTFontCreateWithName(CFSTR("Helvetica-Bold"), text_size, NULL);
      } else if (style.FakeItalic()) {
        ct_font =
            CTFontCreateWithName(CFSTR("Helvetica-Oblique"), text_size, NULL);
      } else {
        ct_font = CTFontCreateWithName(CFSTR("Helvetica"), text_size, NULL);
      }
#endif
#ifdef TTTEXT_OS_IOS
      CGFloat fontWeight = ConvertFontWeight(fd.font_style_.GetWeight());
      if (style.FakeBold()) {
        ct_font = (__bridge CTFontRef)[UIFont boldSystemFontOfSize:text_size];
      } else if (style.FakeItalic()) {
        ct_font = (__bridge CTFontRef)[UIFont italicSystemFontOfSize:text_size];
      } else {
        ct_font = (__bridge CTFontRef)[UIFont systemFontOfSize:text_size
                                                        weight:fontWeight];
      }
#endif
      default_font_desc_ = CTFontCopyFontDescriptor(ct_font);
    }
    ct_font =
        CTFontCreateWithFontDescriptor(default_font_desc_, text_size, NULL);
  }
  CFDictionaryAddValue(attributes, kCTFontAttributeName, ct_font);
  CFRelease(ct_font);
  return attributes;
}
}  // namespace tttext
}  // namespace ttoffice
