// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef TEST_CAPI_CANVAS_TEST_CALLS_H_
#define TEST_CAPI_CANVAS_TEST_CALLS_H_
#include <textra/capi/textra_canvas.h>
/* Private case fixtures for table-driven tests and demo comparisons only.
 * Production code has no operation IDs, command envelope or dispatcher. */
typedef uint32_t textra_test_canvas_operation_t;
#define TEXTRA_TEST_CANVAS_START_PAINT ((textra_test_canvas_operation_t)0)
#define TEXTRA_TEST_CANVAS_END_PAINT ((textra_test_canvas_operation_t)1)
#define TEXTRA_TEST_CANVAS_SAVE ((textra_test_canvas_operation_t)2)
#define TEXTRA_TEST_CANVAS_RESTORE ((textra_test_canvas_operation_t)3)
#define TEXTRA_TEST_CANVAS_TRANSLATE ((textra_test_canvas_operation_t)4)
#define TEXTRA_TEST_CANVAS_SCALE ((textra_test_canvas_operation_t)5)
#define TEXTRA_TEST_CANVAS_ROTATE ((textra_test_canvas_operation_t)6)
#define TEXTRA_TEST_CANVAS_SKEW ((textra_test_canvas_operation_t)7)
#define TEXTRA_TEST_CANVAS_CLIP_RECT ((textra_test_canvas_operation_t)8)
#define TEXTRA_TEST_CANVAS_CLEAR ((textra_test_canvas_operation_t)9)
#define TEXTRA_TEST_CANVAS_CLEAR_RECT ((textra_test_canvas_operation_t)10)
#define TEXTRA_TEST_CANVAS_FILL_RECT ((textra_test_canvas_operation_t)11)
#define TEXTRA_TEST_CANVAS_DRAW_COLOR ((textra_test_canvas_operation_t)12)
#define TEXTRA_TEST_CANVAS_DRAW_LINE ((textra_test_canvas_operation_t)13)
#define TEXTRA_TEST_CANVAS_DRAW_RECT ((textra_test_canvas_operation_t)14)
#define TEXTRA_TEST_CANVAS_DRAW_OVAL ((textra_test_canvas_operation_t)15)
#define TEXTRA_TEST_CANVAS_DRAW_CIRCLE ((textra_test_canvas_operation_t)16)
#define TEXTRA_TEST_CANVAS_DRAW_ARC ((textra_test_canvas_operation_t)17)
#define TEXTRA_TEST_CANVAS_DRAW_PATH ((textra_test_canvas_operation_t)18)
#define TEXTRA_TEST_CANVAS_DRAW_ARC_TO ((textra_test_canvas_operation_t)19)
#define TEXTRA_TEST_CANVAS_DRAW_TEXT ((textra_test_canvas_operation_t)20)
#define TEXTRA_TEST_CANVAS_DRAW_GLYPHS ((textra_test_canvas_operation_t)21)
#define TEXTRA_TEST_CANVAS_DRAW_IMAGE ((textra_test_canvas_operation_t)24)
#define TEXTRA_TEST_CANVAS_DRAW_IMAGE_RECT ((textra_test_canvas_operation_t)25)
#define TEXTRA_TEST_CANVAS_DRAW_ROUND_RECT ((textra_test_canvas_operation_t)26)
/* values: method arguments in ICanvasHelper order (all geometry as float).
 * flags: antialias for CLIP_RECT, useCenter for DRAW_ARC, srcRectPercent for
 * DRAW_IMAGE_RECT. color is used by FILL_RECT/DRAW_COLOR. text is UTF-8 for
 * DRAW_TEXT/GLYPHS and a NUL-terminated image source for image operations.
 * typeface is this provider's own opaque typeface object. x/y are call-local
 * glyph positions; a backend may modify them, never retained layout results.
 */
typedef struct textra_test_canvas_call {
  uint32_t struct_size;
  textra_test_canvas_operation_t operation;
  float values[8];
  uint32_t flags, color;
  const textra_backend_paint_t* paint;
  void* typeface;
  const uint16_t* glyphs;
  uint32_t glyph_count;
  const char* text;
  uint32_t text_bytes;
  float* x;
  float* y;
  const textra_backend_path_t* path;
} textra_test_canvas_call_t;

