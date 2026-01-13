#!/bin/bash

# USER must set NX_WIDTH and NX_HEIGHT before running
if [[ -z "$NX_WIDTH" || -z "$NX_HEIGHT" ]]; then
    echo "Please set NX_WIDTH and NX_HEIGHT environment variables"
    exit 1
fi

# 1. Determine next free X display number
next_display=1
while [[ -e "/tmp/.X11-unix/X${next_display}" ]]; do
    next_display=$((next_display+1))
done

TGT_IPC_FILE="/run/seat_display_driver/display_id"

NESTED_DISPLAY="$next_display"
echo "Using nested display :${NESTED_DISPLAY}"

echo "${NESTED_DISPLAY}" > "${TGT_IPC_FILE}"

echo "Starting Xnest... target server => ${DISPLAY}"
Xnest ":${NESTED_DISPLAY}" \
    -geometry "${NX_WIDTH}x${NX_HEIGHT}+0+0" \
    -name "Seat Display" \
    -background none \
    -bw 10 &

GLOBAL_CONFIG_DIR="/etc/seat-display-driver"
GLOBAL_CONFIG="display_env"
XAUTH_CONFIG="Xauthority"

> "${GLOBAL_CONFIG_DIR}/${XAUTH_CONFIG}"

# wait for background job
while [ ! -e "/tmp/.X11-unix/X${NESTED_DISPLAY}" ]; do
    sleep 0.1
done

xauth -f "${GLOBAL_CONFIG_DIR}/${XAUTH_CONFIG}" generate ":${NESTED_DISPLAY}" . trusted