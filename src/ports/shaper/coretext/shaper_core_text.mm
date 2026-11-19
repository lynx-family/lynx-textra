// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "shaper_core_text.h"

#include "font_feature_settings.h"
#include "shape_text_encoding.h"

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
#include <array>
#include <cmath>
#include <mutex>

#include "src/textlayout/icu_substitute/bidi/bidi_wrapper.h"
#include "src/textlayout/utils/grapheme_utils.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
namespace {
#ifdef USE_APPKIT
CFDictionaryRef GetLegacyFontCascadeAttributes() {
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
#endif

#ifdef USE_UIKIT
enum class DefaultFontKind : size_t {
  kChinese = 0,
  kEmoji,
  kThai,
  kArabic,
  kSystem,
  kFallback,
  kInherited,
};

DefaultFontKind ClassifyDefaultFontCodepoint(UnicodeProperties properties) {
  if (properties.Has(UnicodeProperties::kBlockBasicLatin))
    return DefaultFontKind::kSystem;
  if (properties.IsCJKIdeograph() || properties.IsCJKPunctuation()) {
    return DefaultFontKind::kChinese;
  }
  if (properties.Has(UnicodeProperties::kScriptThai))
    return DefaultFontKind::kThai;
  if (properties.Has(UnicodeProperties::kScriptArabic))
    return DefaultFontKind::kArabic;
  return DefaultFontKind::kFallback;
}

// Classify a complete grapheme using emoji presentation before script ranges.
DefaultFontKind ClassifyDefaultFontCluster(
    const char32_t* text, uint32_t start, uint32_t end,
    const base::UnicodeProperties* cached) {
  const auto cluster = GetEmojiClusterInfo(text, start, end, cached);
  if (cluster.presentation == EmojiPresentation::kText) {
    return DefaultFontKind::kFallback;
  }
  if (cluster.presentation == EmojiPresentation::kEmoji) {
    return DefaultFontKind::kEmoji;
  }
  auto kind = DefaultFontKind::kInherited;
  for (auto i = start; i < end; ++i) {
    const char32_t ch = text[i];
    if (IsZeroWidthJoiner(ch) || IsZeroWidthNonJoiner(ch)) {
      continue;
    }
    const auto properties = base::GetUnicodePropertiesAt(cached, i, ch);
    if (properties.Has(UnicodeProperties::kGeneralCategoryMark)) continue;
    const auto current = ClassifyDefaultFontCodepoint(properties);
    if (kind == DefaultFontKind::kInherited) {
      kind = current;
    } else if (kind != current) {
      return DefaultFontKind::kFallback;
    }
  }
  return kind;
}

// A finite set of system weights, independent of caller UIFont instances and
// text sizes. The table is initialized once and shared by cascade and active
// CJK fallback. Size 14 is only used to obtain system UI descriptors; CoreText
// inherits the actual size (including Text/Display selection) from the parent.
const std::array<CGFloat, 9>& GetUIFontWeights() {
  static const std::array<CGFloat, 9> weights = {
      UIFontWeightUltraLight, UIFontWeightThin,   UIFontWeightLight,
      UIFontWeightRegular,    UIFontWeightMedium, UIFontWeightSemibold,
      UIFontWeightBold,       UIFontWeightHeavy,  UIFontWeightBlack};
  return weights;
}

size_t GetUIFontWeightIndex(CTFontRef font) {
  CGFloat weight = UIFontWeightRegular;
  CFDictionaryRef traits = CTFontCopyTraits(font);
  if (traits) {
    auto value = static_cast<CFNumberRef>(
        CFDictionaryGetValue(traits, kCTFontWeightTrait));
    if (value) CFNumberGetValue(value, kCFNumberCGFloatType, &weight);
    CFRelease(traits);
  }
  const auto& weights = GetUIFontWeights();
  size_t closest = 3;
  CGFloat distance = std::abs(weight - weights[closest]);
  for (size_t i = 0; i < weights.size(); ++i) {
    CGFloat candidate = std::abs(weight - weights[i]);
    if (candidate < distance) {
      closest = i;
      distance = candidate;
    }
  }
  return closest;
}

class SystemUIFontFallbacks {
 public:
  SystemUIFontFallbacks(const SystemUIFontFallbacks&) = delete;
  SystemUIFontFallbacks& operator=(const SystemUIFontFallbacks&) = delete;

  static const SystemUIFontFallbacks& GetInstance() {
    // Process lifetime: 9 system UI fonts and 9 x 4 fallback fonts, initialized
    // together by the first caller (normally TextService's generic background
    // warm).
    static const auto* instance = new SystemUIFontFallbacks();
    return *instance;
  }

  CFDictionaryRef Attributes(size_t weight_index) const {
    return entries_[weight_index].attributes;
  }
  // Returns a retained font. Sized derivatives are bounded per calling thread.
  CTFontRef CopyFont(DefaultFontKind kind, size_t weight, CGFloat size) const;

 private:
  struct Entry {
    CTFontRef system_font = nullptr;
    std::array<CTFontRef, 4> fonts{};
    CFDictionaryRef attributes = nullptr;
  };

  SystemUIFontFallbacks() {
    for (size_t weight = 0; weight < entries_.size(); ++weight) {
      @autoreleasepool {
        auto& entry = entries_[weight];
        UIFont* system_font =
            [UIFont systemFontOfSize:14 weight:GetUIFontWeights()[weight]];
        entry.system_font =
            static_cast<CTFontRef>(CFRetain((__bridge CTFontRef)system_font));
        const CFStringRef samples[] = {CFSTR("\u4E2D"), CFSTR("\U0001F600"),
                                       CFSTR("\u0E01"), CFSTR("\u0627")};
        CFMutableArrayRef descriptors =
            CFArrayCreateMutable(nullptr, 4, &kCFTypeArrayCallBacks);
        if (!descriptors) continue;
        for (size_t script = 0; script < entry.fonts.size(); ++script) {
          CFStringRef sample = samples[script];
          entry.fonts[script] =
              CTFontCreateForString((__bridge CTFontRef)system_font, sample,
                                    CFRangeMake(0, CFStringGetLength(sample)));
          CTFontDescriptorRef descriptor =
              entry.fonts[script]
                  ? CTFontCopyFontDescriptor(entry.fonts[script])
                  : nullptr;
          if (!descriptor) break;
          CFArrayAppendValue(descriptors, descriptor);
          CFRelease(descriptor);
        }
        if (CFArrayGetCount(descriptors) == 4) {
          const void* keys[] = {kCTFontCascadeListAttribute};
          const void* values[] = {descriptors};
          entry.attributes = CFDictionaryCreate(
              nullptr, keys, values, 1, &kCFTypeDictionaryKeyCallBacks,
              &kCFTypeDictionaryValueCallBacks);
        }
        CFRelease(descriptors);
        if (!entry.attributes) {
          for (auto& font : entry.fonts) {
            if (font) CFRelease(font);
            font = nullptr;
          }
        }
      }
    }
  }
  std::array<Entry, 9> entries_{};
};

CFDictionaryRef GetSystemUIFontCascadeAttributes(size_t weight_index) {
  return SystemUIFontFallbacks::GetInstance().Attributes(weight_index);
}

CTFontRef SystemUIFontFallbacks::CopyFont(DefaultFontKind kind, size_t weight,
                                          CGFloat size) const {
  TTASSERT(kind < DefaultFontKind::kInherited && weight < entries_.size());
  TTASSERT(size > 0 && std::isfinite(size));
  const auto& entry = entries_[weight];
  const size_t script = static_cast<size_t>(kind);
  CTFontRef base =
      script < entry.fonts.size() ? entry.fonts[script] : entry.system_font;
  if (!base) {
    base = entry.system_font;
    kind = DefaultFontKind::kFallback;
  }
  if (!base) return nullptr;
  if (kind != DefaultFontKind::kFallback && size == CTFontGetSize(base)) {
    return static_cast<CTFontRef>(CFRetain(base));
  }
  struct SizedFont {
    DefaultFontKind kind = DefaultFontKind::kSystem;
    size_t weight = 0;
    CGFloat size = 0;
    CTFontRef font = nullptr;
  };
  struct SizedFonts {
    std::array<SizedFont, 32> entries{};
    size_t next = 0;
    ~SizedFonts() {
      for (auto& entry : entries) {
        if (entry.font) CFRelease(entry.font);
      }
    }
  };
  // No shared cache lock on the default-font shaping path. Retained results
  // remain valid when this thread evicts an entry or exits.
  static thread_local SizedFonts cache;
  for (const auto& cached : cache.entries) {
    if (cached.font && cached.kind == kind && cached.weight == weight &&
        cached.size == size) {
      return static_cast<CTFontRef>(CFRetain(cached.font));
    }
  }
  CTFontRef font = nullptr;
  if (kind == DefaultFontKind::kFallback && entry.attributes) {
    auto descriptor = CTFontCopyFontDescriptor(base);
    auto safe_descriptor =
        CTFontDescriptorCreateCopyWithAttributes(descriptor, entry.attributes);
    font = CTFontCreateWithFontDescriptor(safe_descriptor, size, nullptr);
    CFRelease(safe_descriptor);
    CFRelease(descriptor);
  } else {
    font = CTFontCreateCopyWithAttributes(base, size, nullptr, nullptr);
  }
  if (!font) return nullptr;
  auto& cached = cache.entries[cache.next];
  if (cached.font) CFRelease(cached.font);
  cached = {kind, weight, size, font};
  cache.next = (cache.next + 1) % cache.entries.size();
  return static_cast<CTFontRef>(CFRetain(font));
}

bool HasExplicitFont(const FontDescriptor& fd) {
  return fd.platform_font_ != 0 ||
         std::any_of(fd.font_family_list_.begin(), fd.font_family_list_.end(),
                     [](const std::string& family) { return !family.empty(); });
}

bool UseDefaultFontPool(const FontDescriptor& fd, float size) {
  return fd.font_feature_settings_.empty() && !HasExplicitFont(fd) &&
         size > 0 && std::isfinite(size) &&
         fd.font_style_.GetSlant() == FontStyle::kUpright_Slant &&
         fd.font_style_.GetWidth() == FontStyle::kNormal_Width;
}

size_t GetDefaultUIFontWeightIndex(const FontDescriptor& fd) {
  const auto weight = fd.font_style_.GetWeight();
  // Preserve regular weight for unsupported numeric weights.
  return weight >= 100 && weight <= 900 && weight % 100 == 0 ? weight / 100 - 1
                                                             : 3;
}

CFAttributedStringRef CreateStringWithDefaultFonts(
    CFStringRef text, const EncodedShapeText& encoded,
    CFDictionaryRef attributes, const FontDescriptor& fd, float size,
    const base::UnicodeProperties* cached) {
  // Explicit fonts (including system UIFont handles) and special styles have
  // already been resolved. Do not rewrite any of their font attributes.
  if (!UseDefaultFontPool(fd, size)) {
    return CFAttributedStringCreate(nullptr, text, attributes);
  }
  const auto length = static_cast<CFIndex>(encoded.Utf16().size());
  auto result = CFAttributedStringCreateMutable(nullptr, 0);
  if (!result) return nullptr;
  CFAttributedStringReplaceString(result, CFRangeMake(0, 0), text);
  CFAttributedStringSetAttributes(result, CFRangeMake(0, length), attributes,
                                  false);
  const auto& pool = SystemUIFontFallbacks::GetInstance();
  const size_t weight = GetDefaultUIFontWeightIndex(fd);
  std::array<CTFontRef, 6> fonts{};
  auto apply = [&](DefaultFontKind kind, CFIndex start, CFIndex end) {
    auto index = static_cast<size_t>(kind);
    if (!fonts[index]) fonts[index] = pool.CopyFont(kind, weight, size);
    if (!fonts[index]) return false;
    CFAttributedStringSetAttribute(result, CFRangeMake(start, end - start),
                                   kCTFontAttributeName, fonts[index]);
    return true;
  };
  CFIndex start = 0;
  CFIndex offset = 0;
  auto previous = DefaultFontKind::kFallback;
  bool complete = true;
  if (encoded.IsAscii()) {
    previous = DefaultFontKind::kSystem;
    offset = length;
  }
  while (offset < length) {
    // CoreFoundation handles surrogate pairs, emoji ZWJ/tag/keycap sequences,
    // and combining marks. Never introduce a font boundary inside a grapheme.
    auto range = CFStringGetRangeOfComposedCharactersAtIndex(text, offset);
    auto end = range.location + range.length;
    TTASSERT(range.location == offset && end > offset && end <= length);
    const auto start32 = encoded.Map()[offset];
    const auto end32 = encoded.Map()[end];
    auto kind = ClassifyDefaultFontCluster(encoded.Text().data(), start32,
                                           end32, cached);
    if (kind == DefaultFontKind::kInherited) kind = previous;
    if (offset > start && kind != previous) {
      if (!apply(previous, start, offset)) {
        complete = false;
        break;
      }
      start = offset;
    }
    previous = kind;
    offset = end;
  }
  if (complete && length > 0) complete = apply(previous, start, length);
  if (!complete) {
    // Resolve the entire text uniformly if a script font cannot be created.
    // Partially assigned font attributes must not leak into shaping.
    complete = apply(DefaultFontKind::kFallback, 0, length);
  }
  for (auto font : fonts) {
    if (font) CFRelease(font);
  }
  if (!complete) {
    CFRelease(result);
    return nullptr;
  }
  return result;
}

#endif

CFDictionaryRef GetSafeFontCascadeAttributes(CTFontRef font) {
#ifdef USE_UIKIT
  return GetSystemUIFontCascadeAttributes(GetUIFontWeightIndex(font));
#elif defined(USE_APPKIT)
  (void)font;
  return GetLegacyFontCascadeAttributes();
#endif
}

}  // namespace

ShaperCoreText::ShaperCoreText(FontmgrCollection& font_collection) noexcept
    : TTShaper(font_collection){};

ShaperCoreText::~ShaperCoreText() = default;

ShaperCoreText::SafeFontCacheState& ShaperCoreText::GetSafeFontCacheState() {
  // This cache can be reached from app-load warm-up work before namespace-scope
  // C++ objects in this translation unit have finished initialization. Create
  // the whole state on first use and intentionally keep it for process life so
  // neither startup nor shutdown ordering can invalidate its containers.
  static SafeFontCacheState* state = new SafeFontCacheState();
  return *state;
}

void ShaperCoreText::SetSafeFontCacheMaxEntries(size_t max_entries) {
  auto& state = GetSafeFontCacheState();
  std::lock_guard<std::mutex> lock(state.mutex_);
  state.max_entries_ = max_entries;
  TrimSafeFontCacheLocked(state);
}

size_t ShaperCoreText::GetSafeFontCacheMaxEntries() {
  auto& state = GetSafeFontCacheState();
  std::lock_guard<std::mutex> lock(state.mutex_);
  return state.max_entries_;
}

void ShaperCoreText::TrimSafeFontCacheLocked(SafeFontCacheState& state) {
  while (state.cache_.size() > state.max_entries_) {
    const auto& oldest_key = state.lru_.back();
    auto iter = state.cache_.find(oldest_key);
    if (iter != state.cache_.end()) {
      if (iter->second.safe_font_ != nullptr) {
        CFRelease(iter->second.safe_font_);
      }
      state.cache_.erase(iter);
    }
    state.lru_.pop_back();
  }
}

CTFontRef ShaperCoreText::CreateSafeFontUnified(
    CTFontRef existing_font, const FontDescriptor& fd) const {
  TTASSERT(existing_font != nullptr);
  CFDictionaryRef attributes = GetSafeFontCascadeAttributes(existing_font);
  CFMutableDictionaryRef feature_attributes = nullptr;
  if (!fd.font_feature_settings_.empty()) {
    feature_attributes =
        attributes
            ? CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attributes)
            : CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                        &kCFTypeDictionaryKeyCallBacks,
                                        &kCFTypeDictionaryValueCallBacks);
    AddFontFeatureSettings(feature_attributes, fd);
    attributes = feature_attributes;
  } else if (!attributes) {
    return static_cast<CTFontRef>(CFRetain(existing_font));
  }

  CGFloat font_size = CTFontGetSize(existing_font);
  CTFontDescriptorRef current_desc = CTFontCopyFontDescriptor(existing_font);
  CTFontDescriptorRef safe_desc =
      CTFontDescriptorCreateCopyWithAttributes(current_desc, attributes);
  CTFontRef safe_font =
      CTFontCreateWithFontDescriptor(safe_desc, font_size, nullptr);

  CFRelease(current_desc);
  CFRelease(safe_desc);
  if (feature_attributes != nullptr) {
    CFRelease(feature_attributes);
  }
  return safe_font;
}

