// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_STYLE_STYLE_MANAGER_H_
#define SRC_TEXTLAYOUT_STYLE_STYLE_MANAGER_H_
#ifdef GTEST
#include <gtest/gtest.h>
#endif
#include <textra/style.h>
#include <textra/tt_color.h>

#include <map>
#include <utility>
#include <vector>

#include "src/textlayout/utils/log_util.h"
#include "src/textlayout/utils/tt_range.h"
namespace ttoffice {
namespace tttext {
class StyleRange {
  friend StyleManager;
  friend BaseRun;
  friend GhostRun;

 public:
  StyleRange() : range_(0, 0), style_() {}
  StyleRange(const Range& range, const Style& style)
      : range_(range), style_(style) {}

 public:
  const Range& GetRange() const { return range_; }
  const Style& GetStyle() const { return style_; }

 private:
  Range range_;
  Style style_;
};
/**
 * Value range list for a single attribute, merges ranges with the same value
 */
class AttributesRangeList {
#define PRINTLOG
#ifdef GTEST
  FRIEND_TEST(AttributesRangeListTest, SetRangeValue);
#endif

 public:
  using ValueType = uint64_t;
  static constexpr ValueType Undefined() { return -1; }
  using UniqueAttributeRange = std::pair<Range, ValueType>;
  using AttributeRange = std::pair<Range, ValueType>;

 public:
  AttributesRangeList() : merge_range_(true) {}
  AttributesRangeList(const AttributesRangeList& other) { *this = other; }

 public:
  void SetMergeRange(const bool merge) { merge_range_ = merge; }
  void SetRangeValue(const Range& range, ValueType value);
  void ClearRangeValue(const Range& range) {
    SetRangeValue(range, Undefined());
  }
  void Clear() { range_list_.clear(); }
  ValueType GetAttrValue(uint32_t idx) const;
  /**
   * Query the style range that idx belongs to, returns the unmatched range if
   * no match is found
   * @param idx
   * @return
   */
  AttributeRange GetAttributeRange(uint32_t idx) const;

 public:
  AttributesRangeList& operator=(const AttributesRangeList& other) {
    for (const auto& [fst, snd] : other.range_list_) {
      range_list_.emplace_back(fst, snd);
    }
    merge_range_ = other.merge_range_;
    return *this;
  }

 protected:
  std::vector<UniqueAttributeRange> range_list_;
  bool merge_range_{};
};
class StyleManager {
 public:
  StyleManager() { style_list_[kDecorationType].SetMergeRange(false); }
  StyleManager(const StyleManager& other)
      : extra_style_list_(other.extra_style_list_),
        default_style_(other.default_style_) {
    for (AttrType k = 0; k < kMaxAttrType; k++) {
      style_list_[k] = other.style_list_[k];
    }
  }

 private:
  template <class T>
  static uint64_t PackValue(const T& t) {
    uint64_t ret = 0;
    memcpy(&ret, &t, sizeof(t));
    return ret;
  }

  template <class T>
  T UnPackValue(const uint64_t v) const {
    T ret;
    memcpy(&ret, &v, sizeof(T));
    return ret;
  }

 public:
  void SetParagraphStyle(const Style& default_style) {
    default_style_ = default_style;
  }

  void SetForegroundColor(const TTColor attr, const uint32_t start,
                          const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kForegroundColor].SetRangeValue(Range{start, end}, attr);
  }

  TTColor GetForegroundColor(const uint32_t idx) const {
    const auto value = style_list_[kForegroundColor].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? default_style_.GetForegroundColor()
               : TTColor(value & 0xFFFFFFFF);
  }

  void SetBackgroundColor(const TTColor attr, const uint32_t start,
                          const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kBackgroundColor].SetRangeValue(Range{start, end}, attr);
  }

  TTColor GetBackgroundColor(const uint32_t idx) const {
    const auto value = style_list_[kBackgroundColor].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? default_style_.GetBackgroundColor()
               : TTColor(value & 0xFFFFFFFF);
  }

