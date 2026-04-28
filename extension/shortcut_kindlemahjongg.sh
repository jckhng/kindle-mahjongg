#!/bin/sh

LOG_FILE="/mnt/us/kindle-mahjongg-shortcut.log"

{
    echo "Shortcut invoked $(date)"
    /bin/sh /mnt/us/extensions/kindle-mahjongg/launch_kindlemahjongg.sh
} >>"$LOG_FILE" 2>&1

