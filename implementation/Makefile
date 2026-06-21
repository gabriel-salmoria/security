#!/usr/bin/make -f
SHELL     := /bin/bash
MAKEFLAGS += --silent

# ── Directories ────────────────────────────────────────────────────────────────
ROOT    := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
TP      := $(ROOT)/third_party
LOCAL   := $(TP)/local
SRC_TP  := $(TP)/src
IMGUI   := $(TP)/imgui
SRC     := $(ROOT)/src
BUILD   := $(ROOT)/build

# ── Versions & URLs ────────────────────────────────────────────────────────────
GMP_VER     := 6.3.0
PBC_VER     := 0.5.14
IMGUI_VER   := v1.91.9
IMGUI_BASE  := https://raw.githubusercontent.com/ocornut/imgui/$(IMGUI_VER)

GMP_URL  := https://gmplib.org/download/gmp/gmp-$(GMP_VER).tar.xz
PBC_URL  := https://crypto.stanford.edu/pbc/files/pbc-$(PBC_VER).tar.gz

# ── Artifact paths ─────────────────────────────────────────────────────────────
GMP_TAR  := $(SRC_TP)/gmp-$(GMP_VER).tar.xz
PBC_TAR  := $(SRC_TP)/pbc-$(PBC_VER).tar.gz
GMP_SRC  := $(SRC_TP)/gmp-$(GMP_VER)
PBC_SRC  := $(SRC_TP)/pbc-$(PBC_VER)
GMP_LIB  := $(LOCAL)/lib/libgmp.a
PBC_LIB  := $(LOCAL)/lib/libpbc.a
IMGUI_STAMP := $(IMGUI)/.downloaded

NPROC := $(shell nproc 2>/dev/null || echo 4)

# ── Compiler & flags ───────────────────────────────────────────────────────────
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall \
             -I$(SRC) \
             -I$(LOCAL)/include \
             -I$(IMGUI) \
             -I$(IMGUI)/backends

LDFLAGS  := -L$(LOCAL)/lib
LDLIBS   := -lpbc -lgmp -lcrypto -lSDL2 -lGL -ldl

TARGET   := $(ROOT)/ibemail

# ── Source collection ──────────────────────────────────────────────────────────
SRCS_PROJECT := $(shell find $(SRC) -name '*.cpp' 2>/dev/null)
SRCS_IMGUI   := $(IMGUI)/imgui.cpp \
                $(IMGUI)/imgui_draw.cpp \
                $(IMGUI)/imgui_tables.cpp \
                $(IMGUI)/imgui_widgets.cpp \
                $(IMGUI)/backends/imgui_impl_sdl2.cpp \
                $(IMGUI)/backends/imgui_impl_opengl3.cpp

SRCS_ALL := $(SRCS_PROJECT) $(SRCS_IMGUI)
OBJS     := $(patsubst $(ROOT)/%.cpp, $(BUILD)/%.o, $(SRCS_ALL))

# ── Top-level targets ──────────────────────────────────────────────────────────
.PHONY: all deps gmp pbc imgui check-sysdeps clean distclean help

all: check-sysdeps deps $(TARGET)

deps: gmp pbc imgui

help:
	echo ""
	echo "  make deps      — download and build all third-party libraries locally"
	echo "  make           — build the ibemail application"
	echo "  make clean     — remove build artifacts"
	echo "  make distclean — remove everything including compiled deps"
	echo ""

# ── System dependency check ────────────────────────────────────────────────────
# SDL2, OpenGL, and OpenSSL headers must be present (provided by the OS).
# Ubuntu:  sudo apt install libsdl2-dev libssl-dev libgl-dev pkg-config
# Arch:    sudo pacman -S sdl2 openssl mesa
check-sysdeps:
	@command -v pkg-config >/dev/null 2>&1 || \
		{ echo "ERROR: pkg-config not found. Install it first."; exit 1; }
	@pkg-config --exists sdl2 2>/dev/null || \
		{ echo "ERROR: SDL2 not found."; \
		  echo "  Ubuntu: sudo apt install libsdl2-dev"; \
		  echo "  Arch:   sudo pacman -S sdl2"; exit 1; }
	@pkg-config --exists openssl 2>/dev/null || \
		{ echo "ERROR: OpenSSL not found."; \
		  echo "  Ubuntu: sudo apt install libssl-dev"; \
		  echo "  Arch:   sudo pacman -S openssl"; exit 1; }
	@echo "[deps] system libraries OK (SDL2, OpenSSL, OpenGL)"

