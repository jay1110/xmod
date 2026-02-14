# Multi-Platform Build Instructions

This document provides instructions for building xmod across multiple platforms including Linux ARM64, macOS (Universal Binaries), and Android.

## Table of Contents

1. [GitHub Actions Automated Builds](#github-actions-automated-builds)
2. [Local Build Instructions](#local-build-instructions)
   - [Linux ARM64 (aarch64)](#linux-arm64-aarch64)
   - [macOS Universal Binaries](#macos-universal-binaries)
   - [Android](#android)
3. [Downloading Build Artifacts](#downloading-build-artifacts)
4. [Platform-Specific Information](#platform-specific-information)

---

## GitHub Actions Automated Builds

The project uses GitHub Actions to automatically build binaries for multiple platforms. The workflow is defined in `.github/workflows/build-multiplatform.yml`.

### Supported Platforms

The workflow builds for the following platforms:

- **Linux ARM64 (aarch64)**: Native ARM64 Linux binaries (`.so` files)
- **macOS**: Universal binaries supporting both x86_64 (Intel) and arm64 (Apple Silicon) architectures
- **Android**: Native libraries for multiple ABIs:
  - `arm64-v8a` (64-bit ARM)
  - `armeabi-v7a` (32-bit ARM)
  - `x86` (32-bit Intel)
  - `x86_64` (64-bit Intel)

### Triggering Builds

The workflow automatically runs on:
- Push to `main` or `develop` branches
- Pull requests targeting `main` or `develop`
- Manual workflow dispatch from GitHub Actions tab

### Build Matrix

Each platform builds three modules:
- `cgame` - Client-side game module
- `ui` - User interface module
- `qagame` - Server-side game module

---

## Downloading Build Artifacts

### From GitHub Actions

1. Go to the [Actions tab](../../actions) in the repository
2. Click on a completed workflow run
3. Scroll down to the **Artifacts** section
4. Download the artifacts you need:
   - `xmod-linux-arm64-cgame`, `xmod-linux-arm64-ui`, `xmod-linux-arm64-game`
   - `xmod-macos-cgame`, `xmod-macos-ui`, `xmod-macos-game`
   - `xmod-android-arm64-v8a-cgame`, etc. (for each ABI and module)
   - `xmod-multiplatform-release` (combined package with all platforms)

### Release Packages

When code is pushed to `main` or `develop`, a combined release package (`xmod-multiplatform-release`) is created containing all platforms in an organized structure:

```
xmod-multiplatform/
├── linux-arm64/
│   ├── cgame.mp.aarch64.so
│   ├── ui.mp.aarch64.so
│   └── qagame.mp.aarch64.so
├── macos/
│   ├── cgame.mp.x86_64.dylib
│   ├── ui.mp.x86_64.dylib
│   └── qagame.mp.x86_64.dylib
└── android/
    ├── arm64-v8a/
    │   ├── libcgame.mp.android.arm64-v8a.so
    │   ├── libui.mp.android.arm64-v8a.so
    │   └── libqagame.mp.android.arm64-v8a.so
    ├── armeabi-v7a/
    │   ├── libcgame.mp.android.armeabi-v7a.so
    │   ├── libui.mp.android.armeabi-v7a.so
    │   └── libqagame.mp.android.armeabi-v7a.so
    ├── x86/
    │   └── ...
    └── x86_64/
        └── ...
```

---

## Local Build Instructions

### Prerequisites

All local builds require:
- Python 3
- GNU Make
- zip utility
- m4 macro processor

### Linux ARM64 (aarch64)

#### Cross-Compilation on x86_64 Linux

Install the ARM64 cross-compiler:

```bash
# Debian/Ubuntu
sudo apt-get update
sudo apt-get install -y build-essential gcc-aarch64-linux-gnu g++-aarch64-linux-gnu python3 zip m4

# Fedora/RHEL
sudo dnf install -y gcc-aarch64-linux-gnu gcc-c++-aarch64-linux-gnu python3 zip m4

# Arch Linux
sudo pacman -S aarch64-linux-gnu-gcc python zip m4
```

Build the modules:

```bash
# Cross-compile all modules (when building on x86_64 for ARM64)
PLATFORM=linux-aarch64 CROSS_COMPILE=1 make release

# Output files will be in:
# build.linux-aarch64-release/cgame/cgame.mp.aarch64.so
# build.linux-aarch64-release/ui/ui.mp.aarch64.so
# build.linux-aarch64-release/game/qagame.mp.aarch64.so
```

#### Native Build on ARM64 Linux

If you're building on a native ARM64 Linux system (e.g., Raspberry Pi 64-bit, AWS Graviton):

```bash
# Install build dependencies
sudo apt-get install -y build-essential python3 zip m4

# Build
PLATFORM=linux-aarch64 make release
```

---

### macOS Universal Binaries

#### Prerequisites (macOS)

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install python3 zip
```

#### Building Universal Binaries

The xmod build system can create universal binaries that run on both Intel and Apple Silicon Macs.

**Option 1: Build for x86_64 (Intel Macs)**

```bash
PLATFORM=osx64 make release

# Output: build.osx64-release/{cgame,ui,game}/*.dylib
```

**Option 2: Create Universal Binaries Manually**

To create a universal binary, you need to build for both architectures and combine them:

```bash
# Build for x86_64
PLATFORM=osx64 make release
mkdir -p artifacts/x86_64
cp build.osx64-release/cgame/*.dylib artifacts/x86_64/ 2>/dev/null || true
cp build.osx64-release/ui/*.dylib artifacts/x86_64/ 2>/dev/null || true
cp build.osx64-release/game/*.dylib artifacts/x86_64/ 2>/dev/null || true

# Clean and build for arm64
make clean
PLATFORM=osx-arm64 make release
mkdir -p artifacts/arm64
cp build.osx-arm64-release/cgame/*.dylib artifacts/arm64/ 2>/dev/null || true
cp build.osx-arm64-release/ui/*.dylib artifacts/arm64/ 2>/dev/null || true
cp build.osx-arm64-release/game/*.dylib artifacts/arm64/ 2>/dev/null || true

# Create universal binary using lipo
lipo -create \
  artifacts/x86_64/cgame.mp.x86_64.dylib \
  artifacts/arm64/cgame.mp.arm64.dylib \
  -output cgame.mp.universal.dylib

# Verify the universal binary
lipo -info cgame.mp.universal.dylib
# Output should show: Architectures in the fat file: x86_64 arm64
```

**Note**: For local development on macOS, the GitHub Actions workflow is recommended for creating universal binaries as it handles both architectures automatically.

---

### Android

#### Prerequisites

Install the Android NDK (r26c recommended):

**Option 1: Using Android Studio**
1. Open Android Studio
2. Go to Tools → SDK Manager
3. Select "SDK Tools" tab
4. Check "NDK (Side by side)"
5. Install version r26c

**Option 2: Command Line**

```bash
# Download NDK r26c
wget https://dl.google.com/android/repository/android-ndk-r26c-linux.zip

# Extract
unzip android-ndk-r26c-linux.zip -d ~/android-ndk

# Set environment variable
export ANDROID_NDK_HOME=~/android-ndk/android-ndk-r26c
```

**Install build dependencies:**

```bash
# Ubuntu/Debian
sudo apt-get install -y build-essential python3 zip m4

# macOS
brew install python3 zip
```

#### Building for Android

The build system supports all major Android ABIs:

**ARM64-v8a (64-bit ARM)**

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk-r26c
PLATFORM=android-arm64 make release

# Output: build.android-arm64-release/{cgame,ui,game}/libcgame.mp.android.arm64-v8a.so
```

**ARMv7a (32-bit ARM)**

```bash
PLATFORM=android-armv7a make release

# Output: build.android-armv7a-release/{cgame,ui,game}/libcgame.mp.android.armeabi-v7a.so
```

**x86 (32-bit Intel)**

```bash
PLATFORM=android-x86 make release

# Output: build.android-x86-release/{cgame,ui,game}/libcgame.mp.android.i386.so
```

**x86_64 (64-bit Intel)**

```bash
PLATFORM=android-x86_64 make release

# Output: build.android-x86_64-release/{cgame,ui,game}/libcgame.mp.android.x86_64.so
```

#### Building All Android ABIs

```bash
#!/bin/bash
export ANDROID_NDK_HOME=/path/to/android-ndk-r26c

for platform in android-arm64 android-armv7a android-x86 android-x86_64; do
  echo "Building for $platform..."
  PLATFORM=$platform make release
  
  # Organize outputs
  mkdir -p release/android
  cp -r build.$platform-release release/android/$platform
done

echo "All Android builds complete!"
ls -R release/android/
```

---

## Platform-Specific Information

### Compiler Toolchains

| Platform | Compiler | ABI/Arch | Toolchain |
|----------|----------|----------|-----------|
| Linux ARM64 | GCC | aarch64 | gcc-aarch64-linux-gnu |
| macOS | Clang | x86_64, arm64 | Apple Clang |
| Android ARM64 | Clang | arm64-v8a | aarch64-linux-android21-clang++ |
| Android ARMv7a | Clang | armeabi-v7a | armv7a-linux-androideabi21-clang++ |
| Android x86 | Clang | x86 | i686-linux-android21-clang++ |
| Android x86_64 | Clang | x86_64 | x86_64-linux-android21-clang++ |

### Platform Defines

The build system automatically sets platform-specific preprocessor defines:

| Platform | Define |
|----------|--------|
| Linux ARM64 | `XMOD_LINUX_AARCH64` |
| macOS x86_64 | `XMOD_OSX64` |
| macOS | `XMOD_OSX` |
| Android ARM64 | `XMOD_ANDROID_ARM64`, `ANDROID`, `__ANDROID__` |
| Android ARMv7a | `XMOD_ANDROID_ARMV7A`, `ANDROID`, `__ANDROID__` |
| Android x86 | `XMOD_ANDROID_X86`, `ANDROID`, `__ANDROID__` |
| Android x86_64 | `XMOD_ANDROID_X86_64`, `ANDROID`, `__ANDROID__` |

These defines are used in `src/base/config.h` for platform-specific code paths.

### Library Extensions

| Platform | Extension |
|----------|-----------|
| Linux | `.so` |
| macOS | `.dylib` or `_mac` |
| Android | `.so` |

### Minimum Requirements

| Platform | Minimum Version |
|----------|----------------|
| Linux ARM64 | Any modern ARM64 Linux distribution |
| macOS | macOS 10.9+ (for x86_64), macOS 11.0+ (for arm64) |
| Android | API Level 21 (Android 5.0 Lollipop) |

---

## Troubleshooting

### NDK Not Found

```
Error: NDK_ROOT is not set
```

**Solution**: Set the `ANDROID_NDK_HOME` or `NDK_ROOT` environment variable:

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk-r26c
```

### Cross-Compiler Not Found

```
Error: aarch64-linux-gnu-g++: command not found
```

**Solution**: Install the ARM64 cross-compiler:

```bash
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

### macOS Lipo Errors

```
Error: can't figure out the architecture type of: ...
```

**Solution**: Ensure both architecture files exist and are valid Mach-O binaries. Check with:

```bash
file your_binary.dylib
```

### Build Fails on Android

If the Android build fails with linker errors, ensure:
1. NDK version is r26c or compatible
2. `ANDROID_NDK_HOME` is correctly set
3. All build dependencies are installed

---

## Related Documentation

- [BUILD.md](BUILD.md) - General build instructions for Windows and Linux
- [README.md](README.md) - Project overview
- [GitHub Actions Workflow](.github/workflows/build-multiplatform.yml) - Automated build configuration

---

## Support

For build issues:
1. Check the [Actions tab](../../actions) for automated build logs
2. Review this documentation
3. Open an issue on GitHub with:
   - Your OS and version
   - Compiler/NDK version
   - Complete error output
   - Steps to reproduce
