# Current build entry points

Use these entry points for Xmod 2.0.4. The longer platform guides in the sidebar
also describe older toolchains and remain available as background material.

## Windows with Omni-bot support

Use Visual Studio 2022's MSVC toolchain through CMake. The Windows Omni-bot DLL
uses the MSVC C++ ABI, so the supported native Windows build is MSVC.

```powershell
cmake -S . -B build/msvc-x86 -A Win32
cmake --build build/msvc-x86 --config Release --parallel 4
cmake -S . -B build/msvc-x64 -A x64
cmake --build build/msvc-x64 --config Release --parallel 4
```

The module directories are `game/Release`, `cgame/Release` and `ui/Release`
inside the selected build directory. Match the module architecture to the
engine executable. Optional local regression tests run with:

```powershell
ctest --test-dir build/msvc-x64 -C Release --output-on-failure
```

## Linux builds directly on Windows

The local cross-build scripts use Clang and the configured Linux sysroot; WSL
is not needed:

```powershell
./build-tools/build-linux32-local.ps1
./build-tools/build-linux64-local.ps1
```

Consult the [VPN platform guide](VPN_BLOCKER.md) for native runtime and libcurl
requirements. A glibc module and a musl/Alpine module require matching engines
and cannot be interchanged.

## GitHub Actions

The [repository workflows](https://github.com/jay1110/xmod/actions) build release
artifacts and additional native Unix targets. Keep configuration templates
current with:

```sh
python build-tools/generate_server_configs.py --check
```

See [building the documentation](README.md) for the independent Sphinx site.