#ifdef __cplusplus
#include "src/capi/textra_internal.h"

/* Probe native SDK callbacks directly. Actual I* rendering is exercised by
 * LayoutDrawer parity tests; this is not a public drawing API. */
static inline textra_status_t textra_test_canvas_call(
    textra_canvas_t* canvas, const textra_test_canvas_call_t* call,
    const textra_typeface_t* typeface) {
  auto* adapter = static_cast<ttoffice::tttext::backend::CallbackCanvas*>(
      canvas->impl.get());
  const auto& callbacks = adapter->Callbacks();
  void* userdata = adapter->Userdata();
  void* native = adapter->NativeObject();
  void* font = typeface
                   ? static_cast<ttoffice::tttext::backend::CallbackTypeface*>(
                         typeface->impl.get())
                         ->NativeObject()
                   : nullptr;
  switch (call->operation) {
    case 0:
      if (auto callback = callbacks.start_paint) callback(userdata, native);
      break;
    case 1:
      if (auto callback = callbacks.end_paint) callback(userdata, native);
      break;
    case 2:
      if (auto callback = callbacks.save) callback(userdata, native);
      break;
    case 3:
      if (auto callback = callbacks.restore) callback(userdata, native);
      break;
    case 4:
      if (auto callback = callbacks.translate)
        callback(userdata, native, call->values[0], call->values[1]);
      break;
    case 5:
      if (auto callback = callbacks.scale)
        callback(userdata, native, call->values[0], call->values[1]);
      break;
    case 6:
      if (auto callback = callbacks.rotate)
        callback(userdata, native, call->values[0]);
      break;
    case 7:
      if (auto callback = callbacks.skew)
        callback(userdata, native, call->values[0], call->values[1]);
      break;
    case 8:
      if (auto callback = callbacks.clip_rect)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->flags);
      break;
    case 9:
      if (auto callback = callbacks.clear) callback(userdata, native);
      break;
    case 10:
      if (auto callback = callbacks.clear_rect)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3]);
      break;
    case 11:
      if (auto callback = callbacks.fill_rect)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->color);
      break;
    case 12:
      if (auto callback = callbacks.draw_color)
        callback(userdata, native, call->color);
      break;
    case 13:
      if (auto callback = callbacks.draw_line)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->paint);
      break;
    case 14:
      if (auto callback = callbacks.draw_rect)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->paint);
      break;
    case 15:
      if (auto callback = callbacks.draw_oval)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->paint);
      break;
    case 16:
      if (auto callback = callbacks.draw_circle)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->paint);
      break;
    case 17:
      if (auto callback = callbacks.draw_arc)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->values[4],
                 call->values[5], call->flags, call->paint);
      break;
    case 18:
      if (auto callback = callbacks.draw_path)
        callback(userdata, native, call->path, call->paint);
      break;
    case 19:
      if (auto callback = callbacks.draw_arc_to)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->values[4],
                 call->values[5], call->values[6], call->paint);
      break;
    case 20:
      if (auto callback = callbacks.draw_text)
        callback(userdata, native, font, call->text, call->text_bytes,
                 call->values[0], call->values[1], call->paint);
      break;
    case 21:
      if (auto callback = callbacks.draw_glyphs)
        callback(userdata, native, font, call->glyph_count, call->glyphs,
                 call->text, call->text_bytes, call->values[0], call->values[1],
                 call->x, call->y, call->paint);
      break;
    case 24:
      if (auto callback = callbacks.draw_image)
        callback(userdata, native, call->text, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->paint);
      break;
    case 25:
      if (auto callback = callbacks.draw_image_rect)
        callback(userdata, native, call->text, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->values[4],
                 call->values[5], call->values[6], call->values[7], call->paint,
                 call->flags);
      break;
    case 26:
      if (auto callback = callbacks.draw_round_rect)
        callback(userdata, native, call->values[0], call->values[1],
                 call->values[2], call->values[3], call->values[4],
                 call->paint);
      break;
    default:
      return TEXTRA_STATUS_UNSUPPORTED;
  }
  return TEXTRA_STATUS_OK;  // ICanvasHelper drawing methods are void.
}
#endif  // __cplusplus
#endif  // TEST_CAPI_CANVAS_TEST_CALLS_H_
