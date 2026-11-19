// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_H_

#include <textra/capi/textra_backend.h>
#include <textra/capi/textra_canvas.h>
#include <textra/capi/textra_common.h>
#include <textra/capi/textra_font.h>
#include <textra/capi/textra_font_manager.h>
#include <textra/capi/textra_gradient.h>
#include <textra/capi/textra_layout_drawer.h>
#include <textra/capi/textra_layout_page_listener.h>
#include <textra/capi/textra_layout_region.h>
#include <textra/capi/textra_paragraph.h>
#include <textra/capi/textra_paragraph_style.h>
#include <textra/capi/textra_platform.h>
#include <textra/capi/textra_run_delegate.h>
#include <textra/capi/textra_style.h>
#include <textra/capi/textra_text_context.h>
#include <textra/capi/textra_text_layout.h>
#include <textra/capi/textra_text_line.h>
#include <textra/capi/textra_typeface.h>
#include <textra/capi/textra_types.h>

/* C-only facade declarations. C++ users may include this header too; symbols
 * have C linkage. See textra_common.h for ABI and lifetime rules.
 * Typical order: create manager/collection -> configure styles/paragraph ->
 * create engine/context/region -> layout -> query/draw -> destroy resources.
 * Low-level ICU/shaper, arbitrary Painter/Path classes, platform concrete
 * classes remain outside the C boundary; host backends register C tables.
 * Existing C++ interfaces and factories remain independent and unchanged.
 */

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_H_
