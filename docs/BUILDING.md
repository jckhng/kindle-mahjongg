# Building Exact Mahjong Solitaire

## Requirements

- Docker.
- ARM binfmt support if your Docker setup does not already run ARM containers.

Install ARM binfmt support on Linux with:

```bash
docker run --privileged --rm tonistiigi/binfmt --install arm
```

## Build And Package

```bash
./docker_rebuild.sh
```

The persistent builder is:

```text
image:     exact-mahjong-solitaire-armhf-build:bullseye
container: exact-mahjong-solitaire-armhf-builder
```

Build outputs:

```text
exact-mahjong-solitaire
smoke-test
dist/exact-mahjong-solitaire-extension.zip
```

## Build Without Packaging

```bash
EXACT_MAHJONG_SOLITAIRE_PACKAGE=0 ./docker_rebuild.sh
```

## Builder Shell

```bash
./docker_shell.sh
```

Inside the container:

```bash
make clean
make exact-mahjong-solitaire smoke-test
./smoke-test
```

If you move the repository, recreate the persistent container:

```bash
docker rm -f exact-mahjong-solitaire-armhf-builder
./docker_rebuild.sh
```

