#!/bin/bash

readonly DEBUG_ON="on"
readonly VISUAL_CLASS="TrueColor"
readonly VISUAL_DEPTH=24
readonly WINDOW_NAME="Seat Display"
readonly WINDOW_BG_WHITE="-wr"
readonly WINDOW_BG_BLACK="-br"
readonly HARD_RESET="-full"
readonly SOFT_RESET="-reset"

readonly GLOBAL_CONFIG_DIR="/etc/seat-display-driver"
readonly GLOBAL_CONFIG="display_env"
readonly XAUTH_CONFIG="Xauthority"
readonly GROUP="seat_display_driver" 

umask 027

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

NESTED_DISPLAY_OPTIONS=(
  -auth "${GLOBAL_CONFIG_DIR}/${XAUTH_CONFIG}"
  -maxclients "${MAX_CLIENTS}"
  -geometry "${NX_WIDTH}x${NX_HEIGHT}+${NX_OFF_X}+${NX_OFF_Y}"
  -name "${WINDOW_NAME}"
  -bw "${BORDER_WIDTH}"
  "${WINDOW_BG_WHITE}"
  -extension SECURITY
  +extension X-RESOURCE
  -full
  -class "${VISUAL_CLASS}"
)

#for protocol in "${PROTOCOL_DISABLED[@]}"; do
#    NESTED_DISPLAY_OPTIONS+=(-nolisten "${protocol}")
#done

if [ "${DEBUG}" = "${DEBUG_ON}" ]; then
    NESTED_DISPLAY_OPTIONS+=(
        -sync
    )
fi

TGT_IPC_FILE="/run/seat_display_driver/display_id"

NESTED_DISPLAY="$next_display"
echo "Using nested display :${NESTED_DISPLAY}"

echo "${NESTED_DISPLAY}" > "${TGT_IPC_FILE}"

: > "${GLOBAL_CONFIG_DIR}/${XAUTH_CONFIG}"

# xauth writes the file and changes perms
xauth -f "${GLOBAL_CONFIG_DIR}/${XAUTH_CONFIG}" add ":${NESTED_DISPLAY}" . $(mcookie --verbose)

chgrp "${GROUP}" "${GLOBAL_CONFIG_DIR}/${XAUTH_CONFIG}"
chmod 0640 "${GLOBAL_CONFIG_DIR}/${XAUTH_CONFIG}"

echo "Starting Xnest... target server => ${DISPLAY}"
echo "Options used for server : ${NESTED_DISPLAY_OPTIONS[*]}"

Xnest ":${NESTED_DISPLAY}" "${NESTED_DISPLAY_OPTIONS[@]}" &

set +x

# wait for background job
while [ ! -e "/tmp/.X11-unix/X${NESTED_DISPLAY}" ]; do
    sleep 0.1
done