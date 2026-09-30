#!/bin/bash
set -xeuo pipefail
cd "$(cd "$(dirname "$0")" && pwd)/.." # Up from scripts/ to repo root
docker info # Verify Docker is running

# Each stage below mirrors its .github/workflows counterpart: Release,
# AE_LEAN_AND_MEAN=OFF, target 'all', then the test binary. The build type is
# not optional -- GCC's -Wmaybe-uninitialized and friends only fire when
# optimizing, so an unset CMAKE_BUILD_TYPE hides the warnings CI treats as
# errors.
#
# The host stages run Apple clang, so they stand in for macos.yml. 'g++' on
# macOS is Apple clang, not GCC, so a real GCC build only happens under act
# below.

# No workflow counterpart. Compile only, signing is expected to fail.
cmake --preset ios-coverage
cmake --build --preset ios-coverage-debug

# macos.yml
CC=clang CXX=clang++ cmake -G Ninja -B build_coverage_clang -DCMAKE_BUILD_TYPE=Release -DAE_LEAN_AND_MEAN=OFF
cmake --build build_coverage_clang --target all
build_coverage_clang/test/test

# ubuntu_deprecated.yml, on the host toolchain
CC=clang CXX=clang++ cmake -G Ninja -B build_coverage_deprecated -DCMAKE_BUILD_TYPE=Release -DAE_LEAN_AND_MEAN=OFF -DAE_DEPRECATED=ON
cmake --build build_coverage_deprecated --target all
build_coverage_deprecated/test/test

# ubuntu_gcc.yml, but only when a real GCC is installed. Homebrew names it
# g++-NN; /usr/bin/g++ is Apple clang and would silently repeat the clang stage.
AE_GXX=""
for _gxx in g++-15 g++-14 g++-13 g++; do
	if command -v "$_gxx" > /dev/null && "$_gxx" --version | head -1 | grep -q "^g++ (\|(GCC)"; then
		AE_GXX="$_gxx"
		break
	fi
done
if [ -n "$AE_GXX" ]; then
	CC="${AE_GXX/g++/gcc}" CXX="$AE_GXX" cmake -G Ninja -B build_coverage_gcc -DCMAKE_BUILD_TYPE=Release -DAE_LEAN_AND_MEAN=OFF
	cmake --build build_coverage_gcc --target all
	build_coverage_gcc/test/test
else
	echo "No real GCC found, skipping the host gcc stage. act -W ubuntu_gcc.yml below covers it."
fi

# ubuntu_mingw.yml
cmake -G Ninja -B build_coverage_mingw -DCMAKE_TOOLCHAIN_FILE=.github/toolchains/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Release -DAE_LEAN_AND_MEAN=OFF
cmake --build build_coverage_mingw --target all
if command -v wine > /dev/null; then
	wine build_coverage_mingw/test/test.exe
else
	echo "wine not installed, skipping the Windows test run. act -W ubuntu_mingw.yml below covers it."
fi

# emscripten.yml
emcmake cmake -G Ninja -B build_coverage_emscripten -DCMAKE_BUILD_TYPE=Release -DAE_LEAN_AND_MEAN=OFF
cmake --build build_coverage_emscripten --target all
node build_coverage_emscripten/test/test.js

# The stages above all run the host toolchain. These run the workflows
# themselves, in the CI container, and are the only real coverage for GCC and
# for wine. Set AE_SKIP_ACT=1 to leave them out while iterating.
if [ "${AE_SKIP_ACT:-0}" != "1" ]; then
	# Architecture per workflow. ubuntu_mingw needs amd64: arm64 wine starts an
	# x86_64 PE in experimental ARM64EC mode and hangs. Everything else runs on
	# the host architecture: Rosetta cannot translate node's JIT, so an amd64
	# container aborts the emscripten workflow on Apple Silicon.
	# AE_BUILD_IMAGE points the workflows at the images docker2/docker_build.sh
	# builds. The names exist in no registry, so nothing can replace them with a
	# published copy. --pull=false is required with it: act pulls by default,
	# and only a non forced pull tolerates the failure and falls back to the
	# local image. One single architecture image per tag, because act only looks
	# for a local image on the host architecture and tries to pull for any
	# other.
	case "$( uname -m )" in
		arm64|aarch64) _host_arch="linux/arm64" ;;
		*)             _host_arch="linux/amd64" ;;
	esac
	for _tag in amd64 arm64; do
		if ! docker image inspect "aether-build:$_tag" > /dev/null 2>&1; then
			echo "Missing local image aether-build:$_tag."
			echo "Build both with scripts/docker_build.sh"
			exit 1
		fi
	done
	for _wf in ubuntu_clang ubuntu_gcc ubuntu_deprecated ubuntu_mingw emscripten; do
		case "$_wf" in
			ubuntu_mingw) _arch="linux/amd64" ;;
			*)            _arch="$_host_arch" ;;
		esac
		act -W ".github/workflows/$_wf.yml" --container-architecture "$_arch" --var "AE_BUILD_IMAGE=aether-build:${_arch#linux/}" --pull=false --rm
	done
fi

# windows.yml has no local counterpart: MSVC only runs on Windows.
