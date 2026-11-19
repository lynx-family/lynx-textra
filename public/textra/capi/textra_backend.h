// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_BACKEND_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_BACKEND_H_

#include <textra/capi/textra_canvas.h>
#include <textra/capi/textra_font_manager.h>
#include <textra/capi/textra_typeface.h>

/* Callback tables are size/version tagged and copied during construction.
 * Each object carries its own callbacks and userdata; no backend registration
 * or global provider exists. Callback modules and borrowed userdata must remain
 * alive until every dependent object/cache has been released. Object ownership
 * transfers only on successful construction; destroy releases it exactly once.
 * Callers supply valid native objects, options and required callbacks; private
 * adapters do not validate individual calls. Matching callbacks transfer one
 * owned face on success; NULL means no match. Calls are synchronous and must
 * not throw. Append optional callback fields; never reorder fields or grow
 * frozen drawing views/array elements.
 */
#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_BACKEND_H_
