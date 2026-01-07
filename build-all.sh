#!/bin/bash
# Build script for all platforms with final package creation
# Usage: ./build-all.sh [command] [options]
#
# Commands:
#   release     Build release configuration (default, implies --clean)
#   debug       Build debug configuration
#   clean       Only clean all build directories (no build)
#   all         Alias for 'release' (build all platforms)
#
# Options:
#   --clean     Force clean build (rebuild everything from scratch)
#   --no-clean  Incremental build (only recompile changed files)
#
# Examples:
#   ./build-all.sh                  # Release build with clean
#   ./build-all.sh release          # Release build with clean
#   ./build-all.sh debug            # Debug build (incremental)
#   ./build-all.sh debug --clean    # Debug build with clean
#   ./build-all.sh clean            # Only clean, no build
set -e
VARIANT="release"
CLEAN_BUILD=-1  # -1 = auto (clean for release, no-clean for debug)
CLEAN_ONLY=0    # 1 = only clean, don't build
JOBS="${JOBS:-$(nproc)}"
# Parse arguments
for arg in "$@"; do
    case "$arg" in
        release|all)
            VARIANT="release"
            ;;
        debug)
            VARIANT="debug"
            ;;
        clean)
            CLEAN_ONLY=1
            ;;
        --clean)
            CLEAN_BUILD=1
            ;;
        --no-clean)
            CLEAN_BUILD=0
            ;;
        --help|-h)
            echo "Usage: $0 [command] [options]"
            echo ""
            echo "Commands:"
            echo "  release     Build release configuration (default, implies --clean)"
            echo "  debug       Build debug configuration"
            echo "  clean       Only clean all build directories (no build)"
            echo "  all         Alias for 'release'"
            echo ""
            echo "Options:"
            echo "  --clean     Force clean build"
            echo "  --no-clean  Incremental build"
            echo "  --help      Show this help"
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [release|debug|clean|all] [--clean|--no-clean]"
            echo "Use --help for more information."
            exit 1
            ;;
    esac
done
# Auto-determine clean build: clean for release, incremental for debug
if [ $CLEAN_BUILD -eq -1 ]; then
    if [ "$VARIANT" = "release" ]; then
        CLEAN_BUILD=1
    else
        CLEAN_BUILD=0
    fi
fi
# Function to clean all build directories
clean_all() {
    echo "=== Cleaning all build directories ==="
    local dirs_to_clean=(
        "build.linux-release"
        "build.linux-debug"
        "build.linux64-release"
        "build.linux64-debug"
        "build.mingw-release"
        "build.mingw-debug"
        "build.mingw64-release"
        "build.mingw64-debug"
        "build-final-release"
        "build-final-debug"
    )
    for dir in "${dirs_to_clean[@]}"; do
        if [ -d "$dir" ]; then
            echo "Removing $dir..."
            rm -rf "$dir"
        fi
    done
    echo "Clean complete."
}
# Handle clean-only mode
if [ $CLEAN_ONLY -eq 1 ]; then
    clean_all
    exit 0
fi
echo "=== xmod Multi-Platform Build Script ==="
echo "Variant: $VARIANT"
echo "Clean build: $([ $CLEAN_BUILD -eq 1 ] && echo 'yes' || echo 'no')"
echo "Parallel jobs: $JOBS"
echo ""
# Function to get build directory for a platform
get_build_dir() {
    local PLATFORM="$1"
    echo "build.${PLATFORM}-${VARIANT}"
}
# Function to build a platform using 'make pkg' to get all processed files
build_platform() {
    local PLATFORM="$1"
    local PLATFORM_NAME="$2"
    local BUILD_DIR=$(get_build_dir "$PLATFORM")
    echo "----------------------------------------"
    echo "Building $PLATFORM_NAME ($VARIANT)..."
    echo "----------------------------------------"
    # Clean build if requested
    if [ $CLEAN_BUILD -eq 1 ]; then
        echo "Cleaning $BUILD_DIR..."
        rm -rf "$BUILD_DIR"
    fi
    # Clean pkg targets to ensure they get rebuilt
    make PLATFORM="$PLATFORM" VARIANT="$VARIANT" pkg.clean 2>/dev/null || true
    # Build everything including pkg
    make PLATFORM="$PLATFORM" VARIANT="$VARIANT" pkg -j"$JOBS"
    echo "Done: $PLATFORM_NAME"
    echo ""
}
# Build all platforms first
echo "========================================"
echo "=== Phase 1: Building all platforms ==="
echo "========================================"
build_platform "linux"   "Linux 32-bit"
build_platform "linux64" "Linux 64-bit"
build_platform "mingw"   "Windows 32-bit"
build_platform "mingw64" "Windows 64-bit"
# Get project info from generated make file (after first build)
REFERENCE_BUILD=$(get_build_dir "linux64")
PROJECT_MK="$REFERENCE_BUILD/make/project.mk"
if [ ! -f "$PROJECT_MK" ]; then
    echo "ERROR: Could not find $PROJECT_MK"
    exit 1
