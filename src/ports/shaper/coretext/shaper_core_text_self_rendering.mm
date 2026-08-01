// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "shaper_core_text_self_rendering.h"

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

#include "src/textlayout/utils/grapheme_utils.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
std::unordered_map<ShaperCoreTextSelfRendering::CachedSafeFontKey,
                   ShaperCoreTextSelfRendering::CachedSafeFontEntry,
                   ShaperCoreTextSelfRendering::CachedSafeFontKey::Hasher>
    ShaperCoreTextSelfRendering::safe_font_cache_;
std::list<ShaperCoreTextSelfRendering::CachedSafeFontKey>
    ShaperCoreTextSelfRendering::safe_font_cache_lru_;
std::mutex ShaperCoreTextSelfRendering::safe_font_cache_mutex_;
size_t ShaperCoreTextSelfRendering::safe_font_cache_max_entries_ = 256;

namespace {

void AppendSafeFontCascadeDescriptors(CFMutableArrayRef descriptors) {
  const CFStringRef fallback_names[] = {
      CFSTR("PingFang SC"),        // Simplified Chinese
      CFSTR("Apple Color Emoji"),  // Emoji & Symbols
      CFSTR("Thonburi"),           // Thai
      CFSTR("Geeza Pro")           // Arabic
  };
  const CFIndex count = sizeof(fallback_names) / sizeof(fallback_names[0]);
  for (CFIndex i = 0; i < count; ++i) {
    CTFontDescriptorRef descriptor =
        CTFontDescriptorCreateWithNameAndSize(fallback_names[i], 0.0);
    if (descriptor != nullptr) {
      CFArrayAppendValue(descriptors, descriptor);
      CFRelease(descriptor);
    }
  }
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

CTFontDescriptorRef
ShaperCoreTextSelfRendering::CopyFontDescriptorWithoutCascade(
    CTFontDescriptorRef font_descriptor) {
  if (font_descriptor == nullptr) {
    return nullptr;
  }
  CFArrayRef empty_cascade =
      CFArrayCreate(kCFAllocatorDefault, nullptr, 0, &kCFTypeArrayCallBacks);
  const void* keys[] = {kCTFontCascadeListAttribute};
  const void* values[] = {empty_cascade};
  CFDictionaryRef attributes = CFDictionaryCreate(
      kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);
  CTFontDescriptorRef descriptor =
      CTFontDescriptorCreateCopyWithAttributes(font_descriptor, attributes);
  CFRelease(attributes);
  CFRelease(empty_cascade);
  return descriptor;
}

CFArrayRef ShaperCoreTextSelfRendering::CreateOrderedCascadeDescriptors(
    const std::vector<CTFontDescriptorRef>& font_descriptors) {
  CFMutableArrayRef cascade =
      CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
  for (CTFontDescriptorRef font_descriptor : font_descriptors) {
    CTFontDescriptorRef descriptor_without_cascade =
        CopyFontDescriptorWithoutCascade(font_descriptor);
    if (descriptor_without_cascade != nullptr) {
      CFArrayAppendValue(cascade, descriptor_without_cascade);
      CFRelease(descriptor_without_cascade);
    }
  }
  AppendSafeFontCascadeDescriptors(cascade);
  return cascade;
}

bool ShaperCoreTextSelfRendering::FontCoversStringRangeExactly(
    CTFontRef font, CFStringRef string, CFRange range) {
  if (font == nullptr || string == nullptr || range.length <= 0) {
    return false;
  }

  CTFontRef resolved_font = CTFontCreateForString(font, string, range);
  if (resolved_font == nullptr) {
    return false;
  }
  CFStringRef expected_name = CTFontCopyPostScriptName(font);
  CFStringRef resolved_name = CTFontCopyPostScriptName(resolved_font);
  const bool matches = expected_name != nullptr && resolved_name != nullptr &&
                       CFEqual(expected_name, resolved_name);
  if (expected_name != nullptr) {
    CFRelease(expected_name);
  }
  if (resolved_name != nullptr) {
    CFRelease(resolved_name);
  }
  CFRelease(resolved_font);
  return matches;
}

ShaperCoreTextSelfRendering::ShaperCoreTextSelfRendering(
    FontmgrCollection& font_collection) noexcept
    : TTShaper(font_collection){};

ShaperCoreTextSelfRendering::~ShaperCoreTextSelfRendering() = default;

void ShaperCoreTextSelfRendering::SetSafeFontCacheMaxEntries(
    size_t max_entries) {
  std::lock_guard<std::mutex> lock(safe_font_cache_mutex_);
  safe_font_cache_max_entries_ = max_entries;
  TrimSafeFontCacheLocked();
}

size_t ShaperCoreTextSelfRendering::GetSafeFontCacheMaxEntries() {
  std::lock_guard<std::mutex> lock(safe_font_cache_mutex_);
  return safe_font_cache_max_entries_;
}

void ShaperCoreTextSelfRendering::ClearSafeFontCache() {
  std::lock_guard<std::mutex> lock(safe_font_cache_mutex_);
  for (auto& entry : safe_font_cache_) {
    if (entry.second.safe_font_ != nullptr) {
      CFRelease(entry.second.safe_font_);
    }
  }
  safe_font_cache_.clear();
  safe_font_cache_lru_.clear();
}

void ShaperCoreTextSelfRendering::TrimSafeFontCacheLocked() {
  while (safe_font_cache_.size() > safe_font_cache_max_entries_) {
    const auto& oldest_key = safe_font_cache_lru_.back();
    auto iter = safe_font_cache_.find(oldest_key);
    if (iter != safe_font_cache_.end()) {
      if (iter->second.safe_font_ != nullptr) {
        CFRelease(iter->second.safe_font_);
      }
      safe_font_cache_.erase(iter);
    }
    safe_font_cache_lru_.pop_back();
  }
}

CTFontRef ShaperCoreTextSelfRendering::CreateSafeFontWithOrderedFamilyList(
    CTFontRef existing_font,
    const std::vector<TypefaceRef>& resolved_typefaces) const {
  if (existing_font == nullptr) {
    return nullptr;
  }

  std::vector<CTFontDescriptorRef> fallback_descriptors;
  auto font_manager = font_collection_.GetDefaultFontManager();
  if (font_manager != nullptr) {
    fallback_descriptors.reserve(resolved_typefaces.size());
    for (size_t i = 1; i < resolved_typefaces.size(); ++i) {
      CTFontRef font = static_cast<CTFontRef>(
          font_manager->getPlatformFontFromTypeface(resolved_typefaces[i]));
      if (font != nullptr) {
        fallback_descriptors.push_back(CTFontCopyFontDescriptor(font));
      }
    }
  }

  CFArrayRef cascade = CreateOrderedCascadeDescriptors(fallback_descriptors);
  for (CTFontDescriptorRef descriptor : fallback_descriptors) {
    CFRelease(descriptor);
  }

  const void* keys[] = {kCTFontCascadeListAttribute};
  const void* values[] = {cascade};
  CFDictionaryRef cascade_attributes = CFDictionaryCreate(
      kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);

  CTFontDescriptorRef existing_descriptor =
      CTFontCopyFontDescriptor(existing_font);
  CTFontDescriptorRef descriptor_without_cascade =
      CopyFontDescriptorWithoutCascade(existing_descriptor);
  CTFontDescriptorRef ordered_descriptor =
      CTFontDescriptorCreateCopyWithAttributes(descriptor_without_cascade,
                                               cascade_attributes);
  CTFontRef ordered_font = CTFontCreateWithFontDescriptor(
      ordered_descriptor, CTFontGetSize(existing_font), nullptr);

  CFRelease(ordered_descriptor);
  CFRelease(descriptor_without_cascade);
  CFRelease(existing_descriptor);
  CFRelease(cascade_attributes);
  CFRelease(cascade);
  return ordered_font;
}

CTFontRef ShaperCoreTextSelfRendering::GetOrCreateSafeFont(
    const FontDescriptor& fd, float text_size, bool fake_bold,
    bool fake_italic) const {
  CachedSafeFontKey cache_key{fd, text_size, fake_bold, fake_italic};
  {
    std::lock_guard<std::mutex> lock(safe_font_cache_mutex_);
    auto iter = safe_font_cache_.find(cache_key);
    if (iter != safe_font_cache_.end()) {
      safe_font_cache_lru_.splice(safe_font_cache_lru_.begin(),
                                  safe_font_cache_lru_, iter->second.lru_it_);
      iter->second.lru_it_ = safe_font_cache_lru_.begin();
      return static_cast<CTFontRef>(CFRetain(iter->second.safe_font_));
    }
  }

  CTFontRef base_font = nullptr;
  std::vector<TypefaceRef> resolved_typefaces;
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
      resolved_typefaces = font_collection_.findTypefaces(fd);
      if (!resolved_typefaces.empty()) {
        auto ct_typeface = static_cast<CTFontRef>(
            font_collection_.GetDefaultFontManager()
                ->getPlatformFontFromTypeface(resolved_typefaces[0]));
        if (ct_typeface != nullptr) {
          base_font = CTFontCreateCopyWithAttributes(ct_typeface, text_size,
                                                     nullptr, nullptr);
        }
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

  // CSS Fonts 4 requires author-provided families to be visited in declaration
  // order before system fallback:
  // https://www.w3.org/TR/css-fonts-4/#font-matching-algorithm. Blink models
  // that walk in FontFallbackIterator, while WebKit removes nested CoreText
  // cascades before composing its own ordered list (see
  // third_party/blink/renderer/platform/fonts/font_fallback_iterator.cc and
  // Source/WebCore/platform/graphics/cocoa/SystemFontDatabaseCoreText.cpp).
  // A descriptor with zero or one resolved family naturally produces only the
  // existing safe fallback list, so ordered matching does not need a separate
  // legacy branch.
  CTFontRef safe_font =
      CreateSafeFontWithOrderedFamilyList(base_font, resolved_typefaces);
  CFRelease(base_font);
  if (safe_font == nullptr) {
    return nullptr;
  }

  std::lock_guard<std::mutex> lock(safe_font_cache_mutex_);
  auto iter = safe_font_cache_.find(cache_key);
  if (iter != safe_font_cache_.end()) {
    CFRelease(safe_font);
    safe_font_cache_lru_.splice(safe_font_cache_lru_.begin(),
                                safe_font_cache_lru_, iter->second.lru_it_);
    iter->second.lru_it_ = safe_font_cache_lru_.begin();
    return static_cast<CTFontRef>(CFRetain(iter->second.safe_font_));
  }

  if (safe_font_cache_max_entries_ == 0) {
    return safe_font;
  }

  safe_font_cache_lru_.push_front(cache_key);
  safe_font_cache_.emplace(
      cache_key, CachedSafeFontEntry{safe_font, safe_font_cache_lru_.begin()});
  TrimSafeFontCacheLocked();
  return static_cast<CTFontRef>(CFRetain(safe_font));
}

void ShaperCoreTextSelfRendering::ProcessBidirection(
    const char32_t* text, uint32_t length, WriteDirection write_direction,
    uint32_t* visual_map, uint32_t* logical_map, uint8_t* dir_vec) {
  auto u16_str = ttoffice::base::U32StringToU16(text, length);
  uint32_t* u16_to_u32_map = tmp_buf_;
  if (u16_str.length() + 1 > SELF_RENDERING_SHAPER_BUFF_SIZE) {
    u16_to_u32_map = new uint32_t[u16_str.length() + 1];
  }
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
  // --- FIX START ---
  // Force inject a safe font into the dictionary.
  // This ensures CTLineCreate has a localized fallback path.
  FontDescriptor default_fd;
  CTFontRef safe_font = GetOrCreateSafeFont(default_fd, 12.0, false, false);
  if (safe_font != nullptr) {
    CFDictionaryAddValue(dict, kCTFontAttributeName, safe_font);
    CFRelease(safe_font);
  }
  // --- FIX END ---

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
  if (u16_to_u32_map != tmp_buf_) {
    delete[] u16_to_u32_map;
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
void ShaperCoreTextSelfRendering::OnShapeText(const ShapeKey& key,
                                              ShapeResult* result) const {
  CFDictionaryRef attributes = GenerateAttributes(key);

  // CoreText returns platform fonts for each run. Preserve the exact
  // TypefaceRef resolved by this shape's FontmgrCollection whenever the run
  // uses one of the declared faces. This mapping is deliberately local to the
  // shape: custom managers can be app-owned even though CoreText font
  // registration and Textra's caches are process-wide. Unmatched runs remain
  // platform fallback and continue through the default-manager path.
  std::vector<TypefaceRef> resolved_typefaces;
  std::vector<std::pair<TypefaceRef, CFStringRef>> ordered_typefaces;
  auto default_font_manager = font_collection_.GetDefaultFontManager();
  if (default_font_manager != nullptr &&
      !key.style_.GetFontDescriptor().font_family_list_.empty()) {
    resolved_typefaces =
        font_collection_.findTypefaces(key.style_.GetFontDescriptor());
    for (const TypefaceRef& typeface : resolved_typefaces) {
      CTFontRef font = static_cast<CTFontRef>(
          default_font_manager->getPlatformFontFromTypeface(typeface));
      if (font != nullptr) {
        CFStringRef postscript_name = CTFontCopyPostScriptName(font);
        if (postscript_name != nullptr) {
          ordered_typefaces.emplace_back(typeface, postscript_name);
        }
      }
    }
  }

  auto u16_str = base::U32StringToU16(
      key.text_.data(), static_cast<uint32_t>(key.text_.length()));
  uint32_t* u16_char_map = tmp_buf_;
  if (u16_str.length() + 1 > SELF_RENDERING_SHAPER_BUFF_SIZE) {
    u16_char_map = new uint32_t[u16_str.length() + 1];
  }
  CFAttributedStringRef cfa = GenerateAttributeString(
      u16_str.data(), static_cast<uint32_t>(u16_str.length()), u16_char_map,
      attributes);
  if (resolved_typefaces.size() > 1) {
    CFMutableAttributedStringRef ordered_cfa =
        CFAttributedStringCreateMutableCopy(kCFAllocatorDefault, 0, cfa);
    ApplyOrderedFamilyFonts(ordered_cfa, key.style_.GetFontSize(),
                            resolved_typefaces);
    CFRelease(cfa);
    cfa = ordered_cfa;
  }
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
      TypefaceRef typeface;
      if (!ordered_typefaces.empty() && font != nullptr) {
        CFStringRef postscript_name = CTFontCopyPostScriptName(font);
        if (postscript_name != nullptr) {
          for (const auto& ordered_typeface : ordered_typefaces) {
            if (CFEqual(postscript_name, ordered_typeface.second)) {
              typeface = ordered_typeface.first;
              break;
            }
          }
          CFRelease(postscript_name);
        }
      }
      if (typeface == nullptr) {
        typeface = default_font_manager->createTypefaceFromPlatformFont(font);
      }
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
  for (const auto& ordered_typeface : ordered_typefaces) {
    CFRelease(ordered_typeface.second);
  }
}

void ShaperCoreTextSelfRendering::ApplyOrderedFamilyFonts(
    CFMutableAttributedStringRef attributed_string, float text_size,
    const std::vector<TypefaceRef>& resolved_typefaces) const {
  auto font_manager = font_collection_.GetDefaultFontManager();
  if (attributed_string == nullptr || font_manager == nullptr ||
      resolved_typefaces.size() < 2) {
    return;
  }

  std::vector<CTFontRef> fonts;
  fonts.reserve(resolved_typefaces.size());
  for (const TypefaceRef& typeface : resolved_typefaces) {
    CTFontRef platform_font = static_cast<CTFontRef>(
        font_manager->getPlatformFontFromTypeface(typeface));
    fonts.push_back(platform_font == nullptr
                        ? nullptr
                        : CTFontCreateCopyWithAttributes(
                              platform_font, text_size, nullptr, nullptr));
  }

  CFStringRef cf_string = CFAttributedStringGetString(attributed_string);
  NSString* string = (__bridge NSString*)cf_string;
  for (NSUInteger index = 0; index < string.length;) {
    NSRange cluster = [string rangeOfComposedCharacterSequenceAtIndex:index];
    CFRange cluster_range = CFRangeMake(static_cast<CFIndex>(cluster.location),
                                        static_cast<CFIndex>(cluster.length));
    for (size_t font_index = 0; font_index < fonts.size(); ++font_index) {
      CTFontRef font = fonts[font_index];
      if (!FontCoversStringRangeExactly(font, cf_string, cluster_range)) {
        continue;
      }
      // The base face is already installed on the full attributed string.
      // Bind only a later explicit family; if every declared face misses, the
      // existing CoreText cascade remains responsible for system fallback.
      if (font_index > 0) {
        CFAttributedStringSetAttribute(attributed_string, cluster_range,
                                       kCTFontAttributeName, font);
      }
      break;
    }
    index = NSMaxRange(cluster);
  }

  for (CTFontRef font : fonts) {
    if (font != nullptr) {
      CFRelease(font);
    }
  }
}

CFAttributedStringRef ShaperCoreTextSelfRendering::GenerateAttributeString(
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
CFDictionaryRef ShaperCoreTextSelfRendering::GenerateAttributes(
    const ShapeKey& key) const {
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
