# Agent Instructions

## Public Naming
User-facing names should not start with a leading underscore. Internal helpers
and implementation details may use leading-underscore naming.

This split is intentional:
- The Doxygen config excludes `ae::_*` symbols from generated docs.
- Public customization seams should use clean `AE_...` macro names.
- Internal `_AE_...` macros and `ae::_...` symbols are implementation details
  and may hide xmacro machinery from generated docs.

When adding a new user-defined customization point, prefer a clean public
`AE_...` macro even if it feeds internal `_AE_...` utilities.

Do not expose xmacro details directly to users when a clean public macro seam is
possible.

## Documentation
`scripts/docs.py` drives doxygen. Run it with no arguments for a menu, or pass a
task: `preview` (HTML site), `markdown` (regenerates the checked-in `docs/*.md`,
needs moxygen), `report` (ranks what is still undocumented). Output lands in
`build_docs/` (gitignored); only `markdown` writes into the repo. `preview`
switches graphs to graphviz automatically when `dot` is installed, so the
Doxyfile stays portable for machines without it. `preview` and `publish` also
list every example from `examples/CMakeLists.txt` on the main page, in numbered
order, at the `<!-- examples -->` line in README.md; an example's description is
the first comment block after its file's banner and license, ending in a period. Settings live
in `Doxyfile`, tab names in `DoxygenLayout.xml`, presentation in
`doxygen-custom.css`; `docs.py` layers per-task overrides on a copy, so none of
them need editing to preview something.

Every external macro carries a description and a usage block:

```cpp
//! Registers the class variable 'MyType::classVar'
//! Usage:
//! \code
//! AE_REGISTER_CLASS_VAR( MyType, classVar );
//! \endcode
#define AE_REGISTER_CLASS_VAR( _CLASS, _V ) ...
```

Five doxygen behaviors shape where those comments go:
- `aether.h` has no `\file` block, so a macro is only documented if it sits
  inside a `\defgroup` block or carries `\ingroup`. Outside one it produces no
  output at all.
- Macros defined inside `#if`/`#ifndef` need a `//! \def NAME` block placed
  *before* the `#if`. Doxygen preprocesses, so a comment in the branch it does
  not take is dropped, and a comment directly above `#ifndef` is absorbed into
  the enclosing group's description instead.
- `\defgroup Name Title` needs both a title and an explicit `\brief`, or the
  group's row in the category index is blank. Follow `@{` with a `//---` rule so
  the next comment block is not absorbed into the group description.
- A macro that appears inside a declaration, like `AE_ALIGN( 16 ) struct Vec3`,
  hides the declaration from doxygen unless it is listed in `PREDEFINED`.
- `QT_AUTOBRIEF` is on, so the first sentence of a `//!` block becomes the brief
  shown in summary tables. Keep that first sentence self-contained.

## Compatibility Builds
`aether.h` ships to desktop, web, and iOS, so changes to it must compile-check
on all three toolchains before landing; `scripts/coverage_build.sh` runs the full
toolchain sweep (documented at the end of this section). The `test` target (Catch2
suite) is the fastest cross-platform compile target, exercising most of the library. All three
build directories are pre-configured; reconfigure only when a `CMakeCache.txt`
is missing.

| Platform | Build dir | Compile command |
|----------|-----------|-----------------|
| Desktop (macOS) | `build_vscode` (Ninja Multi-Config) | `cmake --build build_vscode --config RelWithDebInfo --target test` |
| Web (wasi-sdk) | `build_wasi` (Ninja Multi-Config) | `cmake --build build_wasi --config RelWithDebInfo` (no `test` target: wasi-sdk has no exceptions) |
| iOS (Xcode, arm64) | `build_ios` | `cmake --build build_ios --config RelWithDebInfo --target test -- -allowProvisioningUpdates` |

Notes:
- CI (`.github/workflows/`) covers desktop (macOS, Ubuntu clang/gcc/mingw,
  Windows) and web (`web.yml`) but **not iOS** — run the iOS compile
  check locally.
- `test/config/check.sh native|wasi|emscripten` builds every supported
  combination of the `AE_ENABLE_*` feature flags and checks each unsupported one
  fails with only its `#error`. CI runs it in every workflow but `windows.yml`.
