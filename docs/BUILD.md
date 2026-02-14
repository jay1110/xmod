# Building xmod

This document provides comprehensive instructions for building xmod across all supported platforms and architectures.

## Prerequisites

### Windows

#### Option 1: Visual Studio (Recommended for Windows)
- **Visual Studio 2022 Community** (or newer)
  - Workload: "Desktop development with C++"
  - Individual components:
    - MSVC v143 C++ x64/x86 build tools
    - Windows 11 SDK (or Windows 10 SDK)

#### Option 2: MSYS2 (For cross-compilation)
- Download and install from https://www.msys2.org/
- Install required packages:
  ```bash
  pacman -S base-devel mingw-w64-i686-toolchain mingw-w64-x86_64-toolchain
  ```

### Linux

```bash
# Debian/Ubuntu
sudo apt install build-essential gcc-multilib g++-multilib make

# Fedora/RHEL
sudo dnf install gcc gcc-c++ glibc-devel.i686 libstdc++-devel.i686 make

# Arch
sudo pacman -S base-devel multilib-devel
```

## Building with Visual Studio

Visual Studio projects support both 32-bit (Win32) and 64-bit (x64) builds.

### Steps:

1. Open `src/xmod.sln` in Visual Studio
2. Select your desired configuration and platform:
   - **Configuration**: Debug or Release
   - **Platform**: Win32 (32-bit) or x64 (64-bit)
3. Build → Build Solution (F7)

### Available Configurations:

| Configuration | Platform | Output Directory | Output Files |
|---------------|----------|------------------|--------------|
| Release | Win32 | `src/Release/` | `cgame_mp_x86.dll`, `qagame_mp_x86.dll`, `ui_mp_x86.dll` |
| Release | x64 | `src/Release/` | `cgame_mp_x86_64.dll`, `qagame_mp_x86_64.dll`, `ui_mp_x86_64.dll` |
| Debug | Win32 | `src/Debug/` | `cgame_mp_x86.dll`, `qagame_mp_x86.dll`, `ui_mp_x86.dll` |
| Debug | x64 | `src/Debug/` | `cgame_mp_x86_64.dll`, `qagame_mp_x86_64.dll`, `ui_mp_x86_64.dll` |

### Building from Command Line (MSBuild):

```batch
REM Build Win32 Release
msbuild src\xmod.sln /p:Configuration=Release /p:Platform=Win32

REM Build x64 Release
msbuild src\xmod.sln /p:Configuration=Release /p:Platform=x64

REM Build Win32 Debug
msbuild src\xmod.sln /p:Configuration=Debug /p:Platform=Win32

REM Build x64 Debug
msbuild src\xmod.sln /p:Configuration=Debug /p:Platform=x64
```

## Building with Make (MSYS2/Linux)

The Makefile-based build system supports cross-platform compilation.

### Windows (MSYS2):

#### 32-bit Windows Build:
```bash
make PLATFORM=mingw32 ARCH=x86 -j$(nproc)
```

#### 64-bit Windows Build:
```bash
make PLATFORM=mingw32 ARCH=x86_64 -j$(nproc)
```

### Linux:

#### 32-bit Linux Build:
```bash
make PLATFORM=linux ARCH=x86 -j$(nproc)
```

#### 64-bit Linux Build:
```bash
make PLATFORM=linux ARCH=x86_64 -j$(nproc)
```

### Clean Build:
```bash
make clean
```

## Building All Platforms

### Linux/MSYS2:
Use the provided shell script to build all platforms:
```bash
chmod +x build-all.sh
./build-all.sh
```

### Windows (CMD/PowerShell):
Use the provided batch script:
```batch
build-all.bat
```

These scripts will build:
- Windows 32-bit
- Windows 64-bit
- Linux 32-bit
- Linux 64-bit

