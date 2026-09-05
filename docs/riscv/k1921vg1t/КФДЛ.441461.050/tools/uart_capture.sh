#!/bin/bash
# Capture board console output. Defaults: /dev/ttyUSB0, 115200.
#
# Usage:
#   uart_capture.sh [device] [baud] [seconds]
#
# The CH340B node re-enumerates after reconnecting and comes up at a
# default baud — set the baud explicitly (as done below) before reading.

DEV="${1:-/dev/ttyUSB0}"
BAUD="${2:-115200}"
SECONDS_TO_CAPTURE="${3:-10}"

if [ ! -e "$DEV" ]; then
    echo "device $DEV not found" >&2
    exit 1
fi

stty -F "$DEV" "$BAUD" raw -echo

echo "capturing $DEV at $BAUD for ${SECONDS_TO_CAPTURE}s (log: /tmp/uart_capture.log)" >&2
timeout "$SECONDS_TO_CAPTURE" cat "$DEV" > /tmp/uart_capture.log

echo "captured $(wc -c < /tmp/uart_capture.log) bytes" >&2
cat -v /tmp/uart_capture.log
