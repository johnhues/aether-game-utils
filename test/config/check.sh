#!/usr/bin/env bash
# Builds aether.h under each supported combination of its feature flags for one
# toolchain, and checks each unsupported combination fails with only its
# #error. Warnings are errors. WebAssembly builds also link, so every stubbed
# function must be defined.
#
# Usage: test/config/check.sh native|wasi|emscripten
#   native      $CXX, default clang++
#   wasi        $WASI_SDK_PATH/bin/clang++
#   emscripten  em++ on the PATH (see emsdk_env.sh)
set -uo pipefail
here="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
root="$( cd "$here/../.." && pwd )"
out="$( mktemp -d )"
trap 'rm -rf "$out"' EXIT
toolchain="${1:?usage: check.sh native|wasi|emscripten}"

# Default warnings, plus the ones aether's CMake targets treat as errors
warnings="-Werror -Wreturn-type -Wimplicit-fallthrough -Wnarrowing -Wuninitialized -Wreorder -Wmismatched-tags"
case "$toolchain" in
	native)
		cxx="${CXX:-clang++}"
		flags="-std=c++20 -fno-exceptions -I$root"
		impl="$root/aether.cpp"
		machine="$( $cxx -dumpmachine )"
		case "$machine" in
			*apple*)
				flags="$flags -x objective-c++"
				impl="$root/aether.mm"
				;;
			*mingw*|*windows*)
				flags="$flags -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS" # As set by CMakeLists.txt
				;;
		esac
		link=0
		;;
	wasi)
		cxx="${WASI_SDK_PATH:?set WASI_SDK_PATH to the wasi-sdk install directory}/bin/clang++"
		flags="--target=wasm32-wasip1 -std=c++20 -fno-exceptions -I$root"
		impl="$root/aether.cpp"
		link=1
		;;
	emscripten)
		cxx="em++"
		flags="-std=c++20 -fno-exceptions -I$root"
		impl="$root/aether.cpp"
		link=1
		;;
	*)
		echo "Unknown toolchain '$toolchain'" >&2
		exit 2
		;;
esac

failures=0
log="$out/log"

# Builds Main.cpp with aether under the given defines and expects no warnings
pass()
{
	local name="$1"; shift
	if [ "$link" = "1" ]; then
		local extra=""
		[ "$toolchain" = "emscripten" ] && extra="-lopenal"
		$cxx $flags $warnings "$@" "$here/Main.cpp" "$impl" $extra -o "$out/main.wasm" > "$log" 2>&1
	else
		$cxx $flags $warnings "$@" -fsyntax-only "$here/Main.cpp" > "$log" 2>&1 &&
			$cxx $flags $warnings "$@" -fsyntax-only "$impl" >> "$log" 2>&1
	fi
	local status=$?
	if [ $status -ne 0 ] || grep -q "warning:\|error:" "$log"; then
		echo "FAIL  $name: expected a clean build"
		sed 's/^/      /' "$log" | head -20
		failures=$(( failures + 1 ))
	else
		echo "pass  $name"
	fi
}

# Compiles aether under the given defines and expects exactly one error, the
# #error containing the expected message
fail()
{
	local name="$1"; local message="$2"; shift 2
	$cxx $flags "$@" -fsyntax-only "$impl" > "$log" 2>&1
	local status=$?
	local errors="$( grep -c "error:" "$log" )"
	local warnings_seen="$( grep -c "warning:" "$log" )"
	if [ $status -eq 0 ] || [ "$errors" != "1" ] || [ "$warnings_seen" != "0" ] || ! grep -q "error:.*\"$message\"" "$log"; then
		echo "FAIL  $name: expected only #error \"$message\""
		sed 's/^/      /' "$log" | head -20
		failures=$(( failures + 1 ))
	else
		echo "pass  $name (rejected)"
	fi
}

echo "aether configurations, $toolchain"
case "$toolchain" in
	native)
		pass "defaults"
		pass "no application" -DAE_ENABLE_APPLICATION=0
		pass "no opengl" -DAE_ENABLE_OPENGL=0
		pass "no openal" -DAE_ENABLE_OPENAL=0
		pass "headless" -DAE_ENABLE_APPLICATION=0 -DAE_ENABLE_OPENGL=0 -DAE_ENABLE_OPENAL=0
		fail "aether.js outside wasi-sdk" "AE_ENABLE_AETHER_JS requires a wasi-sdk build" -DAE_ENABLE_AETHER_JS=1
		;;
	wasi)
		pass "aether.js (defaults)"
		pass "aether.js, no opengl" -DAE_ENABLE_OPENGL=0
		pass "aether.js, no openal" -DAE_ENABLE_OPENAL=0
		pass "other host" -DAE_ENABLE_AETHER_JS=0
		pass "other host, no opengl" -DAE_ENABLE_AETHER_JS=0 -DAE_ENABLE_OPENGL=0
		fail "aether.js without application" "AE_ENABLE_AETHER_JS requires AE_ENABLE_APPLICATION" -DAE_ENABLE_APPLICATION=0
		fail "other host with application" "AE_ENABLE_APPLICATION has no backend in this WebAssembly build, see AE_ENABLE_AETHER_JS" -DAE_ENABLE_AETHER_JS=0 -DAE_ENABLE_APPLICATION=1
		fail "other host with openal" "AE_ENABLE_OPENAL has no binding in this WebAssembly build, see AE_ENABLE_AETHER_JS" -DAE_ENABLE_AETHER_JS=0 -DAE_ENABLE_OPENAL=1
		;;
	emscripten)
		pass "guest (defaults)"
		pass "guest, no opengl" -DAE_ENABLE_OPENGL=0
		pass "guest, no openal" -DAE_ENABLE_OPENAL=0
		fail "guest with aether.js" "AE_ENABLE_AETHER_JS requires a wasi-sdk build" -DAE_ENABLE_AETHER_JS=1
		fail "guest with application" "AE_ENABLE_APPLICATION has no backend in this WebAssembly build, see AE_ENABLE_AETHER_JS" -DAE_ENABLE_APPLICATION=1
		;;
esac

if [ $failures -ne 0 ]; then
	echo "$failures configuration check(s) failed"
	exit 1
fi
