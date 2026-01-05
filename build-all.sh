#!/bin/bash
# Build script for all platforms

set -e

echo "=== xmod Multi-Platform Build Script ==="
echo ""

BUILD_DIR="release"
mkdir -p "$BUILD_DIR"

# Windows 32-bit
echo "Building Windows 32-bit..."
make clean
make PLATFORM=mingw32 ARCH=x86 -j$(nproc)
mkdir -p "$BUILD_DIR/windows-32bit"
find build -name "*.dll" -exec cp {} "$BUILD_DIR/windows-32bit/" \; 2>/dev/null || true

# Windows 64-bit  
echo "Building Windows 64-bit..."
make clean
make PLATFORM=mingw32 ARCH=x86_64 -j$(nproc)
mkdir -p "$BUILD_DIR/windows-64bit"
find build -name "*.dll" -exec cp {} "$BUILD_DIR/windows-64bit/" \; 2>/dev/null || true

# Linux 32-bit
echo "Building Linux 32-bit..."
make clean
make PLATFORM=linux ARCH=x86 -j$(nproc)
mkdir -p "$BUILD_DIR/linux-32bit"
find build -name "*.so" -exec cp {} "$BUILD_DIR/linux-32bit/" \; 2>/dev/null || true

# Linux 64-bit
echo "Building Linux 64-bit..."
make clean
make PLATFORM=linux ARCH=x86_64 -j$(nproc)
mkdir -p "$BUILD_DIR/linux-64bit"
find build -name "*.so" -exec cp {} "$BUILD_DIR/linux-64bit/" \; 2>/dev/null || true

echo ""
echo "=== Build complete! ==="
echo "Output in ./$BUILD_DIR/"
ls -lh "$BUILD_DIR"/*/
