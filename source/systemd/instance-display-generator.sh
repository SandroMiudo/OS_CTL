#!/bin/bash

# Check if the current session is a foreground virtual console
if fgconsole &>/dev/null; then
    current_tty="tty$(fgconsole)"
    echo "Foreground virtual console detected: ${current_tty}"

    # Start only the display service for this VT
    sudo systemctl start "instance-2-display@${current_tty}.service"
else
    echo "No foreground virtual console detected, skipping service."
fi