# Paired shaping performance gate

This measures `ShapeText`, result cleanup, and their directly timed total. It
never enables ShapeCache or includes build, font loading, IPC, layout or draw
cost in the timings. The same 33 workloads cover Latin, combining marks, Han,
Arabic, Hebrew, Thai, Indic scripts, Japanese, Korean, Emoji and script mixing.

## Gate contract

- Build the PR base SHA and head SHA separately, using the **same current
  benchmark source/header and workload manifest**. A base revision does not need
  to contain `test/perf`. Require full 40-character revision IDs.
- Run both on one device with identical OS, architecture, backend, font data,
  toolchain and dependency versions. Record executable/metadata/shared-library/
  fixed-font SHA-256 checksums; verify artifacts again after measurement.
- Eight serial paired rounds alternate baseline/candidate launch order and rotate
  workload order. Defaults: 20 warmups, 6 batches of 100 calls per response.
  Process setup is outside the timing; each response uses a fresh font
  collection/shaper over retained font objects in one process per version/round. This is warm-font, cache-disabled shaping, not cold startup.
- Report separate shape/cleanup/total medians. Gate on the median **paired total
  change** only: greater than 5%, greater than 1 microsecond absolute, and slower
  in at least 7/8 pairs. Independent phase medians are never added together.
- Changed glyphs, font assignment, cluster maps, advances or positions; missing
  glyphs; stale responses; wrong revisions; changing identity; adapter failures;
  and timeouts fail the gate separately from a timing regression. Correctness
  improvements require review too; there are no automatic output exemptions.
- A failed case/version is not retried every round. Timeouts terminate its local
  adapter process group. Device adapters must also stop outstanding device work
  before returning on error; otherwise subsequent measurements are invalid.

`results/report.json`, `summary.md`, raw responses, stderr and input/configuration
checksums are retained. Build errors fail the job before performance reporting.
A missing device/toolchain is **not** a passing performance result.

## macOS automatic CQ

`.github/workflows/ci.yml` runs on PRs and builds the exact PR base and
head in separate GN output directories. It uses existing macOS runner labels,
Release `-Oz`, bundled libc++, and bundled HarfBuzz. The base checkout is a
short-lived owned worktree; engine headers come from the matching checkout.
Fetched dependencies are shared and `DEPS`/`.gn` must match. Dependency-changing
PRs fail explicitly instead of silently comparing incorrect dependency pins.

One macOS job runs native CoreText and Skity/SkShaper serially using the same
pair of benchmark executables. The benchmark is linked with Skity once per
revision and selects the backend per request. The macOS job reuses the shared
Skity library built for the demo and unit tests and uses `fonts.macos.json` for both engines. System-font files must
exist on the runner; their file checksums are recorded, with no silent font
substitution. Downloaded/generated dependencies are not committed. macOS host
architecture is detected instead of hard-coding arm64. The macOS job installs
native CMake 3.31.6 in an isolated environment, since the repository's Intel
CMake can otherwise select the wrong target and libpng processor sources on
Apple Silicon. Skity and its subprojects receive matching architecture flags.
For historical revisions,
the timing builder keeps integer-narrowing/nontrivial-memory-call warnings as
warnings in both generated builds if supported by that compiler. It changes no
engine source or optimization flag; the exact compatibility flags are recorded
in each `.build.json`. Existing build/static CQ still enforces compiler warnings.

```sh
python3 test/perf/run_macos.py \
  --baseline FULL_BASE_SHA --candidate FULL_HEAD_SHA \
  --backend macos --output /tmp/textra-performance
```

For Skity, also pass `--backend skity --skity-library /path/libskity.dylib
--fonts test/perf/fonts.macos.json`. An explicit `--harfbuzz-library` can select
a fixed dynamic library for local toolchain validation; its hash and changed GN
arguments are recorded for both builds. Automatic CQ uses bundled HarfBuzz.

Historical validation of PR #195 against its pre-Script base reports Japanese
and Indic mixed-script timing regressions, an RTL Emoji output repair, and a
baseline timeout on mixed marks. These are useful negative controls: the strict
gate reports them without output exemptions or wider thresholds. New PRs compare
against their own current base, which already includes merged changes. A/A
controls can calibrate a runner via `aa_control: true` and matching revisions;
they must not be mistaken for a PR comparison.

## Platform CQ configuration

The workflow reuses Lynx develop's existing runner labels and platform setup:

| Platform | Runner | Execution |
|---|---|---|
| macOS | `lynx-darwin-15.6.1-medium` | CoreText and Skity paired gates |
| iOS | `lynx-darwin-15.6.1-medium` | Two native test apps on one owned simulator |
| Android | `lynx-ubuntu-22.04-physical-medium` | Native Java/JNI host on the runner's AVD |
| Windows | `lynx-windows-2022-large` | Skity executable and fixed fonts |
| Harmony | `lynx-custom-container` | Two benchmark NAPI libraries built in the Lynx SDK image; no runtime result |

Build checks and performance gates share one job per platform, including one
checkout and dependency/toolchain setup. macOS Debug remains separate from
Release, but the Release library and C API tests reuse one GN directory. The
Metal demo and CPU/golden tests share a Metal-enabled CMake tree. Android demos
build the emulator ABI via `-PabiList=...` under JDK 11, without a retained
Gradle daemon. SDK tools and benchmark packaging use JDK 17. iOS demos use
`iphonesimulator` on the
runner's host architecture. Linux Skia and LVGL configurations remain separate.

Both revisions are checked out serially, then compiled concurrently in isolated
output directories. `--jobs` specifies the **total** Ninja budget (default:
available CPUs, capped at eight), divided between the two builds. A single-job
budget falls back to serial compilation. Even if one build fails, both compiler
workers are joined before cleanup. Packaging, deployment, functional tests and
performance sampling do not overlap with these compilers. CoreText and Skity
comparisons are serial; baseline/candidate sampling still alternates within each.

macOS `--backend both` writes `config.macos.json`, `config.skity.json`, and
`results/{macos,skity}/`, all referencing one baseline/candidate executable pair.
Single-backend CLI invocations retain `config.json` and `results/`.

PRs that touch only demos still run build checks; performance runs for engine,
backend, benchmark, dependency or workflow changes. Manual dispatch selects a
platform and runs its paired gate, without rebuilding unrelated demos.

These are fixed workflow jobs: no `TEXTRA_PERF_RUNNERS` or
`TEXTRA_PERF_BUILDERS` repository variables or externally installed builders are
required. `run_platform.py` and the test hosts under `hosts/` are repository-owned.
PR jobs build the exact PR base/head; manual dispatch accepts a full baseline SHA
and a platform (or `all`), using the selected workflow ref as the candidate.

`run_platform.py` makes two owned worktrees. It overlays the same benchmark and
host into each and adds an owned root GN group to make the new test target
reachable. Engine sources are unchanged. Both versions use one dependency tree
and equal `DEPS`/`.gn`; source revision, harness, GN options, host code and
measured artifacts are recorded. Worktrees and owned runtime processes are
cleaned up even on failure.

### iOS

The runner must have Xcode and an available iPhone simulator runtime, as in
Lynx's iOS test jobs. The driver discovers an available runtime/device type and
creates its own simulator; no UDID or production signing credential is required.
Two ad-hoc-signed test apps with run-specific bundle IDs are installed on the same simulator and removed during cleanup. Their Scene
lifecycle supports current iOS runtimes. Requests and responses use app-sandbox
JSON files; transport/activation time is excluded from timing. `--device UDID`
can use an existing simulator for local verification; such a simulator is not
deleted. This is simulator performance, not physical-iPhone performance.

### Android

The existing Lynx physical Linux *host* runner supplies Android SDK/AVD support;
its `physical` label does not mean a phone device pool. The workflow installs
SDK 35 build tools and NDK 21.1, starts one AVD on an unused owned port, and waits
for `sys.boot_completed`. Both APKs execute on that exact serial; an unrelated
adb device cannot satisfy the boot check. The owned emulator is stopped in an
`always()` cleanup step.

A small framework-only Activity initializes the real Textra Java/JNI runtime
and invokes the same C++ benchmark. Packaging uses SDK `aapt`, `d8`, `zipalign`
and `apksigner`, with an ephemeral test keystore and run-specific application IDs, instead of a second Gradle
project. SDK/NDK/Java classes come from the matching source revision. The native-only Android and Harmony test hosts disable the unused SkShaper
port in both builds (recorded in GN arguments). Android avoids duplicate ICU
wrapper objects in historical GN source sets; the Harmony host explicitly links
the static system-ICU wrapper needed by ArkGraphics. No Appium
server or external builder installation is needed. Android's existing JavaShaper
exposes UTF-16 text units and widths rather than actual font glyph IDs. Its output
signature therefore verifies those units, maps and metrics; a zero missing count
is not proof of Android system-font coverage. `glyph_semantics` records this
scope explicitly.

