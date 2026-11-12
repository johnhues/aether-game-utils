#!/bin/bash
set -xeuo pipefail
cd "$(cd "$(dirname "$0")" && pwd)/.." # Up from scripts/ to repo root

# Each stage below mirrors its .github/workflows counterpart: Release,
# AE_LEAN_AND_MEAN=OFF, target 'all', then the test binary. The build type is
# not optional -- GCC's -Wmaybe-uninitialized and friends only fire when
# optimizing, so an unset CMAKE_BUILD_TYPE hides the warnings CI treats as
# errors.
#
# On macOS the host stages run Apple clang, so they stand in for macos.yml.
# 'g++' on macOS is Apple clang, not GCC, so a real GCC build only happens
# under act below. On Linux they stand in for the Ubuntu workflows, and the
# host 'g++' is a real GCC.

# No workflow counterpart. Compile only, signing is expected to fail.
if [ "$( uname )" = "Darwin" ]; then
	cmake --preset ios-coverage
	cmake --build --preset ios-coverage-debug
else
	echo "Not on macOS, skipping the iOS compile check. Nothing else covers it."
fi

# macos.yml
CC=clang CXX=clang++ cmake -G Ninja -B build_coverage_clang -DCMAKE_BUILD_TYPE=Release -DAE_LEAN_AND_MEAN=OFF
cmake --build build_coverage_clang --target all
build_coverage_clang/test/test
test/config/check.sh native

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
CXX=x86_64-w64-mingw32-g++ test/config/check.sh native
if [ "$( uname -s )" = "Darwin" ]; then
	echo "macOS host, skipping the Windows test run under wine. act -W ubuntu_mingw.yml below covers it."
elif command -v wine > /dev/null; then
	# As ubuntu_mingw.yml, in a prefix of its own rather than ~/.wine. The
	# overrides skip the Mono and Gecko install prompts a new prefix shows.
	WINEPREFIX="$PWD/build_coverage_mingw/wineprefix" WINEDLLOVERRIDES="mscoree,mshtml=" wine build_coverage_mingw/test/test.exe
else
	echo "wine not installed, skipping the Windows test run. act -W ubuntu_mingw.yml below covers it."
fi

# web.yml, build
cmake -G Ninja -B build_coverage_wasi -DCMAKE_TOOLCHAIN_FILE="${WASI_SDK_PATH:?set WASI_SDK_PATH to the wasi-sdk install directory}/share/cmake/wasi-sdk-p1.cmake" -DCMAKE_BUILD_TYPE=Release -DAE_LEAN_AND_MEAN=OFF -DAE_LOADERS_OFBX=ON
cmake --build build_coverage_wasi --target all
test/config/check.sh wasi
node test/web/check.js build_coverage_wasi/examples/Release

# web.yml, docs-scroll. docs.py downloads its pinned doxygen into build_docs/.
# Leaves the site served on docs.py's publish port.
python3 scripts/docs.py publish --yes --no-open --config-dir build_coverage_wasi/examples/Release --out build_coverage_docs
node test/web/scroll.js build_coverage_docs

# web.yml, emscripten-guest
if command -v em++ > /dev/null; then
	test/emscripten/build.sh build_coverage_emscripten
	node test/web/check.js build_coverage_emscripten --expect-log "Rendered 60 frames"
	test/config/check.sh emscripten
else
	echo "em++ not on the PATH, skipping the Emscripten guest test. act -W web.yml below covers it."
fi

# The stages above all run the host toolchain. These run the workflows
# themselves, in the CI container, and are the only real coverage for GCC on
# macOS, and for wine on macOS or when it is not installed. Set AE_SKIP_ACT=1 to
# leave them out while iterating, or on a machine without Docker.
if [ "${AE_SKIP_ACT:-0}" != "1" ]; then
	docker info # Verify Docker is running
	# Architecture per workflow. ubuntu_mingw needs amd64: arm64 wine starts an
	# x86_64 PE in experimental ARM64EC mode and hangs. Everything else runs on
	# the host architecture: Rosetta cannot translate node's JIT, so node aborts
	# in an amd64 container on Apple Silicon.
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
	for _wf in ubuntu_clang ubuntu_gcc ubuntu_deprecated ubuntu_mingw; do
		case "$_wf" in
			ubuntu_mingw) _arch="linux/amd64" ;;
			*)            _arch="$_host_arch" ;;
		esac
		act -W ".github/workflows/$_wf.yml" --container-architecture "$_arch" --var "AE_BUILD_IMAGE=aether-build:${_arch#linux/}" --pull=false --rm
	done
	# web.yml without publish-docs, which deploys to GitHub Pages. Its examples
	# upload needs a local artifact server.
	for _job in build emscripten-guest; do
		act -W .github/workflows/web.yml -j "$_job" --artifact-server-path "$PWD/build_coverage_act_artifacts" --container-architecture "$_host_arch" --var "AE_BUILD_IMAGE=aether-build:${_host_arch#linux/}" --pull=false --rm
	done
fi

# windows.yml has no local counterpart: MSVC only runs on Windows.
