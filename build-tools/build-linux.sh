#!/bin/bash
# WSL2 Linux build script for xmod
# This script builds both 32-bit and 64-bit Linux variants

set -e

echo "=== xmod Linux Build Script (WSL2) ==="
echo ""

# Check if we're running in WSL
if ! grep -qi microsoft /proc/version 2>/dev/null; then
    echo "WARNING: This doesn't appear to be WSL. Continuing anyway..."
fi

# Get the repository root (assume script is in build-tools/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT"

echo "Repository root: $REPO_ROOT"
echo ""

# Install dependencies if needed
echo "=== Checking dependencies ==="
if ! command -v gcc &> /dev/null || ! command -v g++ &> /dev/null; then
    echo "Installing build-essential..."
    sudo apt-get update
    sudo apt-get install -y build-essential
fi

if ! dpkg -l | grep -q gcc-multilib; then
    echo "Installing multilib support..."
    sudo apt-get update
    sudo apt-get install -y gcc-multilib g++-multilib
fi

if ! command -v python3 &> /dev/null; then
    echo "Installing python3..."
    sudo apt-get update
    sudo apt-get install -y python3
fi

if ! command -v zip &> /dev/null; then
    echo "Installing zip..."
    sudo apt-get update
    sudo apt-get install -y zip
fi

if ! command -v m4 &> /dev/null; then
    echo "Installing m4..."
    sudo apt-get update
    sudo apt-get install -y m4
fi

echo "All dependencies installed."
echo ""

# Make info.py executable
chmod +x "$REPO_ROOT/project/info.py" 2>/dev/null || true

# Build Linux 64-bit
echo "=== Building Linux 64-bit ==="
make clean || true
PLATFORM=linux64 make release

# Check if build succeeded
if [ ! -f "build.linux64-release/game/qagame.mp.x86_64.so" ]; then
    echo "ERROR: Linux 64-bit build failed - qagame.mp.x86_64.so not found"
    exit 1
fi

echo "Linux 64-bit build completed successfully"
echo ""

# Build Linux 32-bit
echo "=== Building Linux 32-bit ==="
make clean || true
PLATFORM=linux make release

# Check if build succeeded
if [ ! -f "build.linux-release/game/qagame.mp.i386.so" ]; then
    echo "ERROR: Linux 32-bit build failed - qagame.mp.i386.so not found"
    exit 1
fi

echo "Linux 32-bit build completed successfully"
echo ""

echo "=== Build Summary ==="
echo "Linux 64-bit binaries:"
ls -lh build.linux64-release/game/*.so build.linux64-release/cgame/*.so build.linux64-release/ui/*.so 2>/dev/null || echo "  (none found)"
echo ""
echo "Linux 32-bit binaries:"
ls -lh build.linux-release/game/*.so build.linux-release/cgame/*.so build.linux-release/ui/*.so 2>/dev/null || echo "  (none found)"
echo ""
echo "=== Linux builds complete! ==="
