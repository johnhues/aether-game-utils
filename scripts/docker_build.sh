#!/bin/bash
set -e
cd "$( cd "$( dirname "$0" )" && pwd )" # The Dockerfile and its context live here

IMAGE_NAME="aether-build"

# Local only, one image per architecture. The names exist in no registry, so
# nothing can replace them with a published copy, and scripts/coverage_build.sh
# selects one per workflow with --var AE_BUILD_IMAGE. Separate tags rather than a
# multi platform index because act only looks for a local image on the host
# architecture, and tries to pull for any other. docker_push.sh publishes under
# the ghcr name separately.
#
# ubuntu_mingw needs amd64: arm64 wine hangs starting an x86_64 PE in
# experimental ARM64EC mode. Everything else runs on the host architecture:
# Rosetta cannot translate node's JIT, so an amd64 container aborts the
# emscripten workflow on Apple Silicon.
for PLATFORM in amd64 arm64; do
	docker buildx build --platform "linux/${PLATFORM}" -t "${IMAGE_NAME}:${PLATFORM}" --load .
	echo "Built ${IMAGE_NAME}:${PLATFORM}"
done
