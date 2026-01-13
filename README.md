# OS_CTL

OS_CTL exposes and controls display/seat functionality on systemd-based Linux hosts — for example, starting isolated display instances (nested X or DRM-backed) and managing seat-related services.

## System Requirements

- **Operating System:** Linux with systemd. Supported distributions referenced: Ubuntu, Debian, Fedora (see `sys-install.sh`).
- **Package manager note:** The installer script uses `apt` (Debian/Ubuntu) to install packages (see `sys-install.sh`).

## Dependencies

- See `sys.req.ubuntu` for the Ubuntu package list; example packages include `build-essential`, `libdrm-dev`, `libx11-dev`, `xnest`, and `ocl-icd-opencl-dev`.
- Build tools required by the build system: `gcc`, `make`, `swig`, `pkg-config`, and Python development tools (`python3-config`). See `source/Makefile` for details.

## How to Run

- Install system packages (default target is Ubuntu/Debian):

```bash
./sys-install.sh
# or with explicit distro: DISPLAY_DRIVER_DISTRO=debian ./sys-install.sh
```

- Build everything from the repository root:

```bash
make -C source
```

- Install system-wide (requires root):

```bash
sudo make -C source install
```

- Clean build and installed artifacts:

```bash
make -C source clean-build     # remove build directory
make -C source clean-install   # remove installed files
make -C source clean-env       # remove generated env files
make -C source clean-all       # run all clean targets
```

## Environment Variables (selected)

These environment variables can be set to modify the nested display/server behavior and the build/install flow:

- `NX_WIDTH` and `NX_HEIGHT`: width and height for nested X instances (used when installing default env and in `seat_display_driver.rc`).
- `DISPLAY_DRIVER`: select backend (e.g., `XLIB` or `DRM`) when building via `make`.
- `DISPLAY_DRIVER_DISTRO`: distro identifier used by `sys-install.sh` to pick the sysreq file.
- `DISPLAY` and `XAUTHORITY`: common X-related environment variables written into the global config during install.

## Start / Terminate

- There is a convenience runner `source/start.sh`; it can be used as a simple start/installer helper.
- The build installs systemd unit files . To start and stop instances manually consider using `systemctl`:

```bash
sudo systemctl start instance-1-display.service
sudo systemctl stop instance-1-display.service
```

Use `journalctl -u <unit>` to inspect logs for a given unit.

- For cleaning up everything and stop the proper services, cnosider using the runner `source/terminate.sh`

---