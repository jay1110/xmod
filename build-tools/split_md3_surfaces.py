"""Split oversized MD3 surfaces for the original ET renderer, preserving triangles."""
import argparse
import pathlib
import struct

HEADER = struct.Struct('<4si64s9i')
SURFACE = struct.Struct('<4s64s10i')

def convert(path, write=False):
    data = path.read_bytes()
    header = list(HEADER.unpack_from(data))
    assert header[0] == b'IDP3' and header[1] == 15
    offset = header[10]
    surfaces = []
    changed = False
    for _ in range(header[6]):
        s = list(SURFACE.unpack_from(data, offset))
        _, name, flags, frames, shaders, vertices, triangles, ot, os, ost, ov, end = s
        print(f'{path.name}: {name.split(bytes([0]))[0]!r}: {vertices} vertices, {triangles} triangles')
        if vertices <= 1024 and triangles <= 2000:
            surfaces.append(data[offset:offset + end])
        else:
            changed = True
            groups, group, used = [], [], set()
            for i in range(triangles):
                tri = struct.unpack_from('<3i', data, offset + ot + i * 12)
                assert all(0 <= v < vertices for v in tri)
                if len(used.union(tri)) > 1024 or len(group) >= 2000:
                    groups.append(group)
                    group, used = [], set()
                group.append(tri)
                used.update(tri)
            if group:
                groups.append(group)
            for index, group in enumerate(groups):
                ids = sorted({v for tri in group for v in tri})
                remap = {v: i for i, v in enumerate(ids)}
                tris = b''.join(struct.pack('<3i', *(remap[v] for v in tri)) for tri in group)
                shader_data = data[offset + os:offset + os + shaders * 68]
                uv = b''.join(data[offset + ost + v * 8:offset + ost + (v + 1) * 8] for v in ids)
                points = b''.join(data[offset + ov + (f * vertices + v) * 8:offset + ov + (f * vertices + v + 1) * 8] for f in range(frames) for v in ids)
                outname = name.split(bytes([0]))[0][:52] + f'_part{index}'.encode()
                sh = SURFACE.pack(b'IDP3', outname, flags, frames, shaders, len(ids), len(group), 108, 108 + len(tris), 108 + len(tris) + len(shader_data), 108 + len(tris) + len(shader_data) + len(uv), 108 + len(tris) + len(shader_data) + len(uv) + len(points))
                surfaces.append(sh + tris + shader_data + uv + points)
        offset += end
    if changed and write:
        header[6] = len(surfaces)
        header[11] = header[10] + sum(map(len, surfaces))
        result = HEADER.pack(*header) + data[HEADER.size:header[10]] + b''.join(surfaces)
        assert len(result) == header[11]
        path.write_bytes(result)
        print('Updated', path)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--write', action='store_true')
    parser.add_argument('files', nargs='+', type=pathlib.Path)
    args = parser.parse_args()
    for path in args.files:
        convert(path, args.write)
