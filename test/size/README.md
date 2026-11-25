# Advisory package size comparison

Every PR handled by `ci.yml` runs this after performance measurement in the
Android, macOS and Windows jobs. It shares the already downloaded dependencies;
it does not run on a device. Baseline and candidate are exact PR base/head SHAs.
Owned worktrees build both revisions concurrently with a shared job budget.
Android also bounds CMake's Ninja compile/link pools and Gradle workers.

The check warns when `candidate - baseline > 10,240 bytes` (10 KiB), with no
percentage threshold. Equal size, reductions and exactly 10 KiB do not warn.
Growth warnings return success. The build/report/upload steps use
`continue-on-error`, so unavailable measurements also do not fail ordinary CI.
Such errors are explicitly reported as unavailable, never as a size pass.
Existing build, test and performance steps still fail normally.

| Platform | Artifacts measured |
|---|---|
| Android | Actual Release AAR compressed file bytes and each uncompressed `jni/<abi>/*.so`; requires arm64-v8a, armeabi-v7a and x86 Textra libraries |
| macOS | Release `liblynxtextra.dylib`, separately for arm64 and x64, stripped with `xcrun strip -S -x` |
| Windows | Release x64 `lynxtextra.dll`, without PDB/debug files or benchmark code |

Windows currently has no published SDK DLL target. A measurement-only adapter
links `src:capi` and the bidi implementation, using a `.def` generated from each
revision's own public C API declarations. It retains the complete C API instead
of depending on which functions a demo/benchmark calls. This is a native-library
size measurement, not the size of a Windows installer or released SDK package.
The same adapter is supplied to both revisions; production source is unchanged.

Android builds the existing release packaging configuration with JDK 11 and
the repository's pinned SDK/NDK configuration. External dependencies included
in the AAR are reported too. Neither the emulator ABI nor `libperf_host.so`
defines the package size. macOS and Windows measure library file bytes, without
ZIP compression, external Skity libraries or fonts. Different platforms and
architectures have independent thresholds, not a summed cross-platform budget.
macOS uses the bundled libc++ configuration, matching the paired performance
build and preserving the repository's deployment target on newer Apple SDKs.
As in the performance builder, narrow historical compiler diagnostics remain
warnings in both native builds; these flags do not change code generation and
are recorded with the GN arguments. Ordinary CI warning checks stay enabled.

Dependency-changing comparisons reject unequal `DEPS`/`.gn` instead of silently
building the baseline with the wrong dependency set. Configurations, report
JSON, Markdown summary and final libraries/AARs are retained for 14 days.

```sh
python3 test/size/run.py --platform macos \
  --baseline FULL_BASE_SHA --candidate FULL_HEAD_SHA \
  --output /tmp/textra-size-macos
python3 -m unittest discover -s test/size -p 'test_*.py'
```
