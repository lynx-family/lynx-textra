// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <utility>
#include <vector>

#include "src/capi/textra_callback_utils.h"
#include "src/capi/textra_internal.h"
#include "src/textlayout/utils/u_8_string.h"

using namespace textra_capi;

textra_status_t TEXTRA_CALL
textra_font_descriptor_create(textra_font_descriptor_t** out_descriptor) {
  if (!out_descriptor) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_descriptor = nullptr;
  *out_descriptor = new textra_font_descriptor();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_font_descriptor_clone(const textra_font_descriptor_t* source,
                             textra_font_descriptor_t** out_font_descriptor) {
  if (!out_font_descriptor) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_font_descriptor = nullptr;
  if (!source) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto copy = std::make_unique<textra_font_descriptor>();
  copy->impl = source->impl;
  copy->typeface = source->typeface;
  *out_font_descriptor = copy.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_font_descriptor_clear_families(textra_font_descriptor_t* descriptor) {
  if (!descriptor) return TEXTRA_STATUS_INVALID_ARGUMENT;
  descriptor->impl.font_family_list_.clear();
  descriptor->impl.font_family_literals_.clear();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_font_descriptor_add_family(
    textra_font_descriptor_t* descriptor, const char* utf8, uint32_t byte_count,
    textra_bool_t literal) {
  if (!descriptor || !byte_count ||
      !::ttoffice::base::CheckValidUTF8String(utf8, byte_count, false) ||
      !Bool(literal))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  descriptor->impl.font_family_list_.emplace_back(utf8, byte_count);
  descriptor->impl.font_family_literals_.push_back(
      static_cast<uint8_t>(literal));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_font_descriptor_get_family_count(
    const textra_font_descriptor_t* descriptor, uint32_t* out_count) {
  if (!descriptor || !out_count) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_count = static_cast<uint32_t>(descriptor->impl.font_family_list_.size());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_font_descriptor_copy_family(
    const textra_font_descriptor_t* descriptor, uint32_t index, char* buffer,
    uint32_t capacity, uint32_t* out_size, textra_bool_t* out_literal) {
  if (!descriptor || !out_literal || !out_size)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (index >= descriptor->impl.font_family_list_.size())
    return TEXTRA_STATUS_OUT_OF_RANGE;
  auto status = CopyString(descriptor->impl.font_family_list_[index], buffer,
                           capacity, out_size);
  if (status == TEXTRA_STATUS_OK)
    *out_literal = descriptor->impl.font_family_literals_.empty()
                       ? TEXTRA_FALSE
                       : descriptor->impl.font_family_literals_[index];
  return status;
}

textra_status_t TEXTRA_CALL textra_font_descriptor_set_font_style(
    textra_font_descriptor_t* descriptor, const textra_font_style_t* style) {
  if (!descriptor || !style || style->weight < 1 ||
      style->weight > TEXTRA_FONT_WEIGHT_EXTRA_BLACK ||
      style->width < TEXTRA_FONT_WIDTH_ULTRA_CONDENSED ||
      style->width > TEXTRA_FONT_WIDTH_ULTRA_EXPANDED)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (style->slant > TEXTRA_FONT_SLANT_OBLIQUE)
    return TEXTRA_STATUS_UNSUPPORTED;
  descriptor->impl.font_style_ = tt::FontStyle(
      static_cast<tt::Weight>(style->weight),
      static_cast<tt::Width>(style->width), static_cast<uint8_t>(style->slant));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_font_descriptor_get_font_style(
    const textra_font_descriptor_t* descriptor,
    textra_font_style_t* out_style) {
  if (!descriptor || !out_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& value = descriptor->impl.font_style_;
  *out_style = {value.GetWeight(), value.GetWidth(), value.GetSlant()};
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_font_descriptor_set_font_features(
    textra_font_descriptor_t* descriptor, const textra_font_feature_t* features,
    uint32_t count) {
  if (!descriptor || (!features && count))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  for (uint32_t i = 0; i < count; ++i) {
    for (unsigned char byte : features[i].tag) {
      if (byte < 0x20 || byte > 0x7e) return TEXTRA_STATUS_INVALID_ARGUMENT;
    }
  }
  std::vector<tt::FontFeature> values;
  values.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    const auto& feature = features[i];
    values.push_back(
        {{feature.tag[0], feature.tag[1], feature.tag[2], feature.tag[3]},
         feature.value});
  }
  descriptor->impl.font_feature_settings_ = std::move(values);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_font_descriptor_get_font_feature_count(
    const textra_font_descriptor_t* descriptor, uint32_t* out_count) {
  if (!descriptor || !out_count) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_count =
      static_cast<uint32_t>(descriptor->impl.font_feature_settings_.size());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_font_descriptor_get_font_feature(
    const textra_font_descriptor_t* descriptor, uint32_t index,
    textra_font_feature_t* out_feature) {
  if (!descriptor || !out_feature) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& features = descriptor->impl.font_feature_settings_;
  if (index >= features.size()) return TEXTRA_STATUS_OUT_OF_RANGE;
  const auto& feature = features[index];
  std::memcpy(out_feature->tag, feature.tag, sizeof(out_feature->tag));
  out_feature->value = feature.value;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_font_descriptor_set_typeface(textra_font_descriptor_t* descriptor,
                                    const textra_typeface_options_t* options) {
  if (!descriptor) return TEXTRA_STATUS_INVALID_ARGUMENT;
  Ref<textra_typeface> face;
  if (options) {
    textra_typeface_t* value = nullptr;
    auto status = textra_typeface_create(options, &value);
    if (status) return status;
    face = Ref<textra_typeface>::Adopt(value);
  }
  descriptor->impl.platform_font_ =
      face ? reinterpret_cast<uintptr_t>(face->native) : 0;
  descriptor->typeface = std::move(face);
  return TEXTRA_STATUS_OK;
}
