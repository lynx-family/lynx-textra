// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_STYLE_STYLE_MANAGER_H_
#define SRC_TEXTLAYOUT_STYLE_STYLE_MANAGER_H_
#ifdef GTEST
#include <gtest/gtest.h>
#endif
#include <textra/paragraph_style.h>
#include <textra/style.h>
#include <textra/tt_color.h>

#include <map>
#include <memory>
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
  explicit AttributesRangeList(const AttributesRangeList& other) {
    *this = other;
  }

 public:
  void SetMergeRange(bool merge) { merge_range_ = merge; }
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
    for (const auto& other_range : other.range_list_) {
      range_list_.emplace_back(other_range.first, other_range.second);
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
  StyleManager() {
    style_list_[(AttrType)AttributeType::kDecorationType].SetMergeRange(false);
  }
  StyleManager(const StyleManager& other)
      : extra_style_list_(other.extra_style_list_),
        default_style_(other.default_style_) {
    for (AttrType k = 0; k < (AttrType)AttributeType::kMaxAttrType; k++) {
      style_list_[k] = other.style_list_[k];
    }
  }

 private:
  template <class T>
  inline uint64_t PackValue(const T& t) const {
    return (uint64_t)t;
  }

  template <class T>
  inline T UnPackValue(uint64_t v) const {
    return (T)v;
  }
  template <>
  uint64_t PackValue(const float& t) const {
    uint64_t v;
    memcpy(&v, &t, sizeof(float));
    return v;
  }
  template <>
  float UnPackValue(uint64_t v) const {
    float f;
    memcpy(&f, &v, sizeof(float));
    return f;
  }

#define DEFINE_STYLE_MANAGER_ATTRIBUTE(TYPE_NAME, TYPE_TYPE)                \
 public:                                                                    \
  void Set##TYPE_NAME(TYPE_TYPE attr, uint32_t start, uint32_t len = 1) {   \
    PRINTLOG("Set%s range start:%d, len:%d", #TYPE_NAME, start, len);       \
    auto type_id = (AttrType)AttributeType::k##TYPE_NAME;                   \
    auto max_end = Range::MaxIndex();                                       \
    auto end = len > max_end - start ? max_end : start + len;               \
    style_list_[type_id].SetRangeValue(Range{start, end}, PackValue(attr)); \
  }                                                                         \
  TYPE_TYPE Get##TYPE_NAME(uint32_t idx) const {                            \
    PRINTLOG("Get%s range idx:%d", #TYPE_NAME, idx);                        \
    auto type_id = (AttrType)AttributeType::k##TYPE_NAME;                   \
    auto value = style_list_[type_id].GetAttrValue(idx);                    \
    return value == AttributesRangeList::Undefined()                        \
               ? (default_style_.Get##TYPE_NAME())                          \
               : UnPackValue<TYPE_TYPE>(value);                             \
  }

 public:
  void SetParagraphStyle(const Style& default_style) {
    default_style_ = default_style;
  }

 public:
  void SetForegroundColor(TTColor attr, uint32_t start, uint32_t len = 1) {
    auto type_id = static_cast<AttrType>(AttributeType::kForegroundColor);
    auto max_end = Range::MaxIndex();
    auto end = len > max_end - start ? max_end : start + len;
    style_list_[type_id].SetRangeValue(Range{start, end}, attr.GetPlainColor());
  }

  TTColor GetForegroundColor(uint32_t idx) const {
    auto type_id = static_cast<AttrType>(AttributeType::kForegroundColor);
    auto value = style_list_[type_id].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetForegroundColor())
               : TTColor(static_cast<uint32_t>(value & 0xFFFFFFFF));
  }

 public:
  void SetBackgroundColor(TTColor attr, uint32_t start, uint32_t len = 1) {
    auto type_id = (AttrType)AttributeType::kBackgroundColor;
    auto max_end = Range::MaxIndex();
    auto end = len > max_end - start ? max_end : start + len;
    style_list_[type_id].SetRangeValue(Range{start, end}, attr.GetPlainColor());
  }

  TTColor GetBackgroundColor(uint32_t idx) const {
    auto type_id = (AttrType)AttributeType::kBackgroundColor;
    auto value = style_list_[type_id].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetBackgroundColor())
               : TTColor(static_cast<uint32_t>(value & 0xFFFFFFFF));
  }

 public:
  void SetDecorationColor(TTColor attr, uint32_t start, uint32_t len = 1) {
    auto type_id = (AttrType)AttributeType::kDecorationColor;
    auto max_end = Range::MaxIndex();
    auto end = len > max_end - start ? max_end : start + len;
    style_list_[type_id].SetRangeValue(Range{start, end}, attr.GetPlainColor());
  }

  TTColor GetDecorationColor(uint32_t idx) const {
    auto type_id = (AttrType)AttributeType::kDecorationColor;
    auto value = style_list_[type_id].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetDecorationColor())
               : TTColor(static_cast<uint32_t>(value & 0xFFFFFFFF));
  }
  DEFINE_STYLE_MANAGER_ATTRIBUTE(DecorationType, DecorationType)
  DEFINE_STYLE_MANAGER_ATTRIBUTE(DecorationThicknessMultiplier, float)
  DEFINE_STYLE_MANAGER_ATTRIBUTE(ForegroundPainter, Painter*)
  DEFINE_STYLE_MANAGER_ATTRIBUTE(BackgroundPainter, Painter*)
  DEFINE_STYLE_MANAGER_ATTRIBUTE(WordBreak, WordBreakType)
  DEFINE_STYLE_MANAGER_ATTRIBUTE(BaselineOffset, float)

 public:
  using TextShadowList = std::vector<TextShadow>;
  void SetTextShadowList(TextShadowList& attr, uint32_t start,
                         uint32_t len = 1) {
    auto type_id = (AttrType)AttributeType::kTextShadowList;
    auto max_end = Range::MaxIndex();
    auto end = len > max_end - start ? max_end : start + len;
    style_list_[type_id].SetRangeValue(Range{start, end}, PackValue(&attr));
  }
  const TextShadowList& GetTextShadowList(uint32_t idx) const {
    auto type_id = (AttrType)AttributeType::kTextShadowList;
    auto value = style_list_[type_id].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetTextShadowList())
               : *reinterpret_cast<TextShadowList*>(value);
  }

 public:
  void SetDecorationStyle(LineType attr, uint32_t start, uint32_t len = 1) {
    LogUtil::D("Set%s range start:%d, len:%d", "DecorationStyle", start, len);
    auto type_id = (AttrType)AttributeType::kDecorationStyle;
    auto max_end = Range::MaxIndex();
    auto end = len > max_end - start ? max_end : start + len;
    style_list_[type_id].SetRangeValue(Range{start, end}, PackValue(attr));
  }
  LineType GetDecorationStyle(uint32_t idx) const {
    LogUtil::D("Get%s range idx:%d", "DecorationStyle", idx);
    auto type_id = (AttrType)AttributeType::kDecorationStyle;
    auto value = style_list_[type_id].GetAttrValue(idx);
    return value == AttributesRangeList::Undefined()
               ? (default_style_.GetDecorationStyle())
               : UnPackValue<LineType>(value);
  }
  void ApplyStyleInRange(const Style& style, uint32_t start, uint32_t len);
  AttributesRangeList::ValueType GetTypeValue(AttributeType type,
                                              uint32_t idx) {
    auto type_id = (AttrType)type;
    return style_list_[type_id].GetAttrValue(idx);
  }
  void ClearStyleInRange(AttributeType type, const Range& range) {
    auto type_id = (AttrType)type;
    style_list_[type_id].ClearRangeValue(range);
  }
  const Style GetStyle(uint32_t id);
  void GetStyleRange(StyleRange* style_range, uint32_t start_char,
                     AttrType flag = Style::FullFlag(),
                     Range::RangeType end_char = Range::MaxIndex()) const;
  void GetStyleRange(Range* range, uint32_t start_char,
                     AttrType flag = Style::FullFlag(),
                     Range::RangeType end_char = Range::MaxIndex()) const;

  void ClearExtraAttributes(AttributeType extra_attr) {
    extra_style_list_[(AttrType)extra_attr].Clear();
  }
  void SetExtraFloatAttributesInRange(AttributeType extra_attr, float value,
                                      uint32_t start, uint32_t len);
  float GetExtraFloatAttributes(AttributeType extra_attr, uint32_t idx,
                                float default_value = 0);

 private:
  void SetStyle(Style* style, uint64_t value, AttributeType type) const;
  AttributesRangeList::ValueType GetStyleValue(const Style* style,
                                               AttributeType type) const;

 private:
  AttributesRangeList style_list_[(AttrType)AttributeType::kMaxAttrType];
  std::map<AttrType, AttributesRangeList> extra_style_list_;
  Style default_style_;
// #undef GETVALUE
// #undef SETVALUE
#undef DEFINE_STYLE_MANAGER_ATTRIBUTE
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_STYLE_STYLE_MANAGER_H_
