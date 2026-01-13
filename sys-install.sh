#!/bin/bash

set -euo pipefail

# ------------------------
# Supported distributions
# ------------------------
SUPPORTED_DISTROS=("ubuntu" "debian" "fedora")  # extend as needed

# Environment variable
DISPLAY_DRIVER_DISTRO="${DISPLAY_DRIVER_DISTRO:-ubuntu}" # default to ubuntu

# ------------------------
# Verify the distro
# ------------------------
if [[ ! " ${SUPPORTED_DISTROS[*]} " =~ " ${DISPLAY_DRIVER_DISTRO} " ]]; then
    echo "ERROR: Unsupported distro '${DISPLAY_DRIVER_DISTRO}'."
    echo "Supported distributions are: ${SUPPORTED_DISTROS[*]}"
    exit 1
fi

# ------------------------
# Map distro to requirements file
# ------------------------
REQ_FILE="sys.req.${DISPLAY_DRIVER_DISTRO}"

# Fallback check (optional, extra safety)
if [ ! -f "$REQ_FILE" ]; then
    echo "ERROR: Requirements file '$REQ_FILE' not found."
    exit 1
fi

echo "Installing system packages from $REQ_FILE..."

# Install packages listed in the requirements file
# Skips empty lines and comments
grep -vE '^\s*#|^\s*$' "$REQ_FILE" | xargs sudo apt install -y

echo "System dependencies installed successfully."