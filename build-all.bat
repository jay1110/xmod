@echo off
echo === xmod Multi-Platform Build Script ===
echo.

mkdir release 2>nul

REM Windows 32-bit
echo Building Windows 32-bit...
make clean
make PLATFORM=mingw32 ARCH=x86 -j4
mkdir release\windows-32bit 2>nul
xcopy /Y build\*.dll release\windows-32bit\ 2>nul

REM Windows 64-bit
echo Building Windows 64-bit...
make clean
make PLATFORM=mingw32 ARCH=x86_64 -j4
mkdir release\windows-64bit 2>nul
xcopy /Y build\*.dll release\windows-64bit\ 2>nul

REM Linux 32-bit
echo Building Linux 32-bit...
make clean
make PLATFORM=linux ARCH=x86 -j4
mkdir release\linux-32bit 2>nul
xcopy /Y build\*.so release\linux-32bit\ 2>nul

REM Linux 64-bit
echo Building Linux 64-bit...
make clean
make PLATFORM=linux ARCH=x86_64 -j4
mkdir release\linux-64bit 2>nul
xcopy /Y build\*.so release\linux-64bit\ 2>nul

echo.
echo === Build complete! ===
dir /s release\
pause
