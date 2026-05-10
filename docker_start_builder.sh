#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
IMAGE="${EXACT_MAHJONG_SOLITAIRE_DOCKER_IMAGE:-exact-mahjong-solitaire-armhf-build:bullseye}"
CONTAINER="${EXACT_MAHJONG_SOLITAIRE_DOCKER_CONTAINER:-exact-mahjong-solitaire-armhf-builder}"

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    "$ROOT/docker_build_image.sh"
fi

if docker container inspect "$CONTAINER" >/dev/null 2>&1; then
    if [ "$(docker inspect -f '{{.State.Running}}' "$CONTAINER")" != "true" ]; then
        docker start "$CONTAINER" >/dev/null
    fi
else
    docker run -d \
        --platform linux/arm/v7 \
        --name "$CONTAINER" \
        -v "$ROOT:/src/exact-mahjong-solitaire" \
        -w /src/exact-mahjong-solitaire \
        "$IMAGE" \
        sleep infinity >/dev/null
fi

echo "$CONTAINER"