CTFontRef ShaperCoreText::GetOrCreateSafeFont(const FontDescriptor& fd,
                                              float text_size) const {
#ifdef USE_UIKIT
  if (UseDefaultFontPool(fd, text_size)) return nullptr;
#endif

  CachedSafeFontKey cache_key{fd, text_size};
  auto& state = GetSafeFontCacheState();
  {
    std::lock_guard<std::mutex> lock(state.mutex_);

    auto iter = state.cache_.find(cache_key);
    if (iter != state.cache_.end()) {
      state.lru_.splice(state.lru_.begin(), state.lru_, iter->second.lru_it_);
      iter->second.lru_it_ = state.lru_.begin();
      return static_cast<CTFontRef>(CFRetain(iter->second.safe_font_));
    }
  }

  CTFontRef base_font = nullptr;
  if (fd.platform_font_ != 0) {
#ifdef USE_APPKIT
    CTFontRef platform_font =
        reinterpret_cast<CTFontRef>(static_cast<uintptr_t>(fd.platform_font_));
    base_font = CTFontCreateCopyWithAttributes(platform_font, text_size,
                                               nullptr, nullptr);
#endif
#ifdef USE_UIKIT
    UIFont* ui_font = (__bridge UIFont*)((void*)fd.platform_font_);
    base_font = (__bridge_retained CTFontRef)
        [UIFont fontWithDescriptor:ui_font.fontDescriptor size:text_size];
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
    base_font = CTFontCreateWithName(CFSTR("Helvetica"), text_size, nullptr);
#endif
#ifdef USE_UIKIT
    const auto font_weight =
        GetUIFontWeights()[GetDefaultUIFontWeightIndex(fd)];
    base_font =
        (__bridge_retained CTFontRef)[UIFont systemFontOfSize:text_size
                                                       weight:font_weight];
#endif
  }

  if (base_font == nullptr) {
    return nullptr;
  }

  CTFontRef safe_font = CreateSafeFontUnified(base_font, fd);
  CFRelease(base_font);
  if (safe_font == nullptr) {
    return nullptr;
  }

  std::lock_guard<std::mutex> lock(state.mutex_);

  auto iter = state.cache_.find(cache_key);
  if (iter != state.cache_.end()) {
    CFRelease(safe_font);
    state.lru_.splice(state.lru_.begin(), state.lru_, iter->second.lru_it_);
    iter->second.lru_it_ = state.lru_.begin();
    return static_cast<CTFontRef>(CFRetain(iter->second.safe_font_));
  }

  if (state.max_entries_ == 0) {
    return safe_font;
  }

  state.lru_.push_front(cache_key);
  state.cache_.emplace(cache_key,
                       CachedSafeFontEntry{safe_font, state.lru_.begin()});
  TrimSafeFontCacheLocked(state);
  return static_cast<CTFontRef>(CFRetain(safe_font));
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
void ShaperCoreText::OnShapeText(const ShapeKey& key, ShapeResult* result,
                                 const base::UnicodeProperties* cached) const {
  CFDictionaryRef attributes = GenerateAttributes(key);
  const EncodedShapeText encoded(key.text_, tmp_buf_, SHAPER_BUFF_SIZE);
  const auto* u16_char_map = encoded.Map();
  auto cfa = GenerateAttributeString(encoded, attributes, key.style_, cached);
  auto line = CTLineCreateWithAttributedString(cfa);
  CFArrayRef array_ref = CTLineGetGlyphRuns(line);
  auto array_count = CFArrayGetCount(array_ref);

  CTShapingResult ct_result;
  CTFontRef prev_font = nullptr;
  TypefaceRef run_typeface;
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
      run_typeface = std::move(typeface);
      prev_font = font;
    }

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
      if (static_cast<size_t>(ct_indices[k]) < key.text_.length() &&
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
    ct_result.AppendResult(run_typeface, ct_glyphs.data(), ct_advances.data(),
                           ct_indices.data(), ct_position.data(),
                           static_cast<uint32_t>(glyph_count),
                           static_cast<int32_t>(char_count), is_rtl);
  }
  result->AppendPlatformShapingResult(ct_result);
  CFRelease(attributes);
  CFRelease(cfa);
  CFRelease(line);
}

CFAttributedStringRef ShaperCoreText::GenerateAttributeString(
    const EncodedShapeText& text, CFDictionaryRef attribute,
    const ShapeStyle& style, const base::UnicodeProperties* cached) const {
  const auto length = static_cast<CFIndex>(text.Utf16().size());
  CFStringRef cf_string = CFStringCreateWithCharacters(
      kCFAllocatorDefault,
      reinterpret_cast<const UniChar*>(text.Utf16().data()), length);
  TTASSERT(length == CFStringGetLength(cf_string));
#ifdef USE_UIKIT
  auto attr_text = CreateStringWithDefaultFonts(cf_string, text, attribute,
                                                style.GetFontDescriptor(),
                                                style.GetFontSize(), cached);
#else
  (void)style;
  auto attr_text =
      CFAttributedStringCreate(kCFAllocatorDefault, cf_string, attribute);
#endif

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
  CTFontRef safe_font = GetOrCreateSafeFont(fd, text_size);
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