Output will be organized in the `release/` directory:
```
release/
├── windows-32bit/
│   ├── cgame_mp_x86.dll
│   ├── qagame_mp_x86.dll
│   └── ui_mp_x86.dll
├── windows-64bit/
│   ├── cgame_mp_x86_64.dll
│   ├── qagame_mp_x86_64.dll
│   └── ui_mp_x86_64.dll
├── linux-32bit/
│   ├── cgame.mp.i386.so
│   ├── qagame.mp.i386.so
│   └── ui.mp.i386.so
└── linux-64bit/
    ├── cgame.mp.x86_64.so
    ├── qagame.mp.x86_64.so
    └── ui.mp.x86_64.so
```

## Output Files Summary

| Platform | Module | 32-bit Output | 64-bit Output |
|----------|--------|---------------|---------------|
| **Windows** | cgame (Client) | `cgame_mp_x86.dll` | `cgame_mp_x86_64.dll` |
| | qagame (Server) | `qagame_mp_x86.dll` | `qagame_mp_x86_64.dll` |
| | ui (User Interface) | `ui_mp_x86.dll` | `ui_mp_x86_64.dll` |
| **Linux** | cgame (Client) | `cgame.mp.i386.so` | `cgame.mp.x86_64.so` |
| | qagame (Server) | `qagame.mp.i386.so` | `qagame.mp.x86_64.so` |
| | ui (User Interface) | `ui.mp.i386.so` | `ui.mp.x86_64.so` |

## Platform-Specific Defines

The build system automatically sets platform-specific preprocessor defines:

| Platform | Architecture | Define |
|----------|--------------|--------|
| Windows | 32-bit | `XMOD_WINDOWS` |
| Windows | 64-bit | `XMOD_WINDOWS64` |
| Linux | 32-bit | `XMOD_LINUX` |
| Linux | 64-bit | `XMOD_LINUX64` |

These defines are used in `src/base/config.h` for platform detection.

## Troubleshooting

### Visual Studio Build Issues

**Problem**: "Platform 'x64' not found"
- **Solution**: Make sure you have the x64 build tools installed. Open Visual Studio Installer and verify "MSVC x64/x86 build tools" is installed.

**Problem**: Missing Windows SDK
- **Solution**: Install Windows 10 or 11 SDK through Visual Studio Installer.

**Problem**: LNK1112: module machine type 'x64' conflicts with target machine type 'x86'
- **Solution**: Clean the solution and rebuild. Make sure the selected platform matches your configuration.

### Make Build Issues

**Problem**: `gcc: error: unrecognized command line option '-m32'`
- **Solution**: Install multilib support:
  ```bash
  # Ubuntu/Debian
  sudo apt install gcc-multilib g++-multilib
  ```

**Problem**: Missing dependencies
- **Solution**: Ensure all build tools are installed as listed in Prerequisites.

## Testing Your Build

After building, you can test the modules:

1. Copy the appropriate DLL/SO files to your Enemy Territory installation:
   ```
   <ET-Install>/xmod/
   ```

2. Start the game/server with:
   ```
   +set fs_game xmod
   ```

3. The console should show the xmod version on startup.

## Advanced Build Options

### Custom Compiler Flags

You can pass custom flags through environment variables:

```bash
# Linux example
CFLAGS="-march=native" make PLATFORM=linux ARCH=x86_64

# Add debug symbols
DEBUG=1 make PLATFORM=linux ARCH=x86_64
```

### Parallel Builds

Use the `-j` flag to speed up compilation:

```bash
# Use all available CPU cores
make PLATFORM=linux ARCH=x86_64 -j$(nproc)

# Use specific number of cores
make PLATFORM=linux ARCH=x86_64 -j4
```

## Related Documentation

- [README.md](README.md) - General project information
- [notes/BuildSystem.txt](notes/BuildSystem.txt) - Detailed build system documentation
- [DATABASE_MIGRATION.md](DATABASE_MIGRATION.md) - Database migration guide

## Support

If you encounter build issues not covered here:

1. Check existing issues on GitHub
2. Open a new issue with:
   - Your OS and version
   - Compiler version
   - Complete error output
   - Steps to reproduce
