#!/usr/bin/env bash
# Builds PBC (Pairing-Based Cryptography) from source and installs it LOCALLY
# inside the project, at implementation/third_party/local/. Nothing is
# installed system-wide and no sudo is used — the Makefile picks this up via
# -I/-L flags pointing at that local prefix.
#
# GMP is required too, but it is a common system dev package (already listed
# as a prerequisite in the README) — this script links against the system
# GMP rather than also building it from source, since GMP's old ./configure
# is known to choke on very recent GCC versions.
#
# The actual build happens in a throwaway directory outside the repo: PBC's
# vendored libtool has a long-standing bug where `make install` breaks if the
# destination path contains a space, which this repo's path may well have.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TP_DIR="$ROOT_DIR/implementation/third_party"
SRC_DIR="$TP_DIR/src"
PREFIX="$TP_DIR/local"

PBC_VERSION=0.5.14
PBC_TARBALL="pbc-$PBC_VERSION.tar.gz"

if [ -f "$PREFIX/include/pbc/pbc.h" ] && [ -f "$PREFIX/lib/libpbc.a" ]; then
    echo "pbc is already built locally at $PREFIX"
    exit 0
fi

if ! echo '#include <gmp.h>' | ${CC:-cc} -E -x c - >/dev/null 2>&1; then
    echo "GMP development headers not found." >&2
    echo "Install them first, then re-run this script:" >&2
    echo "  Arch:          sudo pacman -S gmp" >&2
    echo "  Debian/Ubuntu: sudo apt-get install libgmp-dev" >&2
    exit 1
fi

echo "=============================================================="
echo " pbc.h was not found (system-wide or local)."
echo " PBC will be downloaded and built from source, then installed"
echo " LOCALLY inside this project, at:"
echo "     $PREFIX"
echo " Nothing is installed system-wide and no sudo is required for"
echo " this step. The Makefile will link against this local copy"
echo " (GMP itself is used from the system, see requirements)."
echo "=============================================================="

missing=()
for tool in gcc g++ make flex bison; do
    command -v "$tool" >/dev/null 2>&1 || missing+=("$tool")
done
if [ "${#missing[@]}" -gt 0 ]; then
    echo "Missing build tools: ${missing[*]}" >&2
    echo "Install them with your package manager first, e.g.:" >&2
    echo "  Arch:          sudo pacman -S base-devel" >&2
    echo "  Debian/Ubuntu: sudo apt-get install build-essential flex bison" >&2
    exit 1
fi

mkdir -p "$SRC_DIR" "$PREFIX"
cd "$SRC_DIR"

if [ ! -f "$PBC_TARBALL" ]; then
    echo "Downloading PBC $PBC_VERSION..."
    wget -q "https://crypto.stanford.edu/pbc/files/$PBC_TARBALL"
fi

# Build in a space-free temp dir: PBC's bundled libtool mishandles install
# paths containing spaces, which this repo's path may have.
BUILD_DIR="$(mktemp -d)"
TMP_PREFIX="$(mktemp -d)"
trap 'rm -rf "$BUILD_DIR" "$TMP_PREFIX"' EXIT

tar -xf "$PBC_TARBALL" -C "$BUILD_DIR"

echo "Building PBC (static, against the system GMP)..."
(
    cd "$BUILD_DIR/pbc-$PBC_VERSION"
    ./configure --prefix="$TMP_PREFIX" --enable-static --disable-shared
    make -j"$(nproc)"
    make install
)

cp -a "$TMP_PREFIX"/. "$PREFIX"/
echo "pbc installed locally at $PREFIX"
