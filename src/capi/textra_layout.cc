// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_callback_utils.h"
#include "src/capi/textra_internal.h"
#include "src/textlayout/layout_position.h"
#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/utils/u_8_string.h"

using namespace textra_capi;

textra_status_t TEXTRA_CALL
textra_text_context_create(textra_text_context_t** out_context) {
  if (!out_context) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_context = nullptr;
  *out_context = new textra_text_context();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_text_context_reset(textra_text_context_t* context) {
  if (!context) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || context->active) return TEXTRA_STATUS_BUSY;
  context->impl.Reset();
  context->previous_region.reset();
  context->bottom_explicit = false;
  context->Unbind();
  context->position = 0;
  context->used = false;
  context->failed = false;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_shaper_cache_create(
    textra_bool_t thread_safe, textra_shaper_cache_t** out_cache) {
  if (!out_cache) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_cache = nullptr;
  if (!Bool(thread_safe)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_cache = new textra_shaper_cache{
      tt::TTTextContext::CreateShaperCache(thread_safe != 0)};
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_context_set_shaper_cache(
    textra_text_context_t* context, const textra_shaper_cache_t* cache) {
  if (!context) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || context->active || context->used || *context->results)
    return TEXTRA_STATUS_BUSY;
  context->impl.SetShaperCache(cache ? cache->impl : nullptr);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_context_get_shaper_cache(
    const textra_text_context_t* context, textra_shaper_cache_t** out_cache) {
  if (!out_cache) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_cache = nullptr;
  if (!context) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto cache = context->impl.GetShaperCache();
  if (!cache) return TEXTRA_STATUS_NOT_FOUND;
  *out_cache = new textra_shaper_cache{std::move(cache)};
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_text_context_clear_shape_cache(textra_text_context_t* context) {
  if (!context) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || context->active) return TEXTRA_STATUS_BUSY;
  tt::TextLayout::ClearShapeCache(context->impl);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_text_context_clear_font_cache(textra_text_context_t* context) {
  if (!context) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || context->active) return TEXTRA_STATUS_BUSY;
  tt::TextLayout::ClearFontCache(context->impl);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_context_set_feature(
    textra_text_context_t* context, textra_feature_t feature,
    textra_bool_t enabled) {
  if (!context || !Bool(enabled)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (feature > TEXTRA_FEATURE_ANDROID_METRICS_ROUND)
    return TEXTRA_STATUS_UNSUPPORTED;
  if (context->active || context->used || *context->results)
    return TEXTRA_STATUS_BUSY;
#if !defined(TEXTRA_ENABLE_ANDROID_BACKEND)
  if (feature == TEXTRA_FEATURE_ANDROID_METRICS_ROUND)
    return TEXTRA_STATUS_UNSUPPORTED;
#endif
#if !defined(TEXTRA_ENABLE_OHOS_BACKEND)
  if (feature == TEXTRA_FEATURE_HARMONY_SHAPER_FORCE_LOW_API)
    return TEXTRA_STATUS_UNSUPPORTED;
#endif
  context->impl.EnableFeature(static_cast<tt::FeatureOption>(feature),
                              enabled != 0);
  context->features[feature] = enabled != 0;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_context_get_feature(
    const textra_text_context_t* context, textra_feature_t feature,
    textra_bool_t* out_enabled) {
  if (!context || !out_enabled) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (feature > TEXTRA_FEATURE_ANDROID_METRICS_ROUND)
    return TEXTRA_STATUS_UNSUPPORTED;
#if !defined(TEXTRA_ENABLE_ANDROID_BACKEND)
  if (feature == TEXTRA_FEATURE_ANDROID_METRICS_ROUND)
    return TEXTRA_STATUS_UNSUPPORTED;
#endif
#if !defined(TEXTRA_ENABLE_OHOS_BACKEND)
  if (feature == TEXTRA_FEATURE_HARMONY_SHAPER_FORCE_LOW_API)
    return TEXTRA_STATUS_UNSUPPORTED;
#endif
  *out_enabled = context->features[feature];
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_context_get_position(
    const textra_text_context_t* context, uint32_t* out_codepoint_offset) {
  if (!context || !out_codepoint_offset) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_codepoint_offset = context->position;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_context_get_layout_bottom(
    const textra_text_context_t* context, float* out_bottom) {
  if (!context || !out_bottom) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_bottom = context->impl.GetLayoutBottom();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_context_set_layout_bottom(
    textra_text_context_t* context, float bottom) {
  if (!context || !Finite(bottom)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || context->active) return TEXTRA_STATUS_BUSY;
  context->impl.SetLayoutBottom(bottom);
  context->bottom_explicit = true;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_text_layout_create(const textra_text_layout_options_t* options,
                          textra_text_layout_t** out_layout) {
  if (!out_layout) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_layout = nullptr;
  if (!ValidStruct(options)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto rendering_backend =
      tt::backend::ResolveBackend(TEXTRA_READ(options, backend, uint32_t{0}));
  auto backend = static_cast<uint32_t>(rendering_backend);
  if (TEXTRA_READ(options, flags, uint32_t{0}) ||
      !tt::PlatformHelper::IsBackendAvailable(rendering_backend))
    return TEXTRA_STATUS_UNSUPPORTED;
  auto result = std::make_unique<textra_text_layout>();
  result->backend = backend;
  const textra_font_manager_options_t* inputs[] = {
      TEXTRA_READ(options, default_font_manager,
                  static_cast<const textra_font_manager_options_t*>(nullptr)),
      TEXTRA_READ(options, asset_font_manager,
                  static_cast<const textra_font_manager_options_t*>(nullptr)),
      TEXTRA_READ(options, dynamic_font_manager,
                  static_cast<const textra_font_manager_options_t*>(nullptr)),
      TEXTRA_READ(options, test_font_manager,
                  static_cast<const textra_font_manager_options_t*>(nullptr))};
  const char* family = TEXTRA_READ(options, default_font_family,
                                   static_cast<const char*>(nullptr));
  const auto family_length =
      TEXTRA_READ(options, default_font_family_length, 0u);
  const auto disable_fallback =
      TEXTRA_READ(options, disable_font_fallback, TEXTRA_FALSE);
  if (!Bool(disable_fallback) ||
      (family && (!family_length || !::ttoffice::base::CheckValidUTF8String(
                                        family, family_length, false))) ||
      (!family && family_length))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  // Callback resources stay borrowed until the whole layout is constructed.
  // This makes a failure in any slot leave every host reference untouched.
  std::array<std::shared_ptr<tt::backend::CallbackFontManager>, 4> callbacks;
  for (uint32_t slot = 0; slot < 4; ++slot) {
    if (!inputs[slot] && slot != TEXTRA_FONT_MANAGER_SLOT_DEFAULT) continue;
    textra_font_manager_options_t config{};
    config.struct_size = sizeof(config);
    if (auto input = inputs[slot]) {
      if (!ValidStruct(input)) return TEXTRA_STATUS_INVALID_ARGUMENT;
      config.font_manager =
          TEXTRA_READ(input, font_manager, decltype(config.font_manager){});
      config.manager_callbacks = TEXTRA_READ(
          input, manager_callbacks, decltype(config.manager_callbacks){});
      config.typeface_callbacks = TEXTRA_READ(
          input, typeface_callbacks, decltype(config.typeface_callbacks){});
      config.userdata =
          TEXTRA_READ(input, userdata, decltype(config.userdata){});
    }
    std::shared_ptr<tt::IFontManager> manager;
    if (config.manager_callbacks) {
      if (!tt::backend::CallbackBackend(backend))
        return TEXTRA_STATUS_BACKEND_MISMATCH;
      if (!config.typeface_callbacks) return TEXTRA_STATUS_INVALID_ARGUMENT;
      callbacks[slot] = std::make_shared<tt::backend::CallbackFontManager>(
          config, rendering_backend);
      callbacks[slot]->SetOwnsManager(false);
      manager = callbacks[slot];
    } else {
      manager =
          tt::PlatformHelper::CreateFontManager(config, rendering_backend);
    }
    if (!manager) return TEXTRA_STATUS_UNSUPPORTED;
    switch (slot) {
      case TEXTRA_FONT_MANAGER_SLOT_DEFAULT:
        result->fonts.SetDefaultFontManager(manager);
        break;
      case TEXTRA_FONT_MANAGER_SLOT_ASSET:
        result->fonts.SetAssetFontManager(manager);
        break;
      case TEXTRA_FONT_MANAGER_SLOT_DYNAMIC:
        result->fonts.SetDynamicFontManager(manager);
        break;
      case TEXTRA_FONT_MANAGER_SLOT_TEST:
        result->fonts.SetTestFontManager(manager);
        break;
    }
  }
  if (family)
    result->fonts.SetDefaultSystemFontFamily(
        std::string(family, family_length));
  if (disable_fallback) result->fonts.disableFontFallback();
  result->impl =
      tt::PlatformHelper::CreateTextLayout(rendering_backend, &result->fonts);
  if (!result->impl) return TEXTRA_STATUS_UNSUPPORTED;
  for (auto& callback : callbacks)
    if (callback) callback->SetOwnsManager(true);
  *out_layout = result.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_text_layout_clear_font_cache(textra_text_layout_t* layout) {
  if (!layout) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || layout->active) return TEXTRA_STATUS_BUSY;
  layout->fonts.clearCaches();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_text_layout_preload(textra_rendering_backend_t backend) {
  auto rendering_backend = static_cast<tt::RenderingBackend>(backend);
  return tt::TextLayout::Preload(rendering_backend) ? TEXTRA_STATUS_OK
                                                    : TEXTRA_STATUS_UNSUPPORTED;
}

textra_status_t TEXTRA_CALL
textra_layout_region_create(const textra_layout_region_options_t* options,
                            textra_layout_region_t** out_region) {
  if (!out_region) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_region = nullptr;
  if (!ValidStruct(options)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto result = std::make_unique<textra_layout_region>();
  result->width = TEXTRA_READ(options, width, 0.0f);
  result->height = TEXTRA_READ(options, height, 0.0f);
  result->pitch = TEXTRA_READ(options, line_pitch, 0.0f);
  result->width_mode =
      TEXTRA_READ(options, width_mode, textra_layout_mode_t{0});
  result->height_mode =
      TEXTRA_READ(options, height_mode, textra_layout_mode_t{0});
  if (!Nonnegative(result->width) || !Nonnegative(result->height) ||
      !Nonnegative(result->pitch) || result->width > tt::LAYOUT_MAX_UNITS ||
      result->height > tt::LAYOUT_MAX_UNITS)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (result->width_mode > 2 || result->height_mode > 2)
    return TEXTRA_STATUS_UNSUPPORTED;
  auto table =
      TEXTRA_READ(options, callbacks,
                  static_cast<const textra_layout_page_callbacks_t*>(nullptr));
  if (table) {
    if (!ValidStruct(table)) return TEXTRA_STATUS_INVALID_ARGUMENT;
    if (TEXTRA_READ(table, version, uint32_t{0}) != 1)
      return TEXTRA_STATUS_UNSUPPORTED;
    result->callbacks.struct_size = sizeof(result->callbacks);
    result->callbacks.version = 1;
    result->callbacks.on_line =
        TEXTRA_READ(table, on_line, decltype(table->on_line){nullptr});
    result->callbacks.get_ranges =
        TEXTRA_READ(table, get_ranges, decltype(table->get_ranges){nullptr});
    result->callbacks.destroy =
        TEXTRA_READ(table, destroy, decltype(table->destroy){nullptr});
  }
  result->userdata =
      TEXTRA_READ(options, userdata, static_cast<void*>(nullptr));
  result->impl = std::make_unique<Region>(result.get());
  result->adopted = true;
  *out_region = result.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_layout_region_clear(textra_layout_region_t* region) {
  if (!region) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || region->active) return TEXTRA_STATUS_BUSY;
  auto identity = std::make_shared<int>(0);
  auto next = std::make_unique<Region>(region);
  region->identity = std::move(identity);
  region->impl.swap(next);
  next.reset();
  for (const auto& dep : region->dependencies)
    if (dep->paragraph->metrics_region == region)
      dep->paragraph->valid_layout = false;
  region->dependencies.clear();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_layout_region_get_metrics(const textra_layout_region_t* region,
                                 textra_layout_region_metrics_t* out_metrics) {
  if (!region || !ValidStruct(out_metrics))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  TEXTRA_WRITE(out_metrics, width, region->impl->GetLayoutedWidth());
  TEXTRA_WRITE(out_metrics, height, region->impl->GetLayoutedHeight());
  TEXTRA_WRITE(out_metrics, line_count, region->impl->GetLineCount());
  TEXTRA_WRITE(out_metrics, full,
               static_cast<textra_bool_t>(region->impl->IsFullFilled()));
  TEXTRA_WRITE(out_metrics, exceeded_max_lines,
               static_cast<textra_bool_t>(region->impl->DidExceedMaxLines()));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_layout_layout(
    textra_text_layout_t* layout, textra_paragraph_t* paragraph,
    textra_layout_region_t* region, textra_text_context_t* context,
    textra_layout_flow_t* out_flow) {
  if (!layout || !paragraph || !region || !context || !out_flow)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || layout->active || region->active || context->active)
    return TEXTRA_STATUS_BUSY;
  if (paragraph->impl->GetType() != tt::ParagraphType::kGeneral)
    return TEXTRA_STATUS_UNSUPPORTED;
  if (context->failed) return TEXTRA_STATUS_INVALID_STATE;
  const bool resume = context->locked &&
                      context->paragraph.get() == paragraph &&
                      context->engine.get() == layout;
  if (context->used && !resume) return TEXTRA_STATUS_INVALID_STATE;
  if (paragraph->locks && !resume) return TEXTRA_STATUS_BUSY;
  auto check = CheckFonts(paragraph, layout);
  if (check) return check;
  if (!context->used) {
    context->paragraph = Ref<textra_paragraph>(paragraph);
    context->engine = Ref<textra_text_layout>(layout);
    ++paragraph->locks;
    context->locked = true;
    context->used = true;
  }
  const auto found = std::find_if(
      region->dependencies.begin(), region->dependencies.end(),
      [&](const auto& d) {
        return d->paragraph.get() == paragraph && d->engine.get() == layout;
      });
  if (found == region->dependencies.end())
    region->dependencies.push_back(
        std::make_unique<Dependency>(paragraph, layout, context));
  if (resume && context->previous_region.lock() != region->identity &&
      !context->bottom_explicit)
    context->impl.SetLayoutBottom(0);
  context->previous_region = region->identity;
  context->bottom_explicit = false;
  Active a(layout->active), b(region->active), c(context->active);
  OperationScope scope;
  struct FailureGuard {
    textra_text_context* c;
    bool success = false;
    ~FailureGuard() {
      if (!success) c->failed = true;
    }
  } guard{context};
  Invalidate(paragraph);
  const auto result = layout->impl->LayoutEx(paragraph->impl.get(),
                                             region->impl.get(), context->impl);
  auto* impl = static_cast<tt::ParagraphImpl*>(paragraph->impl.get());
  auto pos = context->impl.GetLayoutPosition();
  context->position =
      impl->LayoutPositionToCharPos(tt::LayoutPosition(pos.first, pos.second));
  if (scope.local.error) return scope.local.error;
  if (result == tt::LayoutResult::kUnsupported)
    return TEXTRA_STATUS_UNSUPPORTED;
  const auto flow = static_cast<textra_layout_flow_t>(result);
  if (result == tt::LayoutResult::kNormal) {
    --paragraph->locks;
    context->locked = false;
  }
  paragraph->valid_layout = true;
  paragraph->metrics_region = region;
  *out_flow = flow;
  guard.success = true;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_layout_layout_single_line(
    textra_text_layout_t* layout, textra_paragraph_t* paragraph,
    textra_text_context_t* context) {
  if (!layout || !paragraph || !context) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || layout->active || context->active || paragraph->locks)
    return TEXTRA_STATUS_BUSY;
  if (context->used || context->failed) return TEXTRA_STATUS_INVALID_STATE;
  if (paragraph->impl->GetType() != tt::ParagraphType::kSingleLine)
    return TEXTRA_STATUS_UNSUPPORTED;
  auto check = CheckFonts(paragraph, layout);
  if (check) return check;
  Active a(layout->active), b(context->active);
  OperationScope scope;
  Invalidate(paragraph);
  const bool laid_out =
      layout->impl->LayoutSingleLine(paragraph->impl.get(), context->impl);
  if (!laid_out) return TEXTRA_STATUS_UNSUPPORTED;
  if (scope.local.error) return scope.local.error;
  paragraph->single_engine = Ref<textra_text_layout>(layout);
  paragraph->valid_layout = true;
  paragraph->single_results = context->results;
  ++*context->results;
  context->position = paragraph->impl->GetCharCount();
  context->used = true;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_line_get_metrics(
    const textra_layout_region_t* region, uint32_t line_index,
    textra_text_line_metrics_t* out_metrics) {
  if (!region || !ValidStruct(out_metrics))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto* line = region->impl->GetLine(line_index);
  if (!line) return TEXTRA_STATUS_OUT_OF_RANGE;
  TEXTRA_WRITE(out_metrics, start_char, line->GetStartCharPos());
  TEXTRA_WRITE(out_metrics, end_char, line->GetEndCharPos());
  TEXTRA_WRITE(out_metrics, top, line->GetLineTop());
  TEXTRA_WRITE(out_metrics, baseline, line->GetLineBaseLine());
  TEXTRA_WRITE(out_metrics, bottom, line->GetLineBottom());
  TEXTRA_WRITE(out_metrics, left, line->GetLineLeft());
  TEXTRA_WRITE(out_metrics, right, line->GetLineRight());
  TEXTRA_WRITE(out_metrics, content_top, line->GetContentTop());
  TEXTRA_WRITE(out_metrics, content_bottom, line->GetContentBottom());
  TEXTRA_WRITE(out_metrics, ascent, line->GetMaxAscent());
  TEXTRA_WRITE(out_metrics, descent, line->GetMaxDescent());
  TEXTRA_WRITE(out_metrics, start_indent, line->GetStartIndent());
  TEXTRA_WRITE(out_metrics, end_indent, line->GetEndIndent());
  TEXTRA_WRITE(out_metrics, last_line_in_paragraph,
               static_cast<textra_bool_t>(line->IsLastLineOfParagraph()));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_line_get_paragraph(
    const textra_layout_region_t* region, uint32_t line_index,
    textra_paragraph_t** out_paragraph) {
  if (!out_paragraph) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_paragraph = nullptr;
  if (!region) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto* line = region->impl->GetLine(line_index);
  if (!line) return TEXTRA_STATUS_OUT_OF_RANGE;
  for (const auto& dep : region->dependencies)
    if (dep->paragraph->impl.get() == line->GetParagraph()) {
      Retain(dep->paragraph.get());
      *out_paragraph = dep->paragraph.get();
      return TEXTRA_STATUS_OK;
    }
  return TEXTRA_STATUS_INTERNAL_ERROR;
}

textra_status_t TEXTRA_CALL textra_text_line_get_bounds(
    const textra_layout_region_t* region, uint32_t line_index,
    const textra_text_range_t* range, textra_bool_t tight,
    textra_rect_t* out_bounds) {
  if (!region || !out_bounds || !Bool(tight))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto* line = region->impl->GetLine(line_index);
  if (!line) return TEXTRA_STATUS_OUT_OF_RANGE;
  auto start = range ? range->start : line->GetStartCharPos();
  auto end = range ? range->end : line->GetEndCharPos();
  if (start > end || start < line->GetStartCharPos() ||
      end > line->GetEndCharPos())
    return TEXTRA_STATUS_OUT_OF_RANGE;
  float bounds[4];
  if (tight) {
    line->GetTightBoundingRectByCharRange(bounds, start, end);
    *out_bounds = {bounds[0], bounds[1] + line->GetLineBaseLine(), bounds[2],
                   bounds[3] + line->GetLineBaseLine()};
  } else {
    line->GetBoundingRectByCharRange(bounds, start, end);
    *out_bounds = {bounds[0], bounds[1], bounds[0] + bounds[2],
                   bounds[1] + bounds[3]};
  }
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_text_line_hit_test(
    const textra_layout_region_t* region, uint32_t line_index, float x,
    uint32_t* out_char_index) {
  if (!region || !out_char_index || !Finite(x))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto* line = region->impl->GetLine(line_index);
  if (!line) return TEXTRA_STATUS_OUT_OF_RANGE;
  auto index = line->GetCharPosByCoordinateX(x) + line->GetStartCharPos();
  *out_char_index =
      std::max(line->GetStartCharPos(), std::min(index, line->GetEndCharPos()));
  return TEXTRA_STATUS_OK;
}
