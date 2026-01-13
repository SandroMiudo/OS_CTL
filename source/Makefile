# ========================
# Compiler settings
# ========================
CC = gcc
SWIG = swig
CFLAGS = -Wall -O2 \
	     -fPIC \
         -Idisplay \
		 -I.

LDFLAGS = -shared -Wl,-soname,$(DISPLAY_DRIVER_SONAME)

VERSION = 1.0.0

DISPLAY_DRIVER_DRM = DRM
DISPLAY_DRIVER_XLIB = XLIB

DISPLAY_DRIVER ?= XLIB

NX_HEIGHT ?= 1080
NX_WIDTH ?= 1920

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
SWIG_OPTIONS := -Wall

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
.PHONY: all clean install check-deps ffi

# ========================
# Default target: build everything
# ========================
all: check-deps $(PROXY_BIN) $(SEAT_BINS) $(DISPLAY_DRIVER_LIB) $(DISPLAY_BIN) ffi

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
$(DISPLAY_DRIVER_LIB): $(DISPLAY_DRIVER_SRC) | $(DISPLAY_BUILD_DIR)
	$(CC) $(CFLAGS) $(DISPLAY_DRIVER_SRC) $(LDFLAGS) -o $@
	# Create SONAME and default symlink
	ln -s $@ $(DISPLAY_DRIVER_SONAME) # todo : this is an unecassry link
	ln -s $(DISPLAY_DRIVER_SONAME) $(DISPLAY_BUILD_DIR)/libdisplay_driver.so

	@echo "Built lib-display_driver -> $@"

$(info $(dir $(DISPLAY_DRIVER_LIB)))

# ========================
# Build display_manager
# ========================
$(DISPLAY_BIN): $(DISPLAY_SRC) | $(DISPLAY_BUILD_DIR) $(DISPLAY_DRIVER_LIB)
	$(CC) $(CFLAGS) -o $@ $^ -L$(abspath $(dir $(DISPLAY_DRIVER_LIB))) -ldisplay_driver -Wl,-rpath=$(abspath $(dir $(DISPLAY_DRIVER_LIB)))
	@echo "Built display_manager -> $@"

# ========================
# Build ffi directory
# ========================
$(FFI_BUILD_DIR):
	@mkdir -p $(FFI_BUILD_DIR)

# ========================
# FFI
# ========================
ffi: python_ffi | $(DISPLAY_DRIVER_LIB) $(FFI_BUILD_DIR)
	
# ========================
# Python FFI
# ========================
python_ffi:
	@mkdir -p $(PY_FFI_BUILD_DIR)
	$(SWIG_PYTHON) $(SWIG_OPTIONS) -Idisplay -o $(FFI_BUILD_DIR)/display_api_wrap.c -outdir $(PY_FFI_BUILD_DIR) $(PY_FFI_SRC)/$(SWIG_INTERFACE)
	$(CC) -fPIC -c $(FFI_BUILD_DIR)/display_api_wrap.c $(shell python3-config --includes) -Idisplay -o $(FFI_BUILD_DIR)/display_api_wrap.o
	$(CC) -shared -o $(PY_FFI_BUILD_DIR)/_display_api.so $(FFI_BUILD_DIR)/display_api_wrap.o -L$(abspath $(DISPLAY_BUILD_DIR)) -ldisplay_driver

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
	@if [ ! -f $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG) ]; then \
		echo "NX_WIDTH=$(NX_WIDTH)" > $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "NX_HEIGHT=$(NX_HEIGHT)" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "DISPLAY=$$DISPLAY" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
		echo "XAUTHORITY=$$XAUTHORITY" >> $(GLOBAL_CONFIG_DIR)/$(GLOBAL_CONFIG); \
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
# Clean all artifacts
# ========================
clean-all: clean-build clean-install unlink-files clean-env
