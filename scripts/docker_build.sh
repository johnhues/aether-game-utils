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
# Rosetta cannot translate node's JIT, so node aborts in an amd64 container on
# Apple Silicon.
# amd64, plus the host architecture for the workflows that run on it.
case "$( uname -m )" in
	arm64|aarch64) PLATFORMS="amd64 arm64" ;;
	*)             PLATFORMS="amd64" ;;
esac
for PLATFORM in ${PLATFORMS}; do
	docker buildx build --platform "linux/${PLATFORM}" -t "${IMAGE_NAME}:${PLATFORM}" --load .
	echo "Built ${IMAGE_NAME}:${PLATFORM}"
done
