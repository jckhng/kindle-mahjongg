#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
IMAGE="${KINDLE_MAHJONGG_DOCKER_IMAGE:-kindle-mahjongg-armhf-build:bullseye}"

docker build \
    --platform linux/arm/v7 \
    -t "$IMAGE" \
    -f "$ROOT/docker/Dockerfile.armhf-bullseye" \
    "$ROOT/docker"

echo "Built Docker image: $IMAGE"

