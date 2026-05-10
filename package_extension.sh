#!/bin/bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PKG_ROOT="$ROOT/dist"
RELEASE_ROOT="$ROOT/release"
EXT_ROOT="$PKG_ROOT/extensions/exact-mahjong-solitaire"
DOC_ROOT="$PKG_ROOT/documents"
CONTAINER="${EXACT_MAHJONG_SOLITAIRE_DOCKER_CONTAINER:-exact-mahjong-solitaire-armhf-builder}"

rm -rf "$PKG_ROOT"
mkdir -p "$RELEASE_ROOT"
mkdir -p "$EXT_ROOT/bin/armhf" \
         "$EXT_ROOT/lib/armhf" \
         "$EXT_ROOT/assets" \
         "$EXT_ROOT/LICENSES" \
         "$DOC_ROOT"

cp "$ROOT/exact-mahjong-solitaire" "$EXT_ROOT/bin/armhf/exact-mahjong-solitaire"
cp "$ROOT/extension/config.xml" "$EXT_ROOT/config.xml"
cp "$ROOT/extension/menu.json" "$EXT_ROOT/menu.json"
cp "$ROOT/extension/launch_exactmahjongsolitaire.sh" "$EXT_ROOT/launch_exactmahjongsolitaire.sh"
cp "$ROOT/extension/stop_exactmahjongsolitaire.sh" "$EXT_ROOT/stop_exactmahjongsolitaire.sh"
cp "$ROOT/extension/tail_log_exactmahjongsolitaire.sh" "$EXT_ROOT/tail_log_exactmahjongsolitaire.sh"
cp "$ROOT/extension/shortcut_exactmahjongsolitaire.sh" "$DOC_ROOT/shortcut_exactmahjongsolitaire.sh"
cp "$ROOT/extension/NOTICE.txt" "$EXT_ROOT/NOTICE.txt"
cp "$ROOT/extension/README.md" "$EXT_ROOT/README-package.txt"

cp "$ROOT/assets/"* "$EXT_ROOT/assets/"
cp "$ROOT/licenses/COPYING" "$EXT_ROOT/LICENSES/COPYING"
cp "$ROOT/licenses/COPYING-DOCS" "$EXT_ROOT/LICENSES/COPYING-DOCS"
cp "$ROOT/licenses/COPYING.GPL3" "$EXT_ROOT/LICENSES/COPYING.GPL3"

if docker exec "$CONTAINER" /bin/bash -lc 'test -f /src/exact-mahjong-solitaire/exact-mahjong-solitaire' >/dev/null 2>&1; then
    docker exec "$CONTAINER" /bin/bash -lc '
        {
            echo /lib/arm-linux-gnueabihf/ld-linux-armhf.so.3
            ldd /src/exact-mahjong-solitaire/exact-mahjong-solitaire | grep -oE "/[^[:space:]]+"
        } | sort -u
    ' > "$EXT_ROOT/LICENSES/RUNTIME-LIBS.txt"

    while IFS= read -r libpath; do
        [ -n "$libpath" ] || continue
        docker exec "$CONTAINER" /bin/bash -lc "cat '$libpath'" > "$EXT_ROOT/lib/armhf/$(basename "$libpath")"
    done < "$EXT_ROOT/LICENSES/RUNTIME-LIBS.txt"

    cat > "$EXT_ROOT/LICENSES/THIRD-PARTY-NOTICE.txt" <<EOF
This package bundles ARM shared libraries copied from the Docker build
container ($CONTAINER) to reduce Kindle-side external dependencies.

The exact bundled files are listed in RUNTIME-LIBS.txt. These libraries remain
under their own upstream licenses.
EOF
else
    echo "No runtime library set was generated." > "$EXT_ROOT/LICENSES/RUNTIME-LIBS.txt"
    echo "No runtime libraries were bundled at packaging time." > "$EXT_ROOT/LICENSES/THIRD-PARTY-NOTICE.txt"
fi

chmod 755 "$EXT_ROOT/launch_exactmahjongsolitaire.sh" \
          "$EXT_ROOT/stop_exactmahjongsolitaire.sh" \
          "$EXT_ROOT/tail_log_exactmahjongsolitaire.sh" \
          "$DOC_ROOT/shortcut_exactmahjongsolitaire.sh" \
          "$EXT_ROOT/bin/armhf/exact-mahjong-solitaire"

if [ -f "$EXT_ROOT/lib/armhf/ld-linux-armhf.so.3" ]; then
    chmod 755 "$EXT_ROOT/lib/armhf/ld-linux-armhf.so.3"
fi

(
    cd "$PKG_ROOT"
    if command -v zip >/dev/null 2>&1; then
        zip -qr exact-mahjong-solitaire-extension.zip extensions documents
    else
        python3 - <<'PY'
import os
import zipfile

with zipfile.ZipFile("exact-mahjong-solitaire-extension.zip", "w", zipfile.ZIP_DEFLATED) as zf:
    for top in ("extensions", "documents"):
        for root, _, files in os.walk(top):
            for name in files:
                path = os.path.join(root, name)
                zf.write(path, path)
PY
    fi
)

mkdir -p "$RELEASE_ROOT"
cp "$PKG_ROOT/exact-mahjong-solitaire-extension.zip" "$RELEASE_ROOT/exact-mahjong-solitaire-extension.zip"
(
    cd "$RELEASE_ROOT"
    sha256sum "exact-mahjong-solitaire-extension.zip" > SHA256SUMS
)

echo "Package created:"
echo "  $PKG_ROOT/exact-mahjong-solitaire-extension.zip"
echo "  $RELEASE_ROOT/exact-mahjong-solitaire-extension.zip"
echo "  $RELEASE_ROOT/SHA256SUMS"

