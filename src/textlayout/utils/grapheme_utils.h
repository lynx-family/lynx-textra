// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_
#define SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_

#include <cstdint>

#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {

using base::IsCombiningEnclosingKeycap;
using base::IsDevanagariVirama;
using base::IsEmojiCodepoint;
using base::IsEmojiKeycapBase;
using base::IsEmojiModifier;
using base::IsEmojiRegionalIndicator;
using base::IsEmojiTagCharacter;
using base::IsGraphemeExtender;
using base::IsZeroWidthJoiner;
using base::IsZeroWidthNonJoiner;
using base::UnicodeProperties;

inline bool IsEmojiKeycapSequence(uint32_t base, uint32_t selector,
                                  uint32_t keycap) {
  return IsEmojiKeycapBase(base) && selector == 0xFE0F &&
         IsCombiningEnclosingKeycap(keycap);
}

inline bool IsCommonEmoji(uint32_t codepoint) {
  // ASCII emoji characters are keycap bases and also occur in ordinary text.
  return codepoint > 0x7F && IsEmojiCodepoint(codepoint);
}

inline bool IsEmojiBaseForFallback(uint32_t codepoint,
                                   UnicodeProperties properties) {
  return codepoint > 0x7F && properties.Has(UnicodeProperties::kEmoji);
}

inline bool IsEmojiFallbackIgnorable(uint32_t codepoint,
                                     UnicodeProperties properties) {
  return properties.Has(UnicodeProperties::kVariationSelector) ||
         IsEmojiModifier(codepoint) || IsEmojiTagCharacter(codepoint) ||
         IsZeroWidthJoiner(codepoint);
}

inline uint32_t EmojiFallbackKey(
    const char32_t* text, uint32_t start, uint32_t end,
    const base::UnicodeProperties* cached = nullptr) {
  if (text == nullptr || start >= end) {
    return 0;
  }

  for (uint32_t i = start; i < end; ++i) {
    uint32_t codepoint = text[i];
    if (IsCombiningEnclosingKeycap(codepoint) ||
        IsEmojiRegionalIndicator(codepoint) ||
        IsEmojiBaseForFallback(
            codepoint, base::GetUnicodePropertiesAt(cached, i, codepoint))) {
      return codepoint;
    }
  }

  for (uint32_t i = start; i < end; ++i) {
    uint32_t codepoint = text[i];
    if (!IsEmojiFallbackIgnorable(
            codepoint, base::GetUnicodePropertiesAt(cached, i, codepoint))) {
      return codepoint;
    }
  }

  return text[start];
}

enum class EmojiPresentation { kDefault, kText, kEmoji };

struct EmojiClusterInfo {
  EmojiPresentation presentation = EmojiPresentation::kDefault;
};

// Analyze the emoji presentation of one grapheme.
inline EmojiClusterInfo GetEmojiClusterInfo(
    const char32_t* text, uint32_t start, uint32_t end,
    const base::UnicodeProperties* cached = nullptr) {
  EmojiClusterInfo info;
  if (text == nullptr || start >= end) {
    return info;
  }
  uint32_t emoji_base_index = end;
  bool text_presentation = false;
  bool emoji_sequence = false;
  bool keycap = false;
  bool default_emoji_presentation = false;
  for (uint32_t i = start; i < end; ++i) {
    const auto ch = text[i];
    if (emoji_base_index == end && !IsEmojiModifier(ch)) {
      const auto properties = base::GetUnicodePropertiesAt(cached, i, ch);
      if (properties.Has(UnicodeProperties::kEmoji)) {
        emoji_base_index = i;
        default_emoji_presentation =
            properties.Has(UnicodeProperties::kEmojiPresentation);
      }
    }
    text_presentation |= ch == 0xFE0E;
    emoji_sequence |= ch == 0xFE0F || IsZeroWidthJoiner(ch) ||
                      IsEmojiModifier(ch) || IsEmojiTagCharacter(ch);
    keycap |= IsCombiningEnclosingKeycap(ch);
  }
  if (text_presentation) {
    info.presentation = EmojiPresentation::kText;
  } else if (emoji_base_index != end) {
    const auto base = text[emoji_base_index];
    if ((IsEmojiKeycapBase(base) && keycap) || default_emoji_presentation ||
        emoji_sequence) {
      info.presentation = EmojiPresentation::kEmoji;
    }
  }
  return info;
}

