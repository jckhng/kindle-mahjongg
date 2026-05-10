#!/bin/sh

LOG_FILE="/mnt/us/exact-mahjong-solitaire-shortcut.log"

{
    echo "Shortcut invoked $(date)"
    /bin/sh /mnt/us/extensions/exact-mahjong-solitaire/launch_exactmahjongsolitaire.sh
} >>"$LOG_FILE" 2>&1

