// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_SKSHAPER_HARFBUZZ_LIB_H_
#define SRC_PORTS_SHAPER_SKSHAPER_HARFBUZZ_LIB_H_

#include <hb-ot.h>
#include <hb.h>

#include <memory>

namespace ttoffice {
namespace tttext {

#define HB_EMIT_FUNCS_TYPE1                        \
  HB_FUNC(hb_font_create)                          \
  HB_FUNC(hb_font_funcs_create)                    \
  HB_FUNC(hb_font_funcs_set_variation_glyph_func)  \
  HB_FUNC(hb_font_funcs_set_nominal_glyph_func)    \
  HB_FUNC(hb_font_funcs_set_nominal_glyphs_func)   \
  HB_FUNC(hb_font_funcs_set_glyph_h_advance_func)  \
  HB_FUNC(hb_font_funcs_set_glyph_h_advances_func) \
  HB_FUNC(hb_font_funcs_set_glyph_extents_func)    \
  HB_FUNC(hb_font_funcs_make_immutable)            \
  HB_FUNC(hb_font_create_sub_font)                 \
  HB_FUNC(hb_font_set_funcs)                       \
  HB_FUNC(hb_font_set_scale)                       \
  HB_FUNC(hb_ot_font_set_funcs)                    \
                                                   \
  HB_FUNC(hb_face_create)                          \
  HB_FUNC(hb_face_set_index)                       \
  HB_FUNC(hb_face_set_upem)                        \
                                                   \
  HB_FUNC(hb_blob_create)                          \
  HB_FUNC(hb_blob_make_immutable)                  \
                                                   \
  HB_FUNC(hb_script_from_iso15924_tag)             \
  HB_FUNC(hb_language_from_string)                 \
                                                   \
  HB_FUNC(hb_buffer_set_content_type)              \
  HB_FUNC(hb_buffer_set_cluster_level)             \
  HB_FUNC(hb_buffer_add_utf32)                     \
  HB_FUNC(hb_buffer_add)                           \
  HB_FUNC(hb_buffer_set_direction)                 \
  HB_FUNC(hb_buffer_set_script)                    \
  HB_FUNC(hb_buffer_set_language)                  \
  HB_FUNC(hb_buffer_guess_segment_properties)      \
  HB_FUNC(hb_buffer_get_length)                    \
  HB_FUNC(hb_buffer_reverse)                       \
  HB_FUNC(hb_buffer_get_glyph_infos)               \
  HB_FUNC(hb_buffer_get_glyph_positions)           \
  HB_FUNC(hb_buffer_create)                        \
                                                   \
  HB_FUNC(hb_shape)

#define HB_EMIT_FUNCS_TYPE2  \
  HB_FUNC(hb_font_destroy)   \
  HB_FUNC(hb_face_destroy)   \
  HB_FUNC(hb_blob_destroy)   \
  HB_FUNC(hb_buffer_destroy) \
  HB_FUNC(hb_buffer_clear_contents)

#define HB_FUNC(func_name) decltype(func_name)* f_##func_name;

struct HarfbuzzLib {
  static bool HasHarfbuzzLib();
  static const HarfbuzzLib* GetHarfbuzzLib();
  static std::unique_ptr<HarfbuzzLib> LoadHarfbuzzLib();

  HB_EMIT_FUNCS_TYPE1
  HB_EMIT_FUNCS_TYPE2
};

#undef HB_FUNC

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_SKSHAPER_HARFBUZZ_LIB_H_
