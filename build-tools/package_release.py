#!/usr/bin/env python3
"""Package local native/WASM builds with client PK3 and loose server defaults."""

import argparse
import hashlib
from pathlib import Path
import re
import shutil
import zipfile

from stage_server_files import ROOT, private_template, stage, version

PLATFORMS = {
    "windows-x86": ("build/msvc-x86/{kind}/Release/{stem}_mp_x86.dll",),
    "windows-x64": ("build/msvc-x64/{kind}/Release/{stem}_mp_x64.dll",),
    "linux-x86": ("build.linux-release/{kind}/{stem}.mp.i386.so",),
    "linux-x64": ("build.linux64-release/{kind}/{stem}.mp.x86_64.so",),
    "wasm32": ("build.wasm-release/{kind}/{stem}.mp.wasm32.so",),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "release")
    parser.add_argument("--platforms", nargs="+", choices=PLATFORMS, default=list(PLATFORMS))
    parser.add_argument("--musl", type=Path, help="Optional directory with matching Alpine/musl modules")
    args = parser.parse_args()
    release_version = version()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    folder = output / ("xmod-" + release_version)
    folder.mkdir(exist_ok=True)

    modules = []
    for platform in args.platforms:
        for kind, stem in (("game", "qagame"), ("cgame", "cgame"), ("ui", "ui")):
            source = ROOT / PLATFORMS[platform][0].format(kind=kind, stem=stem)
            if not source.is_file() or release_version.encode() not in source.read_bytes():
                raise ValueError(f"Missing or mismatched version: {source}")
            modules.append(source)
    files = stage(folder)
    blank_private = private_template().encode("utf-8")
    for source in modules:
        target = folder / source.name
        shutil.copy2(source, target)
        files.append(target)

    pk3 = folder / ("xmod-" + release_version + ".pk3")
    with zipfile.ZipFile(pk3, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for source in sorted((ROOT / "pak").rglob("*")):
            if not source.is_file():
                continue
            relative = source.relative_to(ROOT / "pak")
            if relative.parts[0].lower() in ("antirush", "vpn", "jxac") or source.suffix.lower() in (".sqlite", ".db", ".log", ".cache", ".bak") or source.name.lower() in ("server.cfg", "server-private.cfg", "xmod.cfg"):
                raise ValueError(f"Server configuration/runtime data must not be packaged in a PK3: {relative}")
            if any(part.startswith(".") for part in relative.parts) or relative.as_posix() in ("pak.defs", "pak.rules"):
                continue
            archive.write(source, relative.as_posix())
        archive.writestr("xmod-" + release_version + ".dat", "")
        for source in modules:
            if not source.name.startswith("qagame"):
                archive.write(source, source.name)
    files.append(pk3)

    install = folder / "INSTALL.txt"
    install.write_text(
        f"Xmod {release_version}\n\n"
        "EN: Stop the server/game and back up your configuration and databases. Replace the old Xmod PK3 "
        "with the new PK3 and install the matching qagame module. Update clients and servers together. "
        "Do not keep the old Xmod PK3 alongside the new one. For a browser client, update the separately hosted WASM modules too.\n\n"
        "Copy the loose antirush/ and jxac/ folders beside qagame in your server's fs_game directory. "
        "Antirush and JXAC rules are server files and are deliberately absent from the PK3. "
        "The local MD5 denylist is jxac/jxac_md5.cfg; reload it with !jxac_reload. "
        "Updated Windows clients also run the bundled compound memory check when jxac_enable and jxac_moduleScan are enabled. "
        "Use !jxac_status for reported scan coverage; see JXAC.md for validation limits. "
        "For a new server, copy server.cfg, xmod.cfg and the blank server-private.cfg there too. For an existing server, merge settings "
        "without overwriting existing private config, edited map rules, GUID permissions, caches or databases. "
        "Launch with +set fs_game xmod +set dedicated 2 +exec server.cfg. "
        "server.cfg owns host/engine settings, xmod.cfg owns mod settings, and server-private.cfg owns passwords/API keys. "
        "Each setting is assigned once. The private file is loaded automatically after xmod.cfg and before map startup. "
        "The shipped private file is always blank; preserve your existing credentials during updates. "
        "See SERVER_SETUP.md, ANTIRUSH.md, VPN_BLOCKER.md and JXAC.md.\n\n"
        "DE: Server/Spiel stoppen und Konfiguration sowie Datenbanken sichern. Alte Xmod-PK3 ersetzen und "
        "passendes qagame-Modul installieren. Server und Clients gemeinsam aktualisieren. Alte PK3 nicht parallel behalten. "
        "Beim Browserclient separat bereitgestellte WASM-Module aktualisieren.\n\n"
        "Die losen Ordner antirush/ und jxac/ neben qagame in das fs_game-Verzeichnis des Servers kopieren. "
        "Die lokale MD5-Sperrliste liegt unter jxac/jxac_md5.cfg; Neuladen mit !jxac_reload. "
        "Aktualisierte Windows-Clients pruefen bei jxac_enable und jxac_moduleScan auch kombinierte Speichermerkmale. "
        "!jxac_status zeigt den gemeldeten Pruefumfang; Grenzen der Validierung stehen in JXAC.md. "
        "Die Regeln liegen absichtlich nicht in der PK3. Bei einem neuen Server auch server.cfg, xmod.cfg und die leere server-private.cfg kopieren. "
        "Bestehende Servereinstellungen abgleichen und eigene Regeln, GUID-Rechte, Caches und Datenbanken erhalten. "
        "Start mit +set fs_game xmod +set dedicated 2 +exec server.cfg. "
        "server.cfg enthaelt Host-/Engine-Werte, xmod.cfg die Mod-Einstellungen, server-private.cfg nur Passwoerter und API-Keys. "
        "Jede Einstellung steht nur in einer Datei. Die private Datei wird automatisch nach xmod.cfg und vor dem Mapstart geladen. "
        "Die mitgelieferte private Datei ist immer leer; vorhandene Zugangsdaten bei Updates erhalten. "
        "Siehe SERVER_SETUP.md, ANTIRUSH.md, VPN_BLOCKER.md und JXAC.md.\n\n"
        "Platforms / Plattformen: " + ", ".join(args.platforms) + ".\n"
        "Linux glibc modules require matching libcurl.so.4 and CA certificates. WASM server builds do not perform VPN lookups. "
        "Optional linux-musl-x64 modules are separate replacements for a matching musl engine. "
        "macOS and Linux ARM64 builds are supplied by GitHub Actions, not this local packager.\n",
        encoding="utf-8", newline="\n")
    files.append(install)
    checksums = folder / "SHA256SUMS.txt"
    checksums.write_text("".join(hashlib.sha256(p.read_bytes()).hexdigest() + "  " +
                                 p.relative_to(folder).as_posix() + "\n" for p in sorted(files)) +
                         hashlib.sha256(blank_private).hexdigest() + "  server-private.cfg\n",
                         encoding="utf-8", newline="\n")
    files.append(checksums)
    musl_files = []
    if args.musl:
        for source in sorted(args.musl.glob("*.so")):
            if not re.fullmatch(r"(?:qagame|cgame|ui)\.mp\.x86_64\.so", source.name):
                raise ValueError(f"Unexpected musl module name: {source}")
            if release_version.encode() not in source.read_bytes():
                raise ValueError(f"Mismatched musl module version: {source}")
            musl_files.append(source)
        if not musl_files:
            raise ValueError(f"No musl modules in {args.musl}")
    archive_path = output / ("xmod-" + release_version + ".zip")
    with zipfile.ZipFile(archive_path, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        # Manifest, not directory traversal: stale/private local output never enters the ZIP.
        for source in sorted(files):
            archive.write(source, "xmod/" + source.relative_to(folder).as_posix())
        archive.writestr("xmod/server-private.cfg", blank_private)
        for source in musl_files:
            archive.write(source, "linux-musl-x64/" + source.name)
        if musl_files:
            archive.writestr("linux-musl-x64/SHA256SUMS.txt", "".join(
                hashlib.sha256(source.read_bytes()).hexdigest() + "  " + source.name + "\n"
                for source in musl_files))
    with zipfile.ZipFile(pk3) as archive:
        assert archive.testzip() is None
        assert not any(name.startswith(("antirush/", "jxac/")) for name in archive.namelist())
        for source in modules:
            if not source.name.startswith("qagame"):
                assert archive.read(source.name) == source.read_bytes()
    with zipfile.ZipFile(archive_path) as archive:
        assert archive.testzip() is None
        assert "xmod/server.cfg" in archive.namelist() and "xmod/xmod.cfg" in archive.namelist()
        assert archive.read("xmod/server-private.cfg") == blank_private
        assert "xmod/antirush/antirush.cfg" in archive.namelist()
        assert archive.read("xmod/jxac/jxac_md5.cfg") == (ROOT / "pkg/jxac/jxac_md5.cfg").read_bytes()
        assert sum(name.startswith("xmod/antirush/maps/") for name in archive.namelist()) == 30
        assert not any(name.startswith("xmod/vpn/") or name.endswith((".sqlite", ".db", ".log", ".cache", ".bak")) for name in archive.namelist())
    print(f"Verified {archive_path}: {len(modules)} modules; server.cfg, xmod.cfg, blank server-private.cfg and 30 loose Antirush map presets")


if __name__ == "__main__":
    main()
