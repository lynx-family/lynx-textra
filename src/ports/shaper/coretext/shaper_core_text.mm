// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "shaper_core_text.h"

#ifndef MIN
#import <objc/NSObjCRuntime.h>
#endif

#if TARGET_OS_IOS
#define USE_UIKIT
#elif TARGET_OS_MAC
#define USE_APPKIT
#endif

#ifdef USE_UIKIT
#import <UIKit/UIKit.h>
#endif

#ifdef USE_APPKIT
#import <AppKit/AppKit.h>
#endif

#include <textra/font_info.h>
#include <textra/fontmgr_collection.h>
#include <textra/i_canvas_helper.h>
#include <textra/style.h>

#include <algorithm>

#include "src/textlayout/icu_substitute/bidi/bidi_wrapper.h"
#include "src/textlayout/tttext_context_impl.h"
#include "src/textlayout/utils/grapheme_utils.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
namespace {

CFDictionaryRef GetSafeFontCascadeAttributes() {
  static CFDictionaryRef attributes = []() -> CFDictionaryRef {
    const CFStringRef fallback_names[] = {
        CFSTR("PingFang SC"),        // Simplified Chinese
        CFSTR("Apple Color Emoji"),  // Emoji & Symbols
        CFSTR("Thonburi"),           // Thai
        CFSTR("Geeza Pro")           // Arabic
    };
    const CFIndex count = sizeof(fallback_names) / sizeof(fallback_names[0]);

    CFMutableArrayRef fallback_descriptors = CFArrayCreateMutable(
        kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
    for (CFIndex i = 0; i < count; ++i) {
      CTFontDescriptorRef desc =
          CTFontDescriptorCreateWithNameAndSize(fallback_names[i], 0.0);
      if (desc != nullptr) {
        CFArrayAppendValue(fallback_descriptors, desc);
        CFRelease(desc);
      }
    }

    CFStringRef keys[] = {kCTFontCascadeListAttribute};
    CFTypeRef values[] = {fallback_descriptors};
    CFDictionaryRef cached_attributes = CFDictionaryCreate(
        kCFAllocatorDefault, (const void**)keys, (const void**)values, 1,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(fallback_descriptors);
    return cached_attributes;
  }();

  return attributes;
}

static float ConvertFontWeight(tttext::Weight weight) {
#ifdef USE_APPKIT
  switch (weight) {
    case FontStyle::kThin_Weight:
      return NSFontWeightUltraLight;
    case FontStyle::kExtraLight_Weight:
      return NSFontWeightThin;
    case FontStyle::kLight_Weight:
      return NSFontWeightLight;
    case FontStyle::kNormal_Weight:
      return NSFontWeightRegular;
    case FontStyle::kMedium_Weight:
      return NSFontWeightMedium;
    case FontStyle::kSemiBold_Weight:
      return NSFontWeightSemibold;
    case FontStyle::kBold_Weight:
      return NSFontWeightBold;
    case FontStyle::kExtraBold_Weight:
      return NSFontWeightHeavy;
    case FontStyle::kBlack_Weight:
      return NSFontWeightBlack;
    default:
      return NSFontWeightRegular;
  }
#endif
#ifdef USE_UIKIT
  switch (weight) {
    case FontStyle::kThin_Weight:
      return UIFontWeightUltraLight;
    case FontStyle::kExtraLight_Weight:
      return UIFontWeightThin;
    case FontStyle::kLight_Weight:
      return UIFontWeightLight;
    case FontStyle::kNormal_Weight:
      return UIFontWeightRegular;
    case FontStyle::kMedium_Weight:
      return UIFontWeightMedium;
    case FontStyle::kSemiBold_Weight:
      return UIFontWeightSemibold;
    case FontStyle::kBold_Weight:
      return UIFontWeightBold;
    case FontStyle::kExtraBold_Weight:
      return UIFontWeightHeavy;
    case FontStyle::kBlack_Weight:
      return UIFontWeightBlack;
    default:
      return UIFontWeightRegular;
  }
#endif
}

}  // namespace

ShaperCoreText::ShaperCoreText(FontmgrCollection& font_collection) noexcept
    : TTShaper(font_collection){};

ShaperCoreText::~ShaperCoreText() = default;

ShaperCoreText::SafeFontCacheType& ShaperCoreText::GetGlobalSafeFontCache() {
  // This cache can be reached from app-load warm-up work before namespace-scope
  // C++ objects in this translation unit have finished initialization. Create
  // the whole state on first use and intentionally keep it for process life so
  // neither startup nor shutdown ordering can invalidate its containers.
  static SafeFontCacheType* cache = new SafeFontCacheType();
  return *cache;
}

void ShaperCoreText::SetSafeFontCacheMaxEntries(size_t max_entries) {
  GetGlobalSafeFontCache().SetMaxEntries(max_entries);
}

size_t ShaperCoreText::GetSafeFontCacheMaxEntries() {
  return GetGlobalSafeFontCache().GetMaxEntries();
}

void ShaperCoreText::ClearSafeFontCache() { GetGlobalSafeFontCache().Clear(); }

/**
 * Creates a "Safe Font" by injecting a mandatory Font Cascade List.
 * This bypasses the global (and thread-unsafe) _CTFontFallbacksArray in
 * CoreText.
 */
CTFontRef ShaperCoreText::CreateSafeFontUnified(CTFontRef existing_font) const {
  if (!existing_font) return nullptr;

  CGFloat font_size = CTFontGetSize(existing_font);
  CTFontDescriptorRef current_desc = CTFontCopyFontDescriptor(existing_font);
  CTFontDescriptorRef safe_desc = CTFontDescriptorCreateCopyWithAttributes(
      current_desc, GetSafeFontCascadeAttributes());
  CTFontRef safe_font =
      CTFontCreateWithFontDescriptor(safe_desc, font_size, nullptr);

  CFRelease(current_desc);
  CFRelease(safe_desc);
  return safe_font;
}

CTFontRef ShaperCoreText::GetOrCreateSafeFont(const FontDescriptor& fd,
                                              float text_size, bool fake_bold,
                                              bool fake_italic) const {
  CachedSafeFontKey cache_key{fd, text_size, fake_bold, fake_italic};
  const auto scope =
      context_ == nullptr ? nullptr : context_->GetImpl().GetShapeCacheScope();
  const auto route = ShapeCacheScopeInternal::CaptureRoute(scope, fd);
  SafeFontCacheType* cache = &GetGlobalSafeFontCache();
  std::shared_ptr<ShapeCachePlatformState> scoped_state;
  static const int kScopedSafeFontCacheIdentity = 0;
  if (route.use_scoped_cache) {
    scoped_state = ShapeCacheScopeInternal::GetOrCreatePlatformState(
        route, &kScopedSafeFontCacheIdentity, []() {
          return std::make_shared<SafeFontCacheType>(
              GetGlobalSafeFontCache().GetMaxEntries());
        });
    cache = scoped_state == nullptr
                ? nullptr
                : static_cast<SafeFontCacheType*>(scoped_state.get());
  }
  SafeFontCacheType::Epoch cache_epoch = 0;
  CTFontRef cached_font = nullptr;
  if (cache != nullptr) {
    ShapeCacheScopeInternal::RunIfCurrent(
        route, [&]() { cached_font = cache->Find(cache_key, &cache_epoch); });
  }
  if (cached_font != nullptr) {
    return cached_font;
  }

  CTFontRef base_font = nullptr;
  if (fd.platform_font_ != 0) {
#ifdef USE_APPKIT
    CTFontRef platform_font =
        reinterpret_cast<CTFontRef>(static_cast<uintptr_t>(fd.platform_font_));
    if (platform_font != nullptr) {
      base_font = CTFontCreateCopyWithAttributes(platform_font, text_size,
                                                 nullptr, nullptr);
    }
#endif
#ifdef USE_UIKIT
    UIFont* ui_font = (__bridge UIFont*)((void*)fd.platform_font_);
    if (ui_font != nullptr) {
      base_font = (__bridge_retained CTFontRef)
          [UIFont fontWithDescriptor:ui_font.fontDescriptor size:text_size];
    }
#endif
  } else if (!fd.font_family_list_.empty()) {
    bool has_valid_font_family = false;
    for (const auto& font_family : fd.font_family_list_) {
      if (!font_family.empty()) {
        has_valid_font_family = true;
        break;
      }
    }
    if (has_valid_font_family) {
      auto typefaces = font_collection_.findTypefaces(fd);
      if (!typefaces.empty()) {
        auto ct_typeface = static_cast<CTFontRef>(
            font_collection_.GetDefaultFontManager()
                ->getPlatformFontFromTypeface(typefaces[0]));
        base_font = CTFontCreateCopyWithAttributes(ct_typeface, text_size,
                                                   nullptr, nullptr);
      }
    }
  }

  if (base_font == nullptr) {
#ifdef USE_APPKIT
    if (fake_bold) {
      base_font =
          CTFontCreateWithName(CFSTR("Helvetica-Bold"), text_size, NULL);
    } else if (fake_italic) {
      base_font =
          CTFontCreateWithName(CFSTR("Helvetica-Oblique"), text_size, NULL);
    } else {
      base_font = CTFontCreateWithName(CFSTR("Helvetica"), text_size, NULL);
    }
#endif
#ifdef USE_UIKIT
    CGFloat font_weight = ConvertFontWeight(fd.font_style_.GetWeight());
    if (fake_bold) {
      base_font =
          (__bridge_retained CTFontRef)[UIFont boldSystemFontOfSize:text_size];
    } else if (fake_italic) {
      base_font = (__bridge_retained CTFontRef)
          [UIFont italicSystemFontOfSize:text_size];
    } else {
      base_font =
          (__bridge_retained CTFontRef)[UIFont systemFontOfSize:text_size
                                                         weight:font_weight];
    }
#endif
  }

  if (base_font == nullptr) {
    return nullptr;
  }

  CTFontRef safe_font = CreateSafeFontUnified(base_font);
  CFRelease(base_font);
  if (safe_font == nullptr) {
    return nullptr;
  }

  CTFontRef result = safe_font;
  if (cache != nullptr) {
    ShapeCacheScopeInternal::RunIfCurrent(route, [&]() {
      result = cache->Add(cache_key, safe_font, cache_epoch);
    });
  }
  return result;
}

void ShaperCoreText::ProcessBidirection(const char32_t* text, uint32_t length,
                                        WriteDirection write_direction,
                                        uint32_t* visual_map,
                                        uint32_t* logical_map,
                                        uint8_t* dir_vec) {
  BidiWrapper::GetInstance().SetPara(text, length, write_direction, dir_vec,
                                     visual_map, logical_map);
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
      ct_position_.insert(ct_position_.begin(), position,
                          position + glyph_count);
    } else {
      ct_glyphs_.insert(ct_glyphs_.end(), glyphs, glyphs + glyph_count);
      ct_advances_.insert(ct_advances_.end(), advances, advances + glyph_count);
      ct_indices_.insert(ct_indices_.end(), indices, indices + glyph_count);
      typeface_.insert(typeface_.end(), glyph_count, typeface);
      ct_position_.insert(ct_position_.end(), position, position + glyph_count);
    }
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
  uint32_t* u16_char_map = tmp_buf_;
  if (u16_str.length() + 1 > SHAPER_BUFF_SIZE) {
    u16_char_map = new uint32_t[u16_str.length() + 1];
  }
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
      auto iter = ct_advances.rbegin();
      while (iter != ct_advances.rend()) {
        if (iter->width < 0 && iter != ct_advances.rbegin()) {
          (iter - 1)->width += iter->width;
        }
        iter++;
      }
    }
    for (auto k = 0; k < glyph_count; k++) {
      ct_indices[k] = u16_char_map[ct_indices[k]];
      if (ct_indices[k] >= 0 &&
          static_cast<size_t>(ct_indices[k]) < key.text_.length() &&
          IsZeroWidthJoiner(key.text_[static_cast<size_t>(ct_indices[k])])) {
        ct_advances[k].width = 0;
        ct_advances[k].height = 0;
      }
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
  if (u16_char_map != tmp_buf_) {
    delete[] u16_char_map;
  }
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
CFDictionaryRef ShaperCoreText::GenerateAttributes(const ShapeKey& key) const {
  CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(
      kCFAllocatorDefault, 5, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);
  auto style = key.style_;
  auto text_size = style.GetFontSize();
  auto& fd = style.GetFontDescriptor();
  CTFontRef safe_font =
      GetOrCreateSafeFont(fd, text_size, style.FakeBold(), style.FakeItalic());
  if (safe_font != nullptr) {
    CFDictionaryAddValue(attributes, kCTFontAttributeName, safe_font);
    CFRelease(safe_font);
  }
  return attributes;
}
}  // namespace tttext
}  // namespace ttoffice
#undef USE_UIKIT
#undef USE_APPKIT
