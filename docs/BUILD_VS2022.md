# Building xmod with Visual Studio 2022 and WSL2

This document describes how to build all platform variants of xmod (Windows 32-bit, Windows 64-bit, Linux 32-bit, Linux 64-bit) from a Windows development environment using Visual Studio 2022 and WSL2, and create a complete release package identical to the GitHub workflow output.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Building Individual Components](#building-individual-components)
- [Release Package Structure](#release-package-structure)
- [Troubleshooting](#troubleshooting)
- [Comparison with GitHub Workflow](#comparison-with-github-workflow)

## Prerequisites

### Required Software

#### Visual Studio 2022

1. **Download and Install Visual Studio 2022 Community** (free) or higher
   - Download from: https://visualstudio.microsoft.com/downloads/

2. **Required Workload:**
   - ✅ Desktop development with C++

3. **Required Individual Components:**
   - ✅ MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
   - ✅ Windows 10 SDK or Windows 11 SDK (10.0.19041.0 or newer)

**Installation Size:** Approximately 6-8 GB

#### WSL2 (Windows Subsystem for Linux)

WSL2 is required to build the Linux variants. If you only need Windows builds, you can skip WSL2 installation.

1. **Check if WSL is already installed:**
   ```powershell
   wsl --version
   ```

2. **Install WSL2 (if not installed):**
   ```powershell
   # Run PowerShell as Administrator
   wsl --install
   ```

3. **Restart your computer** after installation

4. **Set up Ubuntu:**
   - Launch Ubuntu from Start Menu
   - Create a username and password when prompted

5. **Verify WSL2 installation:**
   ```powershell
   wsl --list --verbose
   ```

**Installation Size:** Approximately 2-4 GB

## Quick Start

### Building Everything (All Platforms + Release Package)

1. **Open PowerShell** in the repository root directory

2. **Run the master build script:**
   ```powershell
   .\build-all.ps1
   ```

3. **Wait for completion** (typically 5-15 minutes)

4. **Find your release package:**
   ```
   release/xmod-2.0.0.zip
   ```

### Building Without Linux Support

If WSL2 is not installed:

```powershell
.\build-all.ps1 -SkipLinux
```

## Building Individual Components

### Windows Builds (Visual Studio)

#### Option 1: Using Visual Studio IDE

1. Open `xmod.sln`
2. Select Configuration: `Release` and Platform: `Win32` or `x64`
3. Build → Build Solution (Ctrl+Shift+B)

#### Option 2: Using Command Line (MSBuild)

```powershell
# Build Win32 (32-bit)
msbuild xmod.sln /p:Configuration=Release /p:Platform=Win32 /m

# Build x64 (64-bit)
msbuild xmod.sln /p:Configuration=Release /p:Platform=x64 /m
```

### Linux Builds (WSL2)

```powershell
wsl bash build-tools/build-linux.sh
```

## Release Package Structure

The `xmod-2.0.0.zip` package contains:

```
xmod/
├── xmod-2.0.0.pk3          # Client binaries + pak data (all platforms)
├── qagame_mp_x86.dll       # Windows 32-bit server
├── qagame_mp_x64.dll       # Windows 64-bit server
├── qagame.mp.i386.so       # Linux 32-bit server
├── qagame.mp.x86_64.so     # Linux 64-bit server
├── mapscripts/             # Map scripts
├── linux/                  # Linux server scripts
├── README.txt
├── server.cfg
└── xmod.cfg
```

## Troubleshooting

### "Platform 'x64' not found"

**Solution:** Install MSVC x64 build tools via Visual Studio Installer

### "WSL is not installed"

**Solution:**
```powershell
wsl --install
# Restart computer
```

### "Execution of scripts is disabled"

**Solution:**
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Linux build fails with "gcc: command not found"

**Solution:** The build script auto-installs dependencies. If it fails:
```bash
# In WSL
sudo apt-get update
sudo apt-get install -y build-essential gcc-multilib g++-multilib
```

## Comparison with GitHub Workflow

The local build produces identical output to GitHub Actions, with these differences:

| Aspect | GitHub Workflow | Local Build |
|--------|----------------|-------------|
| Windows Compiler | MinGW | MSVC |
| Linux Compiler | GCC (Ubuntu) | GCC (WSL2) |
| Binaries | Functionally identical | Functionally identical |

## Support

For issues:
1. Check this documentation
2. Review GitHub issues: https://github.com/jay1110/xmod/issues
3. Create a new issue with error details

## See Also

- [BUILD.md](../BUILD.md) - Makefile-based build documentation
- [README.md](../README.md) - Project overview
