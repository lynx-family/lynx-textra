# Skity with CoreText

Use `TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT` for CoreText shaping and Skity
rendering on iOS or macOS. Both use the same C11 header-only interface:

```c
#include <textra/capi/platform/textra_capi_skity_coretext.h>

/* Set struct_size and a borrowed skity_canvas, then check the status:
 * textra_skity_coretext_canvas_callbacks_initialize(&canvas_options);
 */
```

Add Textra's `public` and Skity's SDK to the include paths. Keep `skity_common/` alongside it. Link the Skity C API binary and Textra
core; no adapter source, implementation macro or platform macro is required.
The optional CMake `textra_capi_skity_coretext` target carries these requirements.

Skity alpha.7 supports `CTFontRef` import but cannot export a typeface back to
`CTFontRef`. The complete manager/typeface bridge therefore remains unavailable:
its two initializers return `TEXTRA_STATUS_UNSUPPORTED` without changing options
or acquiring inputs. Canvas initialization and callbacks are available; Canvas
is borrowed and destruction does not release it.

See the [host build guide](../README.md) and
[C API guide](../../README.md).