# ── GMP ────────────────────────────────────────────────────────────────────────
gmp: $(GMP_LIB)

$(GMP_TAR):
	@mkdir -p $(SRC_TP)
	@echo "[gmp] downloading GMP $(GMP_VER)..."
	curl -L --progress-bar -o $@ $(GMP_URL)

$(GMP_SRC)/configure: $(GMP_TAR)
	@echo "[gmp] extracting..."
	tar xf $< -C $(SRC_TP)
	@touch $@

$(GMP_LIB): $(GMP_SRC)/configure
	@echo "[gmp] configuring..."
	@cd $(GMP_SRC) && \
		CC=gcc CFLAGS="-std=gnu17 -O2" \
		./configure \
			--prefix=$(LOCAL) \
			--enable-static \
			--disable-shared \
			--quiet
	@echo "[gmp] building (this takes ~30s)..."
	@$(MAKE) -C $(GMP_SRC) -j$(NPROC) --quiet
	@$(MAKE) -C $(GMP_SRC) install --quiet
	@echo "[gmp] done → $(GMP_LIB)"

# ── PBC ────────────────────────────────────────────────────────────────────────
pbc: $(PBC_LIB)

$(PBC_TAR):
	@mkdir -p $(SRC_TP)
	@echo "[pbc] downloading PBC $(PBC_VER)..."
	curl -L --progress-bar -o $@ $(PBC_URL)

$(PBC_SRC)/configure: $(PBC_TAR)
	@echo "[pbc] extracting..."
	tar xf $< -C $(SRC_TP)
	@touch $@

$(PBC_LIB): $(PBC_SRC)/configure $(GMP_LIB)
	@echo "[pbc] configuring..."
	@cd $(PBC_SRC) && \
		CC=gcc \
		CFLAGS="-std=gnu17 -O2 -I$(LOCAL)/include" \
		LDFLAGS="-L$(LOCAL)/lib" \
		./configure \
			--prefix=$(LOCAL) \
			--enable-static \
			--disable-shared \
			--quiet
	@echo "[pbc] building..."
	@$(MAKE) -C $(PBC_SRC) -j$(NPROC) --quiet
	@$(MAKE) -C $(PBC_SRC) install --quiet
	@echo "[pbc] done → $(PBC_LIB)"

# ── ImGui ──────────────────────────────────────────────────────────────────────
imgui: $(IMGUI_STAMP)

IMGUI_CORE_FILES := \
	imgui.h imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp \
	imgui_internal.h imconfig.h imstb_rectpack.h imstb_textedit.h imstb_truetype.h

IMGUI_BACKEND_FILES := \
	imgui_impl_sdl2.h imgui_impl_sdl2.cpp \
	imgui_impl_opengl3.h imgui_impl_opengl3.cpp imgui_impl_opengl3_loader.h

$(IMGUI_STAMP):
	@mkdir -p $(IMGUI)/backends
	@echo "[imgui] downloading ImGui $(IMGUI_VER)..."
	@for f in $(IMGUI_CORE_FILES); do \
		echo "  $$f"; \
		curl -sL -o $(IMGUI)/$$f $(IMGUI_BASE)/$$f || exit 1; \
	done
	@for f in $(IMGUI_BACKEND_FILES); do \
		echo "  backends/$$f"; \
		curl -sL -o $(IMGUI)/backends/$$f $(IMGUI_BASE)/backends/$$f || exit 1; \
	done
	@touch $(IMGUI_STAMP)
	@echo "[imgui] done"

# ── Project build ──────────────────────────────────────────────────────────────
$(TARGET): $(OBJS)
	@echo "[link] $@"
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD)/%.o: $(ROOT)/%.cpp
	@mkdir -p $(dir $@)
	@echo "[cc]   $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ── Cleanup ────────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BUILD) $(TARGET)

distclean: clean
	rm -rf $(LOCAL) $(GMP_SRC) $(PBC_SRC) $(IMGUI)
	@echo "distclean done. Run 'make deps' to rebuild from scratch."