struct GraphemeRange {
  uint32_t start;
  uint32_t end;
};

inline bool IsGraphemeClusterJoinable(uint32_t codepoint,
                                      const base::UnicodeProperties* cached,
                                      uint32_t index) {
  if (codepoint < 0x80) return IsEmojiKeycapBase(codepoint);
  const auto properties =
      base::GetUnicodePropertiesAt(cached, index, codepoint);
  return IsZeroWidthJoiner(codepoint) || properties.IsGraphemeExtender() ||
         properties.Has(UnicodeProperties::kEmoji |
                        UnicodeProperties::kScriptDevanagari);
}

template <typename GetCodepoint>
inline bool IsGraphemeExtenderAt(uint32_t index,
                                 const GetCodepoint& get_codepoint,
                                 const base::UnicodeProperties* cached) {
  return cached ? cached[index].IsGraphemeExtender()
                : IsGraphemeExtender(get_codepoint(index));
}

template <typename GetCodepoint>
inline uint32_t FindGraphemeClusterEnd(
    uint32_t start, uint32_t count, const GetCodepoint& get_codepoint,
    const base::UnicodeProperties* cached = nullptr) {
  if (start >= count) {
    return start;
  }
  const uint32_t codepoint = get_codepoint(start);

  if (IsEmojiRegionalIndicator(codepoint) && start + 1 < count &&
      IsEmojiRegionalIndicator(get_codepoint(start + 1))) {
    return start + 2;
  }

  if (start + 2 < count &&
      IsEmojiKeycapSequence(codepoint, get_codepoint(start + 1),
                            get_codepoint(start + 2))) {
    return start + 3;
  }

  const auto properties =
      base::GetUnicodePropertiesAt(cached, start, codepoint);
  if (IsEmojiBaseForFallback(codepoint, properties)) {
    uint32_t end = start + 1;
    while (end < count) {
      const uint32_t next = get_codepoint(end);
      if (base::GetUnicodePropertiesAt(cached, end, next)
              .IsGraphemeExtender()) {
        ++end;
        continue;
      }
      if (IsZeroWidthJoiner(next) && end + 1 < count) {
        const auto joined = get_codepoint(end + 1);
        if (IsEmojiBaseForFallback(joined, base::GetUnicodePropertiesAt(
                                               cached, end + 1, joined))) {
          end += 2;
          continue;
        }
      }
      break;
    }
    return end;
  }

  if (properties.Has(UnicodeProperties::kScriptDevanagari)) {
    uint32_t end = start + 1;
    while (end < count) {
      const uint32_t next = get_codepoint(end);
      if (IsZeroWidthJoiner(next) ||
          base::GetUnicodePropertiesAt(cached, end, next)
              .IsGraphemeExtender()) {
        ++end;
        if (IsDevanagariVirama(next)) {
          while (end < count && (IsZeroWidthNonJoiner(get_codepoint(end)) ||
                                 IsZeroWidthJoiner(get_codepoint(end)))) {
            ++end;
          }
          if (end < count &&
              base::GetUnicodePropertiesAt(cached, end, get_codepoint(end))
                  .Has(UnicodeProperties::kDevanagariConsonant)) {
            ++end;
          }
        }
        continue;
      }
      break;
    }
    return end;
  }

  uint32_t end = start + 1;
  while (end < count && IsGraphemeExtenderAt(end, get_codepoint, cached)) {
    ++end;
  }
  return end;
}

template <typename GetCodepoint>
inline GraphemeRange FindGraphemeCluster(
    uint32_t offset, uint32_t count, const GetCodepoint& get_codepoint,
    const base::UnicodeProperties* cached = nullptr) {
  if (offset >= count) {
    return {0, 0};
  }
  uint32_t cluster_start = offset;
  while (cluster_start > 0 &&
         (IsGraphemeClusterJoinable(get_codepoint(cluster_start - 1), cached,
                                    cluster_start - 1) ||
          IsGraphemeExtenderAt(cluster_start, get_codepoint, cached))) {
    --cluster_start;
  }
  while (cluster_start <= offset) {
    const uint32_t cluster_end =
        FindGraphemeClusterEnd(cluster_start, count, get_codepoint, cached);
    if (cluster_end > offset) {
      return {cluster_start, cluster_end};
    }
    cluster_start = cluster_end;
  }
  return {offset, offset + 1};
}

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_
