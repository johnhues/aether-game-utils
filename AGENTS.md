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
| Web (Emscripten) | `build_em` (Ninja Multi-Config) | `source <path-to-emsdk>/emsdk_env.sh && cmake --build build_em --config RelWithDebInfo --target test` |
| iOS (Xcode, arm64) | `build_ios` | `cmake --build build_ios --config RelWithDebInfo --target test -- -allowProvisioningUpdates -destination generic/platform=iOS` |

Notes:
- CI (`.github/workflows/`) covers desktop (macOS, Ubuntu clang/gcc/mingw,
  Windows) and web (`emscripten.yml`) but **not iOS** — run the iOS compile
  check locally.
- The iOS build may stop at the code-signing/link step without provisioning; a
  clean *compile* of all sources is the signal that matters for header
  compatibility.
- Configure commands for `build_em` (`emcmake cmake -S . -B build_em -G "Ninja
  Multi-Config"`) and `build_ios` (Xcode generator + `scripts/ios.toolchain.cmake`)
  live in the VS Code workspace tasks (`aether-game-utils.code-workspace`).

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
