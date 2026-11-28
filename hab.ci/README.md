# Skity release SDKs

Tests and demos consume **Skity v1.1.0-alpha.7** release binaries. `skity.json` pins
both platforms' URLs and SHA-256 digests. No Skity sources or codec dependencies
are fetched or compiled by Textra. Updating this lock is an explicit SDK upgrade.

Run from the repository root after the usual core dependency setup:

```sh
# macOS software-rendering tests and Metal/GL demos
./tools/hab sync . --target-only --target skity-macos
# Android demos (also required before configuring the Android Gradle project)
./tools/hab sync . --target-only --target skity-android
```

Habitat verifies each archive and uses its existing HTTP cache. CI/CQ uses these
same commands. Missing SDKs cause a build configuration error; there is no
source-build fallback. The third_party/skity_release directory is ignored.

The Apple XCFramework includes arm64/x86_64 macOS and iOS device/simulator
slices. Its macOS binaries require macOS 15.0. This deployment requirement is
applied to Skity consumers, not the standalone Textra core. CMake imports the
framework through cmake/SkityRelease.cmake. Test/demo PNG output uses ImageIO;
the release framework does not include Skity's optional codec module.

Android locks the exact `20261124.045156-1` alpha.7 AAR, rather than resolving
`SNAPSHOT` metadata at build time. The AAR includes armeabi-v7a, arm64-v8a, x86
and x86_64. Gradle extracts its C API headers and shared libraries into the demo's
build directory. CMake imports `libskity-capi.so` directly and Gradle packages its
runtime libraries; the demo keeps its existing STL configuration. This avoids the
release's Prefab restriction on STL consumers: the SDK uses a private static STL,
while the integration passes only C handles, POD values and caller-owned buffers.
No Skity C++ SDK objects cross that boundary.

The C API adapters retain imported typefaces and borrow host canvas handles.
The alpha.7 C API has no custom logger registration entry point, so the Android
host no longer installs its C++ logger callbacks. Host JNI/renderer logs remain.
alpha.7 provides cached/uncached CTFontRef import and the adapters use it.
The reverse `skity_typeface -> CTFontRef` operation is still missing from the C
API. Full SKITY_CORETEXT manager/typeface initializers therefore return
UNSUPPORTED without acquiring inputs; canvas callbacks remain usable. The
standalone full-backend test exits 77 (skip) for this explicit dependency gap.
No font is reconstructed by family name and no C++ SDK bridge is used.

Both C API host adapters share one implementation for drawing, matching and
metrics. The C++ adapters under `public/textra/platform/skity` remain unchanged
and continue using Skity's C++ SDK. Ordinary macOS demos and image/gradient tests
retain that C++ path. Their explicit C API demo mode renders to a C API software
bitmap and presents an owned image through the existing C++ GPU canvas, since
this SDK exposes no C/C++ canvas-handle conversion. Android's C API host uses
Textra's existing callback factories for its C++ layout/draw interfaces.
Obsolete Android sources outside src/main/jni are removed; CMake compiles only
the migrated src/main/jni sources.

Compared with v1.0.0, alpha.6 changes the Inter rendering used by
`font_collection.png` and `align_with_bbox.png`. Their reviewed baselines are
updated for the fixed SDK. An isolated run with the previous C++ adapter and the
same alpha.6 SDK produces identical pixels for these cases.

## Remaining SDK gaps

- Required for full Skity/CoreText shaping: exporting CTFontRef from an opaque
  skity_typeface. Imported native fonts work; native-font export is unavailable.
- Optional diagnostics: C equivalent of Logger::RegisterLog.
- Distribution: alpha.7 release assets include Apple and Android SDKs, but no
  Windows binary. This is not a missing C API symbol.

The default macOS targets build the Metal demo and software tests. Configure
with `-DTEXTRA_BUILD_GL_DEMO=ON` to also build `gl_app_demo`. The Apple alpha.7
binary reports OpenGL and Metal enabled; Vulkan is not enabled in this package.

Windows performance CI is temporarily disabled because alpha.7 has no published
Windows SDK. Its GN/Ninja Release core compile check and advisory package-size
comparison remain enabled. `DEPS.windows-ci` contains no Skity source dependency.
Restore performance coverage when a pinned Windows binary SDK is available.

macOS performance hosts use `--skity-framework` with the same immutable release
SDK for both engine revisions. Engine/toolchain pins must still match; only
Skity source entries are excluded from that comparison. The gate compares Textra
engine changes under one renderer SDK, rather than measuring the SDK upgrade.
Both engines and the host use system libc++ to match the release C++ ABI.

## Skia Linux x64 prebuilt

The Linux Skia demo/check uses Brooooooklyn/canvas release `skia-0f81dc8b`,
Skia m130 commit `0f81dc8b2e43c060972a8d8d229b346694a674f8` (on chrome/m130).
`skia.json` pins every URL and SHA-256. No Skia source build or git dependency
sync is performed; the library is a third-party release artifact. Matching
headers and the license are extracted from a checksum-pinned GitHub archive of
the same commit. The compressed archive includes the upstream tree, but only
`include/`, two skcms headers and the license are extracted; no Skia `.cc` files
are placed in the SDK or compiled.

```sh
./tools/hab sync . --target-only --target skia-linux
source tools/envsetup.sh
cmake -S . -B cmake-build-skia -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DTEXTRA_ENABLE_SKIA=ON \
  -DCMAKE_C_COMPILER="$PWD/buildtools/llvm/bin/clang" \
  -DCMAKE_CXX_COMPILER="$PWD/buildtools/llvm/bin/clang++"
cmake --build cmake-build-skia --target ttreaderdemo
```

The binary uses libc++18, so both Textra and its C++ consumer use the matching
headers and static runtime, supplied by three SHA-256-pinned LLVM 18.1.8 Ubuntu
Jammy development packages. CMake extracts these locally with `dpkg-deb`; it
installs no system packages and builds no C++ runtime sources. Use the pinned
Clang toolchain and an Ubuntu 22.04-compatible Linux x64 host. GCC/libstdc++,
other architectures, and other platforms are rejected rather than silently
falling back to a source build. Downloads/extracted artifacts remain ignored
under `third_party/skia_release`.

Upstream artifact/build configuration:
https://github.com/Brooooooklyn/canvas/releases/tag/skia-0f81dc8b
https://github.com/Brooooooklyn/canvas/blob/skia-0f81dc8b/scripts/build-skia.js

`Linux (Skia) / Build` also runs `SkiaPrebuiltSmoke`, checking retained font
ownership, glyph metrics, nonempty repeated Textra canvas draws, and PNG encoding.
Checkout enables Git LFS for the real font fixtures. The Linux reader demo uses
the repository's Noto CJK font directory instead of the nonexistent macOS system
font directory. This is a smoke check, not full cross-version visual parity.
