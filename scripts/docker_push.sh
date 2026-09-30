#!/bin/bash
set -e
cd "$( cd "$( dirname "$0" )" && pwd )" # The Dockerfile and its context live here

IMAGE_NAME="ghcr.io/johnhues/aether-build"
TAG="${1:-latest}"

# Publishes the image the workflows use by default. Authentication is
# environmental: run `docker login ghcr.io` first.
docker buildx build --platform linux/amd64,linux/arm64 -t "${IMAGE_NAME}:${TAG}" --push .
echo "Pushed ${IMAGE_NAME}:${TAG}"
