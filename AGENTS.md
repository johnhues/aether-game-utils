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
Doxyfile stays portable for machines without it. Settings live
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
| Web (Emscripten) | `build_emscripten` (Ninja Multi-Config) | `source <path-to-emsdk>/emsdk_env.sh && cmake --build build_emscripten --config RelWithDebInfo --target test` |
| iOS (Xcode, arm64) | `build_ios` | `cmake --build build_ios --config RelWithDebInfo --target test -- -allowProvisioningUpdates -destination generic/platform=iOS` |

Notes:
- CI (`.github/workflows/`) covers desktop (macOS, Ubuntu clang/gcc/mingw,
  Windows) and web (`emscripten.yml`) but **not iOS** — run the iOS compile
  check locally.
- The iOS build may stop at the code-signing/link step without provisioning; a
  clean *compile* of all sources is the signal that matters for header
  compatibility.
- Configure commands for `build_emscripten` (`emcmake cmake -S . -B build_emscripten -G "Ninja
  Multi-Config"`) and `build_ios` (Xcode generator + `scripts/ios.toolchain.cmake`)
  live in the VS Code tasks (`.vscode/tasks.json`).
- Never configure into a plain `build/`. Every toolchain gets its own `build_*`
  directory, so editor, sweep and container caches cannot collide. `.gitignore`
  covers `build` and `build_*`. The workflows use `build_ci`.

### Full toolchain sweep — `scripts/coverage_build.sh`
Pre-merge sweep that builds every toolchain in one run. Slow (~20-40 min: a fresh
`build_emscripten` and a Dockerized `act` run are the long poles), so run it via a
build agent / in the background, never inline. `set -xeuo pipefail` — it aborts at
the first failure; the `+ `-prefixed trace line just before the error names the
failing command.

Stages, in order:
1. **iOS** — `cmake --preset ios` + `cmake --build --preset ios-debug` (compile only).
2. **clang** — `build_clang`, then runs `build_clang/test/test`.
3. **clang + `AE_DEPRECATED=1`** — `build_deprecated`, then runs its `test`.
4. **gcc** — `build_gcc`, then runs its `test`.
5. **MinGW** — `build_mingw` via `.github/toolchains/mingw-w64.cmake`, **compile only**
   (the `wine .../test.exe` run is commented out). **MinGW is the stand-in for Windows
   on non-Windows hosts** — to Windows-compile-check from macOS/Linux use this
   toolchain; "no Windows machine" does not mean "no Windows coverage". Re-enable the
   `wine` line to actually run the Windows test binary.
6. **Emscripten** — `emcmake cmake -B build_emscripten`, then `node .../test.js`.
7. **act** — `act -W .github/workflows/ubuntu_clang.yml` runs the Ubuntu-clang GitHub
   Actions workflow locally in Docker (the gcc/mingw workflow runs are commented out).

Prerequisites (all currently installed on this machine): Docker running (the script
opens with `docker info`), `emcmake` (emsdk on PATH), `act`, `node`, the MinGW
toolchain (`x86_64-w64-mingw32-g++`), and `wine` (only if the mingw test run is
re-enabled). `AE_LEAN_AND_MEAN` is intentionally OFF here for iteration speed; CI
builds the test suite with it ON, so a green local sweep does not exercise that
configuration.
