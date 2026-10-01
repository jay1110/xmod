#!/usr/bin/env python3
"""Stage public, loose server configuration files outside the client PK3."""

import argparse
import importlib.util
from pathlib import Path
import re
import shutil
import sys

from generate_server_configs import PRIVATE_CVARS, active_settings, generate, validate_templates

ROOT = Path(__file__).resolve().parents[1]


def version():
    text = (ROOT / "project/info.db").read_text(encoding="utf-8")
    return ".".join(re.search(rf"^version{part}\s*=\s*(\d+)", text, re.M)[1]
                    for part in ("Major", "Minor", "Point"))


def render_config(source):
    # These cfg templates use only project title substitution, no m4 logic.
    # Keep the normal GNU/m4 build and Python/CI packaging output equivalent.
    text = source.read_text(encoding="utf-8-sig")
    lines = [line for line in text.splitlines()
             if not line.startswith(("changequote(", "include(", "dnl"))]
    result = "\n".join(lines).replace("__title", "Xmod " + version()) + "\n"
    if re.search(r"__\w+|<<|>>|\bdnl\b", result):
        raise ValueError(f"Unsupported template directive in {source}; update renderer")
    return result


def private_template():
    result = render_config(ROOT / "pkg/server-private.cfg.m4")
    values = active_settings(result)
    if set(values) != PRIVATE_CVARS or any(values.values()):
        raise ValueError("Private release template must contain exactly seven blank credentials")
    return result


def check_private(destination):
    target = Path(destination) / "server-private.cfg"
    if target.is_symlink() or not target.is_file() or target.read_text(encoding="utf-8-sig") != private_template():
        raise ValueError("Existing server-private.cfg is not the blank release template. It was preserved; use a fresh release staging directory.")


def prepare_private(destination, strict=False):
    target = Path(destination) / "server-private.cfg"
    target.parent.mkdir(parents=True, exist_ok=True)
    if target.exists() or target.is_symlink():
        if strict:
            check_private(destination)
        return False
    # Exclusive creation also preserves a file created concurrently by the user.
    with target.open("x", encoding="utf-8", newline="\n") as output:
        output.write(private_template())
    return True


def stage(destination, strict_private=False):
    generated, _ = generate()
    if (ROOT / "pkg/xmod.cfg.m4").read_text(encoding="utf-8") != generated:
        raise ValueError("Stale cvar template; run build-tools/generate_server_configs.py")
    validate_templates()
    destination = Path(destination)
    destination.mkdir(parents=True, exist_ok=True)
    prepare_private(destination, strict=strict_private)
    files = []
    for name in ("server.cfg", "xmod.cfg"):
        target = destination / name
        target.write_text(render_config(ROOT / "pkg" / (name + ".m4")), encoding="utf-8", newline="\n")
        files.append(target)
    # server-private.cfg is deliberately absent from the returned file manifest.
    # Local ZIP creation writes a fresh blank template directly into the archive,
    # even if an existing private file was preserved in this output directory.
    # Explicit public-file allowlist: never copy a live server's keys, ACLs,
    # edited rules, caches, logs, databases or backup files into a release.
    inputs = [ROOT / "pkg/antirush/antirush.cfg"]
    inputs += sorted((ROOT / "pkg/antirush/maps").glob("*.cfg"))
    inputs += sorted((ROOT / "pkg/jxac").glob("*.cfg"))
    inputs += sorted((ROOT / "pkg/mapscripts").glob("*.script"))
    inputs += [ROOT / "pkg/commands.db.sample"]
    for source in inputs:
        target = destination / source.relative_to(ROOT / "pkg")
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        files.append(target)
    for source in (ROOT / "CHANGELOG.md", ROOT / "docs/ANTIRUSH.md",
                   ROOT / "docs/VPN_BLOCKER.md", ROOT / "docs/JXAC.md",
                   ROOT / "docs/SERVER_SETUP.md"):
        target = destination / source.name
        shutil.copy2(source, target)
        files.append(target)
    # Match the offline link in SERVER_SETUP.md using the same source-backed
    # generator as Sphinx, without requiring Sphinx or reading runtime data.
    spec = importlib.util.spec_from_file_location("xmod_release_reference", ROOT / "docs/_ext/xmod_reference.py")
    reference = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = reference
    spec.loader.exec_module(reference)
    target = destination / "_generated/registered-cvars.md"
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(reference.registered_reference(ROOT), encoding="utf-8", newline="\n")
    files.append(target)
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("destination", type=Path, help="Release xmod/ directory (not the PK3 staging directory)")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--release", action="store_true", help="Refuse existing private data before a directory-based ZIP/tar release")
    mode.add_argument("--private-only", action="store_true", help="Create the blank private template only if the file does not exist")
    mode.add_argument("--check-private", action="store_true", help="Require an unmodified blank private template; never writes files")
    args = parser.parse_args()
    if args.check_private:
        check_private(args.destination)
        print("Private release template verified: seven empty credentials")
    elif args.private_only:
        created = prepare_private(args.destination)
        print("Created blank private template" if created else "Preserved existing server-private.cfg")
    else:
        files = stage(args.destination, strict_private=args.release)
        print(f"Staged {len(files)} public server files outside the PK3; existing private credentials preserved in {args.destination}")


if __name__ == "__main__":
    main()