fi
# Parse project variables
PROJECT_NAME=$(grep "^PROJECT.name " "$PROJECT_MK" | sed 's/.*= *//')
PROJECT_VERSION=$(grep "^PROJECT.version " "$PROJECT_MK" | sed 's/.*= *//')
PROJECT_PACKAGE=$(grep "^PROJECT.packageBase " "$PROJECT_MK" | sed 's/.*= *//')
PROJECT_PK3=$(grep "^PROJECT.pk3 " "$PROJECT_MK" | sed 's/.*= *//')
echo ""
echo "Project: $PROJECT_NAME $PROJECT_VERSION"
echo "Package: $PROJECT_PACKAGE"
echo "PK3: $PROJECT_PK3"
echo ""
# Final output directories
FINAL_DIR="build-final-${VARIANT}"
FINAL_PAK="${FINAL_DIR}/pak"
FINAL_PKG="${FINAL_DIR}/${PROJECT_PACKAGE}"
OUTPUT_TAR="${FINAL_DIR}/${PROJECT_PACKAGE}.tar.gz"
OUTPUT_PK3="${FINAL_PKG}/${PROJECT_PK3}"
# Clean and create output directories
rm -rf "$FINAL_DIR"
mkdir -p "$FINAL_PAK"
mkdir -p "$FINAL_PKG"
echo "========================================"
echo "=== Phase 2: Collecting pak files ==="
echo "========================================"
# Copy pak directory structure from reference build (includes all subdirs like animations/, gfx/, etc.)
echo "Copying pak structure from $REFERENCE_BUILD/pak/..."
rsync -av --exclude='*.so' --exclude='*.dll' "$REFERENCE_BUILD/pak/" "$FINAL_PAK/"
echo "========================================"
echo "=== Phase 3: Collecting binaries ==="
echo "========================================"
# Function to collect binaries from a platform build
collect_binaries() {
    local PLATFORM="$1"
    local FILE_EXT="$2"
    local BUILD_DIR=$(get_build_dir "$PLATFORM")
    echo "Collecting binaries from $BUILD_DIR..."
    # Copy cgame to pak (for pk3) - these go in root of pak
    for f in "$BUILD_DIR"/pak/cgame*"$FILE_EXT"; do
        if [ -f "$f" ]; then
            cp -v "$f" "$FINAL_PAK/"
        fi
    done
    # Copy ui to pak (for pk3) - these go in root of pak
    for f in "$BUILD_DIR"/pak/ui*"$FILE_EXT"; do
        if [ -f "$f" ]; then
            cp -v "$f" "$FINAL_PAK/"
        fi
    done
    # Copy qagame to package directory (server-side, not in pk3)
    # qagame is in the game/ subdirectory of the build
    for f in "$BUILD_DIR"/game/qagame*"$FILE_EXT"; do
        if [ -f "$f" ]; then
            cp -v "$f" "$FINAL_PKG/"
        fi
    done
}
collect_binaries "linux"   ".so"
collect_binaries "linux64" ".so"
collect_binaries "mingw"   ".dll"
collect_binaries "mingw64" ".dll"
echo "========================================"
echo "=== Phase 4: Collecting pkg files ==="
echo "========================================"
# Copy package files from reference build (configs, mapscripts, jxac, etc.)
# This includes server.cfg, xmod.cfg, README.txt (already processed from m4)
echo "Copying package files from $REFERENCE_BUILD/$PROJECT_PACKAGE/..."
# Copy everything except binaries and pk3 (we create our own)
rsync -av --exclude='*.so' --exclude='*.dll' --exclude='*.pk3' \
    "$REFERENCE_BUILD/$PROJECT_PACKAGE/" "$FINAL_PKG/"
echo "========================================"
echo "=== Phase 5: Creating packages ==="
echo "========================================"
# Create pk3 file (zip of pak contents with directory structure)
echo "Creating $OUTPUT_PK3..."
(cd "$FINAL_PAK" && zip -qr - *) > "$OUTPUT_PK3"
echo "Created: $OUTPUT_PK3"
# Create tar.gz package
echo "Creating $OUTPUT_TAR..."
tar czf "$OUTPUT_TAR" -C "$FINAL_DIR" "$PROJECT_PACKAGE"
echo "Created: $OUTPUT_TAR"
echo ""
echo "========================================"
echo "=== Build complete! ==="
echo "========================================"
echo "Variant: $VARIANT"
echo ""
echo "Final packages:"
echo "  $OUTPUT_PK3"
echo "  $OUTPUT_TAR"
echo ""
echo "Package directory structure:"
find "$FINAL_PKG" -type f | sort
echo ""
echo "Pak directories (in pk3):"
find "$FINAL_PAK" -type d | sort
echo ""
echo "All binaries included:"
find "$FINAL_DIR" -type f \( -name "*.so" -o -name "*.dll" \) -exec ls -lh {} \;
echo ""
echo "PK3 contents verification:"
unzip -l "$OUTPUT_PK3" | head -60
