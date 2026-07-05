#!/usr/bin/make -f
SHELL     := /bin/bash
MAKEFLAGS += --silent

# Local, sudo-free prefix used when pbc/gmp aren't found system-wide (see `setup`).
LOCAL_TP := implementation/third_party/local

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I./implementation/src -I/usr/include/SDL2 -I./implementation/imgui -I./implementation/imgui/backends -I$(LOCAL_TP)/include
LDFLAGS = -L$(LOCAL_TP)/lib -lpbc -lgmp -lcrypto -lSDL2 -lGL -ldl

SRCS := $(shell find implementation/src -name "*.cpp")
IMGUI_SRCS := $(wildcard implementation/imgui/*.cpp) $(wildcard implementation/imgui/backends/imgui_impl_sdl2.cpp) $(wildcard implementation/imgui/backends/imgui_impl_opengl3.cpp)

ALL_SRCS = $(SRCS) $(IMGUI_SRCS)
OBJS = $(ALL_SRCS:%=implementation/obj/%.o)

TARGET = implementation/ibemail_app

.PHONY: all build run setup check-deps check-sysdeps rel-1 rel-2 reports clean clean-reports veryclean clean_app build_app help

# `make` builds just the application code, not the LaTeX reports.
all: build ## Build the application (same as `build`; default target)

build: check-sysdeps check-deps $(TARGET) ## Build the application (compiles sources, links the binary)

run: build ## Build (if needed) and run the application
	./$(TARGET)

# Verifies the compiler + SDL2/GMP/OpenSSL are installed, with a clear message
# and a non-zero exit instead of a wall of cryptic compiler/linker errors.
check-sysdeps: ## (internal) verifies compiler + SDL2/GMP/OpenSSL are installed
	@ok=1; \
	if ! command -v $(CXX) >/dev/null 2>&1; then \
		echo "==> $(CXX) not found. Install a C++17 compiler, e.g.:"; \
		echo "==>   Arch:          sudo pacman -S base-devel"; \
		echo "==>   Debian/Ubuntu: sudo apt-get install build-essential"; \
		ok=0; \
	fi; \
	if ! echo '#include <SDL2/SDL.h>' | $(CXX) -E -xc++ -I/usr/include/SDL2 - >/dev/null 2>&1; then \
		echo "==> SDL2 development headers not found. Install them, e.g.:"; \
		echo "==>   Arch:          sudo pacman -S sdl2"; \
		echo "==>   Debian/Ubuntu: sudo apt-get install libsdl2-dev"; \
		echo "==>   Fedora:        sudo dnf install SDL2-devel"; \
		ok=0; \
	fi; \
	if ! echo '#include <gmp.h>' | $(CXX) -E -xc++ -I$(LOCAL_TP)/include - >/dev/null 2>&1; then \
		echo "==> GMP development headers not found. Install them, e.g.:"; \
		echo "==>   Arch:          sudo pacman -S gmp"; \
		echo "==>   Debian/Ubuntu: sudo apt-get install libgmp-dev"; \
		echo "==>   Fedora:        sudo dnf install gmp-devel"; \
		ok=0; \
	fi; \
	if ! echo '#include <openssl/evp.h>' | $(CXX) -E -xc++ - >/dev/null 2>&1; then \
		echo "==> OpenSSL development headers not found. Install them, e.g.:"; \
		echo "==>   Arch:          sudo pacman -S openssl"; \
		echo "==>   Debian/Ubuntu: sudo apt-get install libssl-dev"; \
		echo "==>   Fedora:        sudo dnf install openssl-devel"; \
		ok=0; \
	fi; \
	for lib in SDL2 gmp crypto GL; do \
		if ! echo 'int main(){return 0;}' | $(CXX) -xc++ - -o /dev/null -L$(LOCAL_TP)/lib -l$$lib >/dev/null 2>&1; then \
			echo "==> Could not link against lib$$lib — is it installed (runtime + dev package)?"; \
			ok=0; \
		fi; \
	done; \
	if [ "$$ok" = "0" ]; then \
		echo "==> Fix the missing dependencies above, then re-run make."; \
		exit 1; \
	fi

# Installs pbc/gmp locally inside the project if they aren't found anywhere.
check-deps: ## (internal) auto-runs `setup` before building if pbc is missing
	@if [ ! -f /usr/include/pbc/pbc.h ] && [ ! -f /usr/local/include/pbc/pbc.h ] && [ ! -f $(LOCAL_TP)/include/pbc/pbc.h ]; then \
		echo "==> pbc.h not found (system-wide or local)."; \
		echo "==> It will now be built from source and installed LOCALLY inside"; \
		echo "==>   $(LOCAL_TP)"; \
		echo "==> No system packages are touched and no sudo is required for this."; \
		$(MAKE) setup; \
	fi

setup: ## Install pbc locally inside the project, from source (no sudo)
	./scripts/setup-pbc.sh

# Kept as aliases so older invocations still work.
build_app: build

rel-1: ## Build the partial report PDF (rel-1/main.pdf)
	$(MAKE) -C rel-1

rel-2: ## Build the final report PDF (rel-2/main.pdf)
	$(MAKE) -C rel-2

reports: rel-1 rel-2 ## Build both LaTeX reports (rel-1 and rel-2)

$(TARGET): $(OBJS)
	printf ' %-6s %s\n' "LD" "$@"
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

implementation/obj/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	printf ' %-6s %s\n' "CC" "$<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean: ## Remove application build artifacts (object files + binary)
	rm -rf implementation/obj $(TARGET)

clean_app: clean

clean-reports: ## Remove LaTeX build artifacts for both reports
	$(MAKE) -C rel-1 clean
	$(MAKE) -C rel-2 clean

veryclean: clean ## Remove everything: app build artifacts and report build artifacts
	$(MAKE) -C rel-1 veryclean
	$(MAKE) -C rel-2 veryclean

help: ## Show this help message
	@echo "Available targets:"
	@grep -E '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-14s\033[0m %s\n", $$1, $$2}'