- `node test/web/check.js <dir>` loads every example of a web build (eg.
  `build_wasi/examples/Release`) in headless Chrome and fails on a halt, an
  exception, a console or network error or warning, an aether `[warn]` log, or
  no frames. Needs Node 22+ and Chrome (`$CHROME`, Playwright's Chromium in
  `$PLAYWRIGHT_BROWSERS_PATH`, the macOS app, Chrome/Chromium on the PATH, or
  Flathub's `org.chromium.Chromium`). `web.yml`
  runs it, the CI image provides Node and Playwright's Chromium.
- `node test/web/scroll.js <site>` flings the main page of a
  `scripts/docs.py publish` site in headless Chrome, with WebKit's suspended
  animation frames in out of view iframes. It fails when the live examples it
  passes load, a frame is removed before its program starts, the last example
  is slow to get ready, or the page hangs or logs an error. It finds Chrome as
  `check.js` does. `web.yml` runs it in
  `docs-scroll`.
- `web.yml` also builds `test/emscripten` (`test/emscripten/build.sh`, needs
  `em++` on the PATH), which checks `aether.h` still compiles cleanly as a guest
  of an Emscripten program. Emscripten is not otherwise a dependency.
- The iOS build may stop at the code-signing/link step without provisioning; a
  clean *compile* of all sources is the signal that matters for header
  compatibility.
- Configure commands for `build_wasi` (`cmake --preset wasi`, with `WASI_SDK_PATH`
  set to the wasi-sdk install directory) and `build_ios` (Xcode generator +
  `scripts/ios.toolchain.cmake`) live in the VS Code tasks (`.vscode/tasks.json`).
- Never configure into a plain `build/`. Every toolchain gets its own `build_*`
  directory, so editor, sweep and container caches cannot collide. `.gitignore`
  covers `build` and `build_*`. The workflows use `build_ci`.

### Full toolchain sweep — `scripts/coverage_build.sh`
Pre-merge sweep that builds every toolchain in one run. Slow (~20-40 min: a fresh
`build_coverage_wasi` and a Dockerized `act` run are the long poles), so run it via a
build agent / in the background, never inline. `set -xeuo pipefail` — it aborts at
the first failure; the `+ `-prefixed trace line just before the error names the
failing command.

Stages, in order:
1. **iOS** — `cmake --preset ios` + `cmake --build --preset ios-debug` (compile only,
   skipped on non-macOS hosts).
2. **clang** — `build_clang`, then runs `build_clang/test/test`.
3. **clang + `AE_DEPRECATED=1`** — `build_deprecated`, then runs its `test`.
4. **gcc** — `build_gcc`, then runs its `test`.
5. **MinGW** — `build_mingw` via `.github/toolchains/mingw-w64.cmake`, then runs
   `test.exe` under wine when it is installed, in a prefix inside the build directory as
   `ubuntu_mingw.yml` does. **MinGW is the stand-in for Windows on non-Windows hosts** —
   to Windows-check from macOS/Linux use this toolchain; "no Windows machine" does not
   mean "no Windows coverage".
6. **Web** — `build_coverage_wasi` with the wasi-sdk toolchain, its configuration
   checks, and `test/web/check.js` on every example, then a `docs.py publish` site
   of those examples in `build_coverage_docs` and `test/web/scroll.js` on it, then
   `test/emscripten/build.sh` and its page when `em++` is on the PATH.
7. **act** — `act -W .github/workflows/ubuntu_clang.yml` runs the Ubuntu-clang GitHub
   Actions workflow locally in Docker (the gcc/mingw workflow runs are commented out).

Prerequisites (all currently installed on this machine): Docker running (checked
with `docker info` before the act stage; set `AE_SKIP_ACT=1` without it),
`WASI_SDK_PATH`, `em++` (optional, emsdk on PATH), `act`, `node`, the MinGW
toolchain (`x86_64-w64-mingw32-g++`), Chrome or Chromium (see `test/web/check.js`), and
`wine` (runs the MinGW test binary, skipped without it). `AE_LEAN_AND_MEAN` is intentionally OFF here for iteration speed; CI
builds the test suite with it ON, so a green local sweep does not exercise that
configuration.
