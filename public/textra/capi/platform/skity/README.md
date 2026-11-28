# Skity host adapter

Use `TEXTRA_RENDERING_BACKEND_SKITY` for Skity rendering with HarfBuzz shaping.
Include `textra/capi/platform/textra_capi_skity.h` with Textra's
`public` directory on the include path. This is a C11 header-only implementation, also usable from C++17. Link the
Skity C API binary and the Textra core; no adapter `.cc` is compiled.

Canvas is a borrowed `skity_canvas`. An optional manager input is a borrowed
`skity_font_manager`; a NULL input creates an owned default manager. Native
font input is a `skity_typeface` retained by initialization. Adopt owned records
once, or destroy them if engine initialization fails. Borrowed managers, callback
code and userdata must outlive dependent objects/caches.

Keep `skity_common/` beside `skity/` when copying headers into a host. See the
[host build guide](../README.md). Skity with CoreText shaping has a separate
entry header under [skity_coretext](../skity_coretext/README.md).