  void SetDecorationColor(const TTColor attr, const uint32_t start,
                          const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kDecorationColor].SetRangeValue(Range{start, end}, attr);
  }

  TTColor GetDecorationColor(const uint32_t idx) const {
    const auto value = style_list_[kDecorationColor].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetDecorationColor())
               : TTColor(value & 0xFFFFFFFF);
  }

  void SetDecorationType(const DecorationType attr, const uint32_t start,
                         const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kDecorationType].SetRangeValue(Range{start, end},
                                               PackValue(attr));
  }

  DecorationType GetDecorationType(const uint32_t idx) const {
    const auto value = style_list_[kDecorationType].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetDecorationType())
               : UnPackValue<DecorationType>(value);
  }

  void SetDecorationThicknessMultiplier(const float attr, const uint32_t start,
                                        const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kDecorationThicknessMultiplier].SetRangeValue(Range{start, end},
                                                              PackValue(attr));
  }

  float GetDecorationThicknessMultiplier(const uint32_t idx) const {
    const auto value =
        style_list_[kDecorationThicknessMultiplier].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetDecorationThicknessMultiplier())
               : UnPackValue<float>(value);
  }

  void SetForegroundPainter(const Painter* attr, const uint32_t start,
                            const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kForegroundPainter].SetRangeValue(Range{start, end},
                                                  PackValue(attr));
  }

  Painter* GetForegroundPainter(const uint32_t idx) const {
    const auto value = style_list_[kForegroundPainter].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetForegroundPainter())
               : UnPackValue<Painter*>(value);
  }

  void SetBackgroundPainter(const Painter* attr, const uint32_t start,
                            const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kBackgroundPainter].SetRangeValue(Range{start, end},
                                                  PackValue(attr));
  }

  Painter* GetBackgroundPainter(const uint32_t idx) const {
    const auto value = style_list_[kBackgroundPainter].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetBackgroundPainter())
               : UnPackValue<Painter*>(value);
  }

  void SetWordBreak(const WordBreakType attr, const uint32_t start,
                    const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kWordBreak].SetRangeValue(Range{start, end}, PackValue(attr));
  }

  WordBreakType GetWordBreak(const uint32_t idx) const {
    const auto value = style_list_[kWordBreak].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetWordBreak())
               : UnPackValue<WordBreakType>(value);
  }

  void SetBaselineOffset(const float attr, const uint32_t start,
                         const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kBaselineOffset].SetRangeValue(Range{start, end},
                                               PackValue(attr));
  }

  float GetBaselineOffset(const uint32_t idx) const {
    const auto value = style_list_[kBaselineOffset].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetBaselineOffset())
               : UnPackValue<float>(value);
  }

  using TextShadowList = std::vector<TextShadow>;
  void SetTextShadowList(const TextShadowList& attr, const uint32_t start,
                         const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kTextShadowList].SetRangeValue(Range{start, end},
                                               PackValue(&attr));
  }
  const TextShadowList& GetTextShadowList(const uint32_t idx) const {
    const auto value = style_list_[kTextShadowList].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetTextShadowList())
               : *reinterpret_cast<TextShadowList*>(value);
  }

  void SetDecorationStyle(const LineType attr, const uint32_t start,
                          const uint32_t len = 1) {
    constexpr auto max_end = Range::MaxIndex();
    const auto end = len > max_end - start ? max_end : start + len;
    style_list_[kDecorationStyle].SetRangeValue(Range{start, end},
                                                PackValue(attr));
  }

  LineType GetDecorationStyle(const uint32_t idx) const {
    const auto value = style_list_[kDecorationStyle].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetDecorationStyle())
               : UnPackValue<LineType>(value);
  }

  void ApplyStyleInRange(const Style& style, const uint32_t start,
                         const uint32_t len);
  AttributesRangeList::ValueType GetTypeValue(const AttributeType type,
                                              const uint32_t idx) const {
    return style_list_[type].GetAttrValue(idx);
  }
  void ClearStyleInRange(const AttributeType type, const Range& range) {
    style_list_[type].ClearRangeValue(range);
  }

  Style GetStyle(const uint32_t index) const;
  void GetStyleRange(StyleRange* style_range, uint32_t start_char,
                     AttrType flag = Style::FullFlag,
                     Range::RangeType end_char = Range::MaxIndex()) const;
  void GetStyleRange(Range* range, uint32_t start_char,
                     AttrType flag = Style::FullFlag,
                     Range::RangeType end_char = Range::MaxIndex()) const;

  void ClearExtraAttributes(const AttributeType extra_attr) {
    extra_style_list_[extra_attr].Clear();
  }
  void SetExtraFloatAttributesInRange(AttributeType extra_attr, float value,
                                      uint32_t start, uint32_t len);
  float GetExtraFloatAttributes(AttributeType extra_attr, uint32_t idx,
                                float default_value = 0);

 private:
  void SetStyle(Style* style, uint64_t value, AttributeType type) const;
  static AttributesRangeList::ValueType GetStyleValue(const Style* style,
                                                      AttributeType type);

 private:
  AttributesRangeList style_list_[kMaxAttrType];
  std::map<AttrType, AttributesRangeList> extra_style_list_;
  Style default_style_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_STYLE_STYLE_MANAGER_H_
