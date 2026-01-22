# ========================
# Compiler settings
# ========================
CC = gcc
SWIG = swig
CFLAGS = -Wall -Wextra -Wshadow -Wconversion -O2 \
	     -fPIC \
		 -Ilib \
         -Idisplay \
		 -I.

LDFLAGS = -shared -Wl,-soname,$(DISPLAY_DRIVER_SONAME) -pthread

VERSION = 1.0.0

DISPLAY_DRIVER_DRM = DRM
DISPLAY_DRIVER_XLIB = XLIB

DISPLAY_DRIVER ?= XLIB

NX_HEIGHT ?= 1080
NX_WIDTH ?= 1920

PYTHON_CFLAGS = $(shell python3-config --includes) -Iinclude/python
PYTHON_LDFLAGS = $(shell python3-config --ldflags --embed)

ifeq ($(DISPLAY_DRIVER), $(DISPLAY_DRIVER_DRM))
CFLAGS += -DDISPLAY_DRIVER_DRM $(shell pkg-config --cflags-only-I libdrm) -Idisplay/drm_driver
LDFLAGS += $(shell pkg-config --libs libdrm) 
DISPLAY_DRIVER_CORE = display/drm_driver/drm_driver_core.c
endif

ifeq ($(DISPLAY_DRIVER), $(DISPLAY_DRIVER_XLIB))
CFLAGS += -DDISPLAY_DRIVER_XLIB $(shell pkg-config --cflags-only-I x11) -Idisplay/xlib_driver 
LDFLAGS += $(shell pkg-config --libs x11)
DISPLAY_DRIVER_CORE = display/xlib_driver/xlib_driver_core.c
endif

SWIG_INTERFACE := display_api.i
SWIG_PYTHON := $(SWIG) -python
SWIG_RUST   := $(SWIG) -rust
SWIG_OPTIONS := -Wall # -debug-tmused 

GLOBAL_CONFIG_DIR := /etc/seat-display-driver
GLOBAL_CONFIG := display_env

# ========================
# Build directory
# ========================
BUILD_DIR := $(abspath ../build)

# ========================
# Proxy binary
# ========================
PROXY_SRC := proxy/display_proxy.c
PROXY_BUILD_DIR := $(BUILD_DIR)/proxy
PROXY_BIN := $(PROXY_BUILD_DIR)/proxy

# ========================
# Seat target binaries (each is a separate binary; both depend on seat_utility.c)
# ========================
GLOBAL_UTIL_SRC := utils.c

SEAT_UTIL_SRC := seat-config/seat_utility.c 

SEAT_BUILD_DIR := $(PROXY_BUILD_DIR)/target

SEAT_SRC := seat-config/seat1_calle.c \
           seat-config/seat2_calle.c

SEAT_BINS := $(SEAT_SRC:seat-config/%.c=$(SEAT_BUILD_DIR)/%)

$(info SEAT_BINS = $(SEAT_BINS))
$(info SEAT_BUILD_DIR = $(SEAT_BUILD_DIR))

# ========================
# Start nested X script (installed to final location)
# ========================
SEAT_SCRIPT_SRC := seat-config/start_nested_x.sh
SEAT_SCRIPT_TGT := $(basename $(notdir $(SEAT_SCRIPT_SRC)))

# ========================
# Display driver lib (.so)
# ========================
DISPLAY_DRIVER_SRC := display/display_driver.c \
	display/display_api.c \
	display/frame_buffer.c \
	$(DISPLAY_DRIVER_CORE)

DISPLAY_BUILD_DIR := $(BUILD_DIR)/display

DISPLAY_DRIVER_LIB := $(DISPLAY_BUILD_DIR)/libdisplay_driver.so.$(VERSION)

# Extract major version from DISPLAY_DRIVER_LIB
LIB_DISPLAY_DRIVER_FILENAME := $(notdir $(DISPLAY_DRIVER_LIB))
DISPLAY_DRIVER_SONAME := $(DISPLAY_BUILD_DIR)/libdisplay_driver.so.$(firstword $(subst ., ,$(subst libdisplay_driver.so., ,$(LIB_DISPLAY_DRIVER_FILENAME))))

