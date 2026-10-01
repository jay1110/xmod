"""Windows-hosted Clang driver targeting GNU/Linux x86 or x86-64 with libstdc++."""
import os
import pathlib
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[1]
sdk = pathlib.Path(os.environ.get('XMOD_LLVM_BIN', str(pathlib.Path.home() / 'emsdk/upstream/bin')))
architecture = os.environ.get('XMOD_LINUX_ARCH', 'amd64')
targets = {'amd64': ('linux-cross', 'x86_64-linux-gnu'),
           'i386': ('linux-cross-i386', 'i686-linux-gnu')}
if architecture not in targets:
    sys.exit('Unsupported XMOD_LINUX_ARCH: ' + architecture)
directory, target = targets[architecture]
crossroot = root / 'build' / directory
sysroot = crossroot / 'sysroot'
args = [str(sdk / 'clang++.exe'), '--driver-mode=g++', '--target=' + target,
        '--sysroot=' + sysroot.as_posix(),
        '--gcc-install-dir=' + (sysroot / 'usr/lib/gcc' / target / '11').as_posix(),
        '--ld-path=' + (crossroot / 'ld.lld.exe').as_posix()]
args += sys.argv[1:]
sys.exit(subprocess.call(args))
