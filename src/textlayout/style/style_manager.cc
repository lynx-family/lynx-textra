// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/style/style_manager.h"

#include <textra/macro.h>

#include <algorithm>
#include <limits>

namespace ttoffice {
namespace tttext {
void AttributesRangeList::SetRangeValue(const Range& range, ValueType value) {
  if (range.Empty()) return;

  auto iter = range_list_.begin();
  bool inserted = false;
  /**
   * Step 1: Delete areas in the list that are contained by range, and insert
   * the range area
   */
  while (iter != range_list_.end()) {
    if (iter->first.GetStart() >= range.GetEnd()) {
      if (!inserted && value != Undefined()) {
        iter = range_list_.insert(iter, std::make_pair(range, value));
        inserted = true;
      }
      break;
    }
    if (range.Contain(iter->first)) {
      iter = range_list_.erase(iter);
      continue;
    }
    auto r2 = iter->first.Difference(range);
    if (!r2.Empty()) {
      iter = range_list_.insert(iter + 1, std::make_pair(r2, iter->second));
    }
    if (range < iter->first) {
      TTASSERT(!inserted);
      iter = range_list_.insert(iter, std::make_pair(range, value));
      iter++;
      inserted = true;
    }
    iter++;
  }
  if (!inserted && value != Undefined()) {
    range_list_.emplace_back(std::make_pair(range, value));
  }
  /**
   * Check if adjacent areas can be merged into one
   */
  if (merge_range_) {
    iter = range_list_.begin();
    while (iter != range_list_.end() && iter + 1 != range_list_.end()) {
      auto next = iter + 1;
      if (iter->first.IsAdjacent(next->first)) {
        if (iter->second == next->second) {
          iter->first.Merge(next->first);
          range_list_.erase(next);
          continue;
        }
      }
      iter++;
    }
  }
#ifdef TTTEXT_DEBUG
  iter = range_list_.begin();
  while (iter != range_list_.end() && iter + 1 != range_list_.end()) {
    TTASSERT(iter->first < (iter + 1)->first);
    iter++;
  }
#endif
}
AttributesRangeList::ValueType AttributesRangeList::GetAttrValue(
    uint32_t idx) const {
  for (auto& range : range_list_) {
    if (range.first > idx) break;
    if (range.first.Contain(idx)) {
      return range.second;
    }
  }
  return Undefined();
}
AttributesRangeList::AttributeRange AttributesRangeList::GetAttributeRange(
    uint32_t idx) const {
  uint32_t miss_start = 0;
  uint32_t miss_end = std::numeric_limits<uint32_t>::max();
  for (auto& range : range_list_) {
    if (range.first > idx) {
      miss_end = range.first.GetStart();
      break;
    }
    if (range.first.Contain(idx)) {
      return {range.first, range.second};
    }
    miss_start = range.first.GetEnd();
  }
  return {Range{miss_start, miss_end}, Undefined()};
}
void StyleManager::ApplyStyleInRange(const Style& style, uint32_t start,
                                     uint32_t len) {
  for (int id = (int32_t)AttributeType::kStyleManagerAttrStart;
       id < (int32_t)AttributeType::kStyleManagerAttrEnd; id++) {
    if (!style.HasAttribute((AttributeType)id)) continue;
    auto value = GetStyleValue(&style, (AttributeType)id);
    auto max_end = Range::MaxIndex();
    auto end = len > max_end - start ? max_end : start + len;
    style_list_[id].SetRangeValue(Range{start, end}, value);
  }
}
const Style StyleManager::GetStyle(uint32_t idx) {
  Style ret(default_style_);
  for (int id = (int32_t)AttributeType::kStyleManagerAttrStart;
       id < (int32_t)AttributeType::kStyleManagerAttrEnd; id++) {
    auto var = GetTypeValue((AttributeType)id, idx);
    if (var != AttributesRangeList::Undefined()) {
      SetStyle(&ret, var, (AttributeType)id);
    }
  }
  return ret;
}
void StyleManager::GetStyleRange(StyleRange* style_range, uint32_t start_char,
                                 uint32_t flag,
                                 Range::RangeType end_char) const {
  auto* style = &style_range->style_;
  auto end = end_char;
  auto range_start = start_char;
  auto range_end = end;
  for (int id = (int32_t)AttributeType::kStyleManagerAttrStart;
       id < (int32_t)AttributeType::kStyleManagerAttrEnd; id++) {
    auto style_flag = 1u << (uint32_t)id;
    if (flag & style_flag) {
      auto attr_range = style_list_[id].GetAttributeRange(start_char);
      if (attr_range.second != AttributesRangeList::Undefined()) {
        SetStyle(style, attr_range.second, (AttributeType)id);
      } else {
        SetStyle(style, GetStyleValue(&default_style_, (AttributeType)id),
                 (AttributeType)id);
      }
      range_start = std::max(range_start, attr_range.first.GetStart());
      range_end = std::min(range_end, attr_range.first.GetEnd());
    }
  }
  style_range->range_ = Range{range_start, range_end};
  TTASSERT(style_range->range_.Contain(start_char));
}
void StyleManager::GetStyleRange(Range* range, uint32_t start_char,
                                 AttrType flag,
                                 Range::RangeType end_char) const {
  auto end = end_char;
  auto range_start = start_char;
  auto range_end = end;
  for (int id = (int32_t)AttributeType::kStyleManagerAttrStart;
       id < (int32_t)AttributeType::kStyleManagerAttrEnd; id++) {
    auto style_flag = 1u << (uint32_t)id;
    if (flag & style_flag) {
      auto attr_range = style_list_[id].GetAttributeRange(start_char);
      range_start = std::max(range_start, attr_range.first.GetStart());
      range_end = std::min(range_end, attr_range.first.GetEnd());
    }
  }
  *range = Range{range_start, range_end};
  TTASSERT(range->Contain(start_char));
}
void StyleManager::SetExtraFloatAttributesInRange(AttributeType extra_attr,
                                                  float value, uint32_t start,
                                                  uint32_t len) {
  auto type_id = (AttrType)extra_attr;
  auto& range_list = extra_style_list_[type_id];
  range_list.SetRangeValue(Range{start, start + len}, PackValue(value));
}
float StyleManager::GetExtraFloatAttributes(AttributeType extra_attr,
                                            uint32_t idx, float default_value) {
  float ret = default_value;
  auto type_id = (AttrType)extra_attr;
  auto& range_list = extra_style_list_[type_id];
  auto attr = range_list.GetAttrValue(idx);
  if (attr != AttributesRangeList::Undefined()) {
    ret = UnPackValue<float>(attr);
  }
  return ret;
}
void StyleManager::SetStyle(Style* style, uint64_t value,
                            AttributeType type) const {
  switch (type) {
    case AttributeType::kForegroundColor:
      return style->SetForegroundColor(TTColor(static_cast<uint32_t>(value)));
    case AttributeType::kBackgroundColor:
      return style->SetBackgroundColor(TTColor(static_cast<uint32_t>(value)));
    case AttributeType::kDecorationColor:
      return style->SetDecorationColor(TTColor(static_cast<uint32_t>(value)));
    case AttributeType::kDecorationType:
      return style->SetDecorationType((DecorationType)value);
    case AttributeType::kDecorationStyle:
      return style->SetDecorationStyle((LineType)value);
    case AttributeType::kDecorationThicknessMultiplier:
      return style->SetDecorationThicknessMultiplier(UnPackValue<float>(value));
    case AttributeType::kBold:
      return style->SetBold(value);
    case AttributeType::kItalic:
      return style->SetItalic(value);
    case AttributeType::kTextShadowList:
      return style->SetTextShadowList(*(const TextShadowList*)value);
    case AttributeType::kForegroundPainter:
      return style->SetForegroundPainter(reinterpret_cast<Painter*>(value));
    case AttributeType::kBackgroundPainter:
      return style->SetBackgroundPainter(reinterpret_cast<Painter*>(value));
    case AttributeType::kWordBreak:
      return style->SetWordBreak((WordBreakType)value);
    case AttributeType::kBaselineOffset:
      return style->SetBaselineOffset(UnPackValue<float>(value));
    default:
      TTASSERT(false);
  }
}
AttributesRangeList::ValueType StyleManager::GetStyleValue(
    const Style* style, AttributeType type) const {
  switch (type) {
    case AttributeType::kForegroundColor:
      return style->GetForegroundColor();
    case AttributeType::kBackgroundColor:
      return style->GetBackgroundColor();
    case AttributeType::kDecorationColor:
      return style->GetDecorationColor();
    case AttributeType::kDecorationType:
      return (uint64_t)style->GetDecorationType();
    case AttributeType::kDecorationStyle:
      return (uint64_t)style->GetDecorationStyle();
    case AttributeType::kDecorationThicknessMultiplier:
      return PackValue(
          static_cast<float>(style->GetDecorationThicknessMultiplier()));
    case AttributeType::kBold:
      return (uint64_t)style->GetBold();
    case AttributeType::kItalic:
      return (uint64_t)style->GetItalic();
    case AttributeType::kTextShadowList:
      return (uint64_t)&style->GetTextShadowList();
    case AttributeType::kForegroundPainter:
      return (uint64_t)style->GetForegroundPainter();
    case AttributeType::kBackgroundPainter:
      return (uint64_t)style->GetBackgroundPainter();
    case AttributeType::kWordBreak:
      return (uint64_t)style->GetWordBreak();
    case AttributeType::kBaselineOffset:
      return PackValue(static_cast<float>(style->GetBaselineOffset()));
    default:
      TTASSERT(false);
  }
  return AttributesRangeList::Undefined();
}
}  // namespace tttext
}  // namespace ttoffice