### Windows

The runner supplies the same VS 2022 BuildTools and Windows SDK as Lynx Windows
builds. The workflow exports the installation paths, installs CMake 3.31.6, and
uses Python 3.11 with UTF-8 mode for the pinned GN scripts. Git LFS 3.7.1 and
native Windows Habitat 0.3.151 are downloaded with fixed SHA-256 checksums; the
Unix Habitat launcher is not used. Ninja is exposed to the Windows compiler
wrappers. One Release Skity build and fixed font files are shared by both engines.
Repository fonts are supplemented by the Noto files in `fonts.lock.json`, pinned
to a full google/fonts commit and individual SHA-256 checksums. Missing Windows
language packs therefore do not change the test set. Downloads stay in the
owned output directory; fonts are never silently substituted. Source/license
URLs are retained in the lock file. This measures SkShaper,
since Textra has no Windows-native shaping backend.

### Harmony

Use the same pinned `ubuntu24.04-harmony` SDK container as Lynx develop. Both
revisions build the same benchmark NAPI host (`run(request, metadata)`). The job
is explicitly named **build (no runtime measurement)** and emits
`build-only.json` with `performance_measured: false`. It does not claim to be a
Harmony runtime performance gate: Lynx's referenced public CI has no Harmony
device deployment/execution pool. That pool is still needed for measured data.

Local platform invocation:

```sh
python3 test/perf/run_platform.py --platform ios \
  --baseline FULL_BASE_SHA --candidate FULL_HEAD_SHA \
  --output /tmp/textra-ios-performance
```

Use `--platform android` with a booted AVD, `--platform windows` on the Windows
runner, or `--platform harmony` inside the Harmony SDK container. Local iOS and
Android execution can specify `--device` explicitly. A missing SDK, runtime,
font or device fails the job rather than producing a passing performance result.

For an already prepared platform pair:

```sh
python3 test/perf/gate.py --config /path/pair.json --output /path/new-results
```

Configuration example (all paths should be absolute):

```json
{
  "schema": 1,
  "platform": "ios",
  "backend": "ios",
  "baseline": {
    "revision": "1111111111111111111111111111111111111111",
    "command": ["/opt/textra-perf/launch-ios", "--app", "BASE_APP"],
    "artifacts": [{"path": "/path/base-app.zip", "sha256": "ACTUAL_SHA256"}]
  },
  "candidate": {
    "revision": "2222222222222222222222222222222222222222",
    "command": ["/opt/textra-perf/launch-ios", "--app", "HEAD_APP"],
    "artifacts": [{"path": "/path/head-app.zip", "sha256": "ACTUAL_SHA256"}]
  }
}
```

The default adapter protocol consumes one request on stdin and returns exactly
one JSON response on stdout. `protocol: "jsonl"` retains one process per
version/round, sending and receiving one JSON object per line, serially. Use the
executable's `--server` option for this mode. A timed-out host is killed and
restarted for the next case, while that failed case/version is not retried. Native executables take a metadata JSON file as their only argument:
`revision`, `dependencies` (fingerprint of actual shared dependencies/toolchain),
and `benchmark` (`gate.benchmark_digest(harness)` of the current source/header).
Skity metadata also includes `fonts`, the SHA-256 of the ordered font-content
checksum list. Builders must verify the deployed device font bytes against that
list; the macOS builder checks the exact local font artifacts before/after
sampling. macOS device identity is a boot-session fingerprint, unaffected by
hostname/DNS changes.
Those fields are returned from the deployed build metadata, not copied from the
request. Runtime `device`, `os`, `arch`, `backend`, `fonts`, `dependencies` and
`benchmark` identity must match across every response.

Requests include request ID, exact UTF-32 codepoints, case ID/direction/primary
policy, backend, warmups, batches, iterations and mandatory disabled cache.
Responses contain that request ID/case, actual engine revision, runtime identity,
output glyph/missing counts and signature, and shape/cleanup/total microseconds
for each batch. See `shaping_benchmark.cc` and `gate.validate_response` for the
executable protocol. Transport/launch overhead never enters batch timing.

## Validation

```sh
python3 -m unittest discover -s test/perf -p 'test_*.py'
```

Synthetic regression, persistent-host restart, noisy-round, small absolute-change, output-change,
missing-glyph, identity-drift, stale-revision, checksum and timeout scenarios
exercise the gate's decisions. Real platform results must be reported separately
from these synthetic detector tests.