$(info DISPLAY_DRIVER_SONAME = $(DISPLAY_DRIVER_SONAME))
$(info LIB_DISPLAY_DRIVER_FILENAME = $(LIB_DISPLAY_DRIVER_FILENAME))

# ========================
# Static lib
# ========================
LIB_DIR := $(BUILD_DIR)/lib
LIB_AR := $(LIB_DIR)/libd_globals.a

LIB_SRC := $(wildcard lib/*.c)
LIB_OBJ := $(LIB_SRC:lib/%.c=$(LIB_DIR)/%.o)

# ========================
# Display manager binary
# ========================
DISPLAY_SRC := display/display_manager.c
			
DISPLAY_BIN := $(DISPLAY_BUILD_DIR)/display

# ========================
# FFI variables
# ========================
FFI_BUILD_DIR := $(BUILD_DIR)/ffi
PY_FFI_BUILD_DIR := $(FFI_BUILD_DIR)/python

# ========================
# Python FFI
# ========================
PY_FFI_SRC := ffi/python

# ========================
# Systemd units
# ========================
SYSTEMD_UNITS := \
    systemd/instance-1-display.service \
    systemd/instance-2-display.service \

SYSTEMD_SCRIPT_SRC := systemd/instance-display-generator.sh
SYSTEMD_SCRIPT_TGT := $(basename $(notdir $(SYSTEMD_SCRIPT_SRC)))

# ========================
# General examples
# ========================
EXAMPLE_OUT_DIR := $(BUILD_DIR)/examples
EXAMPLE_SRC_DIR := examples

# ========================
# C examples
# ========================
C_EXAMPLE_SRC_DIR := $(EXAMPLE_SRC_DIR)/c
C_EXAMPLE_OUT_DIR := $(EXAMPLE_OUT_DIR)/c
C_EXAMPLE_SRC := $(wildcard $(C_EXAMPLE_SRC_DIR)/*.c)

C_EXAMPLE_OUT := $(patsubst $(C_EXAMPLE_SRC_DIR)/%.c, \
                            $(C_EXAMPLE_OUT_DIR)/%, \
                            $(C_EXAMPLE_SRC))

# ========================
# Rust examples
# ========================
RUST_EXAMPLE_OUT_DIR := $(EXAMPLE_OUT_DIR)/rust
RUST_MANIFEST_PATH := $(EXAMPLE_SRC_DIR)/rust/Cargo.toml

# ========================
# Install target locations
# ========================
INSTALL_ROOT_BIN = /usr/local/bin/seat_display_driver
INSTALL_ROOT_LIB = /usr/local/lib/seat_display_driver
INSTALL_PROXY_DIR = $(INSTALL_ROOT_BIN)/proxy
INSTALL_PROXY_TARGET_DIR = $(INSTALL_PROXY_DIR)/target
INSTALL_DISPLAY_DIR = $(INSTALL_ROOT_BIN)/display
INSTALL_SCRIPT_DIR = $(INSTALL_ROOT_BIN)
INSTALL_DISPLAY_DRIVER_DIR = $(INSTALL_ROOT_LIB)/display
INSTALL_FFI_DIR = $(INSTALL_ROOT_LIB)/ffi
INSTALL_PY_FFI_DIR = $(INSTALL_FFI_DIR)/python
ASSETS_DIR = /usr/local/src/seat_display_driver/assets
INSTALL_SYSTEM_DIR = /usr/lib/systemd/system

SYSTEMD_DIR = /etc/systemd/system
LOCAL_BIN = /usr/local/bin

# ========================
# Phony targets
# ========================
.PHONY: all clean install check-deps ffi python_ffi rust_ffi example_all example_c example_rust

### TODO: for now ffi for all languages are build every time,
# but this should not be the case -> make it language specific

# ========================
# Default target: build everything (without inluding examples)
# ========================
all: check-deps $(LIB_AR) $(PROXY_BIN) $(SEAT_BINS) $(DISPLAY_DRIVER_LIB) $(DISPLAY_BIN) ffi rust_ffi

# ========================
# build examples
# ========================
example_all: example_c example_rust

# ========================
# build only c examples
# ========================
example_c: $(C_EXAMPLE_OUT)

# ========================
# build only rust examples
# ========================
example_rust: rust_ffi

# ========================
# Build static lib
# ========================
$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

$(LIB_AR): $(LIB_OBJ) 
	ar rcs $@ $^

$(LIB_DIR)/%.o: lib/%.c | $(LIB_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ========================
# Build proxy (main)
# ========================
$(PROXY_BIN): $(PROXY_SRC)
	@mkdir -p $(PROXY_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built proxy -> $@"

# ========================
# Build seat target binaries (each links/includes seat_utility.c)
# ========================
$(SEAT_BINS): $(SEAT_BUILD_DIR)/%: seat-config/%.c $(SEAT_UTIL_SRC) $(GLOBAL_UTIL_SRC)
	@mkdir -p $(SEAT_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(shell pkg-config --libs OpenCL)
	@echo "Built seat binary -> $@"

# ========================
# Build display directory
# ========================
$(DISPLAY_BUILD_DIR):
	@mkdir -p $(DISPLAY_BUILD_DIR)

# ========================
# Build display_driver
# ========================
$(DISPLAY_DRIVER_LIB): $(DISPLAY_DRIVER_SRC) $(GLOBAL_UTIL_SRC) $(LIB_AR) | $(DISPLAY_BUILD_DIR)
	$(CC) $(CFLAGS) $(PYTHON_CFLAGS) $^ $(LDFLAGS) $(PYTHON_LDFLAGS) -L$(abspath $(dir $(LIB_AR))) -ld_globals -o $@
	# Create SONAME and default symlink
	ln -s $@ $(DISPLAY_DRIVER_SONAME)
	ln -s $(DISPLAY_DRIVER_SONAME) $(DISPLAY_BUILD_DIR)/libdisplay_driver.so

	@echo "Built lib-display_driver -> $@"

$(info $(dir $(DISPLAY_DRIVER_LIB)))

# ========================
# Build display_manager
# ========================
$(DISPLAY_BIN): $(DISPLAY_SRC) | $(DISPLAY_BUILD_DIR) $(DISPLAY_DRIVER_LIB)
	$(CC) $(CFLAGS) $(PYTHON_CFLAGS) -o $@ $^ -L$(abspath $(dir $(DISPLAY_DRIVER_LIB))) -ldisplay_driver
	@echo "Built display_manager -> $@"

# ========================
# Build out directories
# ========================
$(RUST_EXAMPLE_OUT_DIR):
	@mkdir -p $(RUST_EXAMPLE_OUT_DIR)

$(C_EXAMPLE_OUT_DIR):
	@mkdir -p $(C_EXAMPLE_OUT_DIR)

# ========================
# Build ffi directory
# ========================
$(FFI_BUILD_DIR):
	@mkdir -p $(FFI_BUILD_DIR)
	
# ========================
# FFI
# ========================
ffi: python_ffi | $(DISPLAY_DRIVER_LIB) $(FFI_BUILD_DIR) $(RUST_EXAMPLE_OUT_DIR) $(C_EXAMPLE_OUT_DIR)
	
# ========================
# Python FFI
# ========================
python_ffi:
	@mkdir -p $(PY_FFI_BUILD_DIR)
	$(SWIG_PYTHON) $(SWIG_OPTIONS) -Ilib -Idisplay -o $(FFI_BUILD_DIR)/display_api_wrap.c -outdir $(PY_FFI_BUILD_DIR) $(PY_FFI_SRC)/$(SWIG_INTERFACE)
	$(CC) -fPIC -c $(FFI_BUILD_DIR)/display_api_wrap.c $(PYTHON_CFLAGS) -Ilib -Idisplay -o $(FFI_BUILD_DIR)/display_api_wrap.o
	$(CC) -shared -o $(PY_FFI_BUILD_DIR)/_display_api.so $(FFI_BUILD_DIR)/display_api_wrap.o -L$(abspath $(DISPLAY_BUILD_DIR)) -ldisplay_driver -L$(abspath $(dir $(LIB_AR))) -ld_globals

# ========================
# compile C examples
# ========================
$(C_EXAMPLE_OUT_DIR)/%: $(C_EXAMPLE_SRC_DIR)/%.c | $(DISPLAY_BUILD_DIR) $(DISPLAY_DRIVER_LIB)
	$(CC) $(CFLAGS) $(PYTHON_CFLAGS) $< -L$(abspath $(DISPLAY_BUILD_DIR)) -ldisplay_driver -o $@

# ========================
# compile Rust examples
# ========================
rust_ffi: | $(DISPLAY_BUILD_DIR) $(DISPLAY_DRIVER_LIB)
	PATH=$(HOME)/.cargo/bin:$$PATH cargo build --target-dir $(RUST_EXAMPLE_OUT_DIR) --bins --release --manifest-path $(RUST_MANIFEST_PATH)

# ========================
# Install everything to system paths
# ========================
install: all
	@echo "Installing binaries and supporting files..."

	# -- assets --
	@mkdir -p $(ASSETS_DIR)

	# -- proxy binary --
	@mkdir -p $(INSTALL_PROXY_DIR)
	@cp $(PROXY_BIN) $(INSTALL_PROXY_DIR)/
	@echo "Installed proxy to $(INSTALL_PROXY_DIR)/"

	# -- seat target binaries (as separate executables) --
	@mkdir -p $(INSTALL_PROXY_TARGET_DIR)
	@for bin in $(SEAT_BINS); do \
		cp $$bin $(INSTALL_PROXY_TARGET_DIR)/$$(basename $$bin); \
		echo "Installed seat binary -> $$(basename $$bin)"; \
	done

	@echo "Installed seat binaries to $(INSTALL_PROXY_TARGET_DIR)/"

	# -- install start_nested_x.sh to root install dir --
	@mkdir -p $(INSTALL_SCRIPT_DIR)
	@cp $(SEAT_SCRIPT_SRC) $(INSTALL_SCRIPT_DIR)/$(SEAT_SCRIPT_TGT)
	@chmod +x $(INSTALL_SCRIPT_DIR)/$(SEAT_SCRIPT_TGT)
	@echo "Installed $(SEAT_SCRIPT_TGT) to $(INSTALL_SCRIPT_DIR)/"

	# -- display_manager --
	@mkdir -p $(INSTALL_DISPLAY_DIR)
	@cp $(DISPLAY_BIN) $(INSTALL_DISPLAY_DIR)/
	@echo "Installed display_manager to $(INSTALL_DISPLAY_DIR)/"

	# -- display_driver --
	@mkdir -p $(INSTALL_DISPLAY_DRIVER_DIR)
	@cp $(DISPLAY_DRIVER_LIB) $(INSTALL_DISPLAY_DRIVER_DIR)/

	# Create symlinks inside install directory
	@ln -sf $(notdir $(DISPLAY_DRIVER_LIB)) \
	     $(INSTALL_DISPLAY_DRIVER_DIR)/$(notdir $(DISPLAY_DRIVER_SONAME))
	@ln -sf $(notdir $(DISPLAY_DRIVER_SONAME)) \
	     $(INSTALL_DISPLAY_DRIVER_DIR)/libdisplay_driver.so

	@echo "Installed display_driver to $(INSTALL_DISPLAY_DRIVER_DIR)/"

	# -- ffi --
	@cp -R $(FFI_BUILD_DIR)/. $(INSTALL_FFI_DIR)/
	@echo "Installed ffi"

	# -- systemd units --
	@for u in $(SYSTEMD_UNITS); do \
		cp $$u $(INSTALL_SYSTEM_DIR)/; \
		echo "Installed systemd unit: $$u"; \
		unit_name=$$(basename $$u); \
		ln -sf $(INSTALL_SYSTEM_DIR)/$$unit_name $(SYSTEMD_DIR)/$$unit_name; \
		echo "Created symlink: $(SYSTEMD_DIR)/$$unit_name -> $(INSTALL_SYSTEM_DIR)/$$unit_name"; \
	done

	# -- install instance-display-generator.sh to root install dir --
	@cp $(SYSTEMD_SCRIPT_SRC) $(INSTALL_SCRIPT_DIR)/$(SYSTEMD_SCRIPT_TGT)
	@chmod +x $(INSTALL_SCRIPT_DIR)/$(SYSTEMD_SCRIPT_TGT)
	@echo "Installed $(SYSTEMD_SCRIPT_TGT) to $(INSTALL_SCRIPT_DIR)/"


	# -- install env files --
	@mkdir -p $(GLOBAL_CONFIG_DIR)
	@chgrp seat_display_driver $(GLOBAL_CONFIG_DIR) && chmod 2750 $(GLOBAL_CONFIG_DIR)
	@if [ ! -f $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG) ]; then \
		echo "NX_WIDTH=$(NX_WIDTH)" > $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "NX_HEIGHT=$(NX_HEIGHT)" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "DISPLAY=$$DISPLAY" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "XAUTHORITY=$$XAUTHORITY" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "DEBUG=$(DEBUG)" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "BORDER_WIDTH=$(BORDER_WIDTH)" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "NX_OFF_X=$(NX_OFF_X)" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "NX_OFF_Y=$(NX_OFF_Y)" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "DEBUG=$(DEBUG)" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "MAX_CLIENTS=$(MAX_CLIENTS)" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
	fi

	@echo "Install complete."

# ========================
# Check installed depencies
# ========================
check-deps:
	@echo "Checking required libraries..."
	@pkg-config --exists libdrm || echo "ERROR: libdrm-dev missing"
	@pkg-config --exists x11 || echo "ERROR: libx11-dev missing"
	@pkg-config --exists OpenCL || echo "WARNING: OpenCL development files missing"
	@command -v Xnest >/dev/null 2>&1 || echo "ERROR: Xnest executable not found"
	@echo "-> Dependency check complete."

# ========================
# Clean build artifacts
# ========================
clean-build: clean-install
	@rm -rf $(BUILD_DIR)
	@echo "-> Cleaned build directory."

# ========================
# Clean install artifacts
# ========================
clean-install:
	@rm -rf $(INSTALL_ROOT_BIN)
	@rm -rf $(INSTALL_ROOT_LIB)
	@rm -rf $(ASSETS_DIR)
	@echo "-> Cleaned install directories."

# ========================
# Clean env
# ========================
clean-env:
	@rm -rf $(GLOBAL_CONFIG_DIR)
	@echo "-> Removed env."

clean-examples:
	@rm -rf $(EXAMPLE_OUT_DIR)
	@echo "-> Removed examples."

# ========================
# Unlink system files
# ========================
unlink-files:
	@for u in $(SYSTEMD_UNITS); do \
		unit_name=$$(basename $$u); \
		unlink $(SYSTEMD_DIR)/$$unit_name; \
		echo "Unlinked symlink: $(SYSTEMD_DIR)/$$unit_name -> $(INSTALL_SYSTEM_DIR)/$$unit_name"; \
		rm $(INSTALL_SYSTEM_DIR)/$$unit_name; \
		echo "-> Removed systemd unit: $$unit_name"; \
	done
	@echo "-> Unlinked system files"

# ========================
# Clean all artifacts(without examples)
# ========================
clean-all: clean-build clean-install unlink-files clean-env
