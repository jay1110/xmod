"""Prepare an Ubuntu 22.04 x86 sysroot for the Windows-hosted Clang build."""
import argparse
import gzip
import hashlib
import io
import json
import os
import pathlib
import shutil
import subprocess
import tarfile
import urllib.request

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--arch', choices=('amd64', 'i386'), default='amd64')
options = parser.parse_args()
directory = 'linux-cross' if options.arch == 'amd64' else 'linux-cross-i386'
root = pathlib.Path(__file__).resolve().parents[1] / 'build' / directory
sysroot = root / 'sysroot'
root.mkdir(parents=True, exist_ok=True)
base = 'https://archive.ubuntu.com/ubuntu/'
index = root / 'Packages.gz'
if not index.exists():
    urllib.request.urlretrieve(base + f'dists/jammy/main/binary-{options.arch}/Packages.gz', index)
packages = {}
for paragraph in gzip.decompress(index.read_bytes()).decode().split('\n\n'):
    fields = dict(line.split(': ', 1) for line in paragraph.splitlines() if ': ' in line and not line.startswith(' '))
    if 'Package' in fields:
        packages[fields['Package']] = fields
needed = ['libc6', 'libc6-dev', 'linux-libc-dev', 'libstdc++-11-dev', 'libstdc++6', 'libgcc-11-dev', 'libgcc-s1', 'gcc-11-base',
          'libcurl4-openssl-dev', 'libcurl4']
links = []
manifest = []
for name in needed:
    item = packages[name]
    archive = root / pathlib.Path(item['Filename']).name
    if not archive.exists():
        urllib.request.urlretrieve(base + item['Filename'], archive)
    data = archive.read_bytes()
    assert hashlib.sha256(data).hexdigest() == item['SHA256'], name
    assert data[:8] == b'!<arch>\n'
    pos = 8
    while pos < len(data):
        header = data[pos:pos + 60]
        size = int(header[48:58])
        payload = data[pos + 60:pos + 60 + size]
        pos += 60 + size + size % 2
        if not header[:16].decode().strip().startswith('data.tar'):
            continue
        if header[:16].decode().strip().startswith('data.tar.zst'):
            payload = subprocess.run([os.environ.get('XMOD_ZSTD', 'C:/msys64/usr/bin/zstd.exe'), '-d', '-c'], input=payload, capture_output=True, check=True).stdout
        with tarfile.open(fileobj=io.BytesIO(payload)) as tar:
            for member in tar:
                path = (sysroot / member.name).resolve()
                assert path.is_relative_to(sysroot.resolve())
                if member.isfile():
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_bytes(tar.extractfile(member).read())
                elif member.issym() or member.islnk():
                    target = (sysroot / member.linkname.lstrip('/')) if member.linkname.startswith('/') or member.islnk() else path.parent / member.linkname
                    target = target.resolve()
                    assert target.is_relative_to(sysroot.resolve())
                    links.append((path, target))
    manifest.append({key: item[key] for key in ['Package', 'Version', 'Filename', 'SHA256']})
    print(name, item['Version'], flush=True)
while links:
    remaining = []
    for path, target in links:
        if target.is_file():
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(target, path)
        elif target.is_dir():
            shutil.copytree(target, path, dirs_exist_ok=True)
        else:
            remaining.append((path, target))
    if len(remaining) == len(links):
        # Documentation/runtime utilities not needed by the compiler may be absent.
        print('Unresolved optional links:', len(remaining))
        break
    links = remaining
(root / 'packages.json').write_text(json.dumps(manifest, indent=2))
print('Sysroot:', sysroot)
