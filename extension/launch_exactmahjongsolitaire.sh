#!/bin/sh

EXT_DIR="/mnt/us/extensions/exact-mahjong-solitaire"
APP_BIN="$EXT_DIR/bin/armhf/exact-mahjong-solitaire"
APP_LOG="/mnt/us/exact-mahjong-solitaire.log"
APP_TITLE="Exact Mahjong Solitaire"
APP_LOADER="$EXT_DIR/lib/armhf/ld-linux-armhf.so.3"
RUNTIME_MODE="${EXACT_MAHJONG_SOLITAIRE_RUNTIME:-auto}"

if [ ! -x "$APP_BIN" ]; then
    echo "$APP_TITLE binary not found: $APP_BIN" >"$APP_LOG"
    exit 1
fi

if [ "$1" = "--restart" ]; then
    pkill -f "$APP_BIN" 2>/dev/null
    pkill -f "$APP_LOADER.*$APP_BIN" 2>/dev/null
    sleep 1
fi

if pgrep -f "$APP_BIN" >/dev/null 2>&1; then
    exit 0
fi

LIB_PATHS=""
for dir in "$EXT_DIR/lib/armhf" "$EXT_DIR/bin/armhf"
do
    if [ -d "$dir" ]; then
        if [ -n "$LIB_PATHS" ]; then
            LIB_PATHS="$LIB_PATHS:$dir"
        else
            LIB_PATHS="$dir"
        fi
    fi
done

cd "$EXT_DIR" || exit 1

{
    echo "----- $APP_TITLE launch $(date) -----"
    echo "app=$APP_BIN"
    echo "runtime_mode=$RUNTIME_MODE"
    echo "lib_paths=$LIB_PATHS"
} >>"$APP_LOG"

if [ -f "$APP_LOADER" ] && [ ! -x "$APP_LOADER" ]; then
    chmod 755 "$APP_LOADER" 2>/dev/null || true
fi

try_launch() {
    mode="$1"
    shift

    echo "Trying runtime: $mode" >>"$APP_LOG"
    DISPLAY=:0 GDK_NATIVE_WINDOWS=1 "$@" >>"$APP_LOG" 2>&1 &
    pid="$!"
    sleep 1

    if kill -0 "$pid" 2>/dev/null || pgrep -f "$APP_BIN" >/dev/null 2>&1; then
        echo "Started runtime: $mode pid=$pid" >>"$APP_LOG"
        if command -v xwininfo >/dev/null 2>&1; then
            DISPLAY=:0 xwininfo -root -tree 2>/dev/null | grep -i "mahjongg\\|exactmahjongsolitaire\\|kindle" >>"$APP_LOG" 2>&1 || true
        fi
        exit 0
    fi

    wait "$pid" 2>/dev/null
    code="$?"
    echo "Runtime failed: $mode exit=$code" >>"$APP_LOG"
}

case "$RUNTIME_MODE" in
    direct)
        try_launch direct "$APP_BIN"
        ;;
    loader)
        if [ -x "$APP_LOADER" ]; then
            try_launch loader "$APP_LOADER" --library-path "$LIB_PATHS" "$APP_BIN"
        fi
        ;;
    ldpath)
        LD_LIBRARY_PATH="${LIB_PATHS}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" try_launch ldpath "$APP_BIN"
        ;;
    auto|*)
        try_launch direct "$APP_BIN"
        if [ -x "$APP_LOADER" ]; then
            try_launch loader "$APP_LOADER" --library-path "$LIB_PATHS" "$APP_BIN"
        fi
        LD_LIBRARY_PATH="${LIB_PATHS}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" try_launch ldpath "$APP_BIN"
        ;;
esac

echo "All launch methods failed." >>"$APP_LOG"
exit 1
