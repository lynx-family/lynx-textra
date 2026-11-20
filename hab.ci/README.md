# Skity release SDKs

Tests and demos consume **Skity v1.0.0** release binaries. `skity.json` pins
both platforms' URLs and SHA-256 digests. CI dependency syncs do not fetch or compile Skity sources or codec dependencies. Updating this lock is an explicit SDK upgrade.

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

Android uses the release AAR's Prefab skity-native::skity target. v1.0.0 uses
c++_shared and includes armeabi-v7a, arm64-v8a, x86 and x86_64. Keep the demo's
STL setting compatible with that SDK.

Windows performance CI is temporarily disabled because v1.0.0 has no published
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
