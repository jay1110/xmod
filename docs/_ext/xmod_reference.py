"""Publish the existing DocBook reference without requiring its legacy toolchain.

Only repository-owned entity fragments are expanded. External DTDs, network
access and the server's private configuration/database are never consulted.
"""
from __future__ import annotations

import importlib.util
import os
from pathlib import Path
import re
import shutil
import sys
import xml.etree.ElementTree as ET


def compact(text: str) -> str:
    return " ".join(text.split())


def escape(text: str) -> str:
    return re.sub(r"([\\`*\[\]_])", r"\\\1", compact(text))


def code(text: str) -> str:
    return "`` " + compact(text).replace("`", "'") + " ``"


def write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text(encoding="utf-8") != content:
        path.write_text(content, encoding="utf-8", newline="\n")


def entities(root: Path, release: str) -> dict[str, str]:
    values = {}
    for source in sorted((root / "doc/glossary").glob("entity.*.xml")):
        text = source.read_text(encoding="utf-8")
        for match in re.finditer(r'<!ENTITY\s+([\w:.-]+)\s+([\'"])(.*?)\2\s*>', text, re.S):
            values[match[1]] = match[3]
    values.update({"project:name": "Xmod", "project:pk3": f"xmod-{release}.pk3"})
    for name in ("boolean.mode", "syntax.duration"):
        values[name] = (root / "doc/book" / f"{name}.xml").read_text(encoding="utf-8")
    for source in (root / "doc/feature").glob("*.xml"):
        values[f"feature.{source.stem}"] = source.read_text(encoding="utf-8")
    return values


def expand(text: str, values: dict[str, str]) -> str:
    def replace(match: re.Match) -> str:
        name = match[1]
        if name in {"amp", "lt", "gt", "quot", "apos"} or name.startswith("#"):
            return match[0]
        if name not in values:
            raise ValueError(f"Unknown DocBook entity: {name}")
        return values[name]

    if "<!DOCTYPE" in text or "<!ENTITY" in text:
        raise ValueError("External declarations are not allowed in reference entries")
    for _ in range(12):
        result = re.sub(r"&([^;\s]+);", replace, text)
        if result == text:
            return result
        text = result
    raise ValueError("Recursive DocBook entity expansion")


class Renderer:
    def __init__(self, page: Path, references: dict[str, Path], generated: Path):
        self.page = page
        self.references = references
        self.generated = generated

    def inline(self, node: ET.Element) -> str:
        tag = node.tag
        if tag == "xref":
            target = node.get("linkend", "")
            label = target.removeprefix("cvar.").removeprefix("cmd.")
            if target in self.references:
                destination = os.path.relpath(self.references[target], self.page.parent).replace("\\", "/")
                return f"[{escape(label)}]({destination})"
            return code(label)
        if tag in {"command", "literal", "filename", "varname", "replaceable"}:
            return code("".join(node.itertext()))
        if tag == "emphasis":
            return "**" + self.mixed(node) + "**"
        return self.mixed(node)

    def mixed(self, node: ET.Element) -> str:
        # Preserve separators around inline markup while collapsing XML indentation.
        parts = [node.text or ""]
        for child in node:
            parts.append(self.inline(child))
            parts.append(child.tail or "")
        return compact("".join(parts))

    def synopsis(self, node: ET.Element) -> str:
        parts = []
        for child in node:
            value = compact("".join(child.itertext()))
            if child.tag == "arg" and child.get("choice", "opt") == "opt" and not value.startswith("["):
                value = f"[{value}]"
            parts.append(value)
        return "```text\n" + " ".join(parts) + "\n```\n\n"

    def table(self, node: ET.Element) -> str:
        title = node.findtext("title")
        group = node.find("tgroup")
        if group is None:
            return self.children(node)
        rows = group.findall("thead/row") + group.findall("tbody/row")
        if not rows:
            return ""
        rendered = []
        for row in rows:
            cells = []
            for entry in row.findall("entry"):
                value = self.mixed(entry).replace("|", "\\|")
                cells.append(value)
            rendered.append(cells)
        width = max(map(len, rendered))
        rendered = [row + [""] * (width - len(row)) for row in rendered]
        if not group.findall("thead/row"):
            rendered.insert(0, [""] * width)
        lines = ["| " + " | ".join(row) + " |" for row in rendered]
        lines.insert(1, "| " + " | ".join(["---"] * width) + " |")
        return (f"**{escape(title)}**\n\n" if title else "") + "\n".join(lines) + "\n\n"

    def children(self, node: ET.Element, depth: int = 2) -> str:
        result = escape(node.text or "")
        if result:
            result += "\n\n"
        for child in node:
            result += self.block(child, depth)
            tail = escape(child.tail or "")
            if tail and tail != "•":
                result += tail + "\n\n"
        return result

    def block(self, node: ET.Element, depth: int = 2) -> str:
        tag = node.tag
        if tag in {"refmeta", "refnamediv", "colspec", "title"}:
            return ""
        if tag in {"refsection", "section"}:
            title = node.findtext("title", "Details")
            return "#" * depth + " " + escape(title) + "\n\n" + self.children(node, depth + 1)
        if tag == "cmdsynopsis":
            return self.synopsis(node)
        if tag in {"table", "informaltable"}:
            return self.table(node)
        if tag == "para":
            if any(child.tag in {"itemizedlist", "note", "warning", "tip", "important", "caution", "table", "informaltable"} for child in node):
                return self.children(node, depth)
            return self.mixed(node) + "\n\n"
        if tag in {"note", "warning", "tip", "important", "caution"}:
            return f":::{{{tag}}}\n\n" + self.children(node, depth) + ":::\n\n"
        if tag == "itemizedlist":
            return "\n".join("- " + self.mixed(child) for child in node.findall("listitem")) + "\n\n"
        if tag == "variablelist":
            return "\n".join("- **" + self.mixed(entry.find("term")) + "**: " + self.mixed(entry.find("listitem"))
                             for entry in node.findall("varlistentry")) + "\n\n"
        if tag == "screen":
            return "```text\n" + "".join(node.itertext()).strip() + "\n```\n\n"
        if tag == "imagedata":
            image = self.generated / node.get("fileref", "")
            destination = os.path.relpath(image, self.page.parent).replace("\\", "/")
            return f"![{escape(image.stem)}]({destination})\n\n"
        if tag in {"imageobject", "figure", "refsynopsisdiv", "refentry"}:
            return self.children(node, depth)
        return self.inline(node) + "\n\n"


def registrations(root: Path):
    source = root / "build-tools/generate_server_configs.py"
    spec = importlib.util.spec_from_file_location("xmod_public_cvars", source)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module.registrations()


def registered_reference(root: Path, references: dict[str, Path] | None = None) -> str:
    """Return the same complete cvar reference for Sphinx and the release ZIP."""
    references = references or {}
    lines = ["# Registered server cvars", "", "This list is generated from current server/game registrations, including read-only and internal state variables. It shows compiled defaults; release templates may apply documented starter settings. Empty credential defaults are shown without reading any private configuration.", "", "| Cvar | Compiled default | Flags | Source |", "| --- | --- | --- | --- |"]
    for item in sorted(registrations(root), key=lambda item: item.name.lower()):
        detailed = references.get(f"cvar.{item.name}")
        name = f"[{escape(item.name)}](cvar/{item.name}.md)" if detailed else code(item.name)
        default = code(repr(item.default)).replace("|", "\\|")
        flags = code(item.flags or "0").replace("|", "\\|")
        source = f"[registration](https://github.com/jay1110/xmod/blob/master/{item.source}#L{item.line})"
        lines.append(f"| {name} | {default} | {flags} | {source} |")
    return "\n".join(lines) + "\n"


def generate(app) -> None:
    docs = Path(app.srcdir)
    root = docs.parent
    generated = docs / "_generated"
    compiled = {item.name.lower(): item for item in registrations(root)}
    values = entities(root, app.config.release)
    references: dict[str, Path] = {}
    page_sources: dict[str, Path] = {}
    entries = []
    for category in ("cmd", "cvar"):
        for source in sorted((root / "doc" / category).glob("*.xml"), key=lambda p: p.name.lower()):
            tree = ET.fromstring("<fragment>" + expand(source.read_text(encoding="utf-8"), values) + "</fragment>")
            entry = tree.find("refentry")
            if entry is None:
                raise ValueError(f"Missing refentry in {source}")
            name = entry.findtext("refnamediv/refname", source.stem)
            page = generated / category / f"{name}.md"
            # Page names must remain unique on case-sensitive and Windows filesystems.
            page_key = f"{category}/{name}".casefold()
            if page_key in page_sources:
                previous = page_sources[page_key].relative_to(root).as_posix()
                current = source.relative_to(root).as_posix()
                raise ValueError(f"Duplicate reference page {category}/{name}: {previous} and {current}")
            page_sources[page_key] = source
            for node in entry.iter():
                if node.get("id"):
                    references[node.get("id")] = page
            entries.append((category, source, entry, name, page))

    for category, source, entry, name, page in entries:
        renderer = Renderer(page, references, generated)
        content = f"# {'!' if category == 'cmd' else ''}{name}\n\n"
        content += renderer.mixed(entry.find("refnamediv/refpurpose")) + "\n\n"
        if category == "cmd":
            content += "Use `!help " + name + "` in the installed build for current syntax and permissions.\n\n"
        elif name.lower() in compiled:
            item = compiled[name.lower()]
            content += "**Compiled default:** " + code(repr(item.default)) + ".\n\n"
            content += "**Registration flags:** " + code(item.flags or "0") + ".\n\n"
        for child in entry:
            if category == "cvar" and name.lower() in compiled and child.findtext("title") == "Default":
                continue
            content += renderer.block(child)
        relative = source.relative_to(root).as_posix()
        content += f"[Edit the source reference](https://github.com/jay1110/xmod/blob/master/{relative}).\n"
        write(page, content)

    for category, title, target in (("cmd", "Admin command reference", "commands"), ("cvar", "Cvar reference", "cvars")):
        selected = [(name, page) for kind, _, _, name, page in entries if kind == category]
        content = f"# {title}\n\n"
        if category == "cmd":
            content += "These entries publish the existing command documentation. Enable `g_admin 1`; `!help` lists commands granted to your authenticated account.\n\n"
        else:
            content += "Descriptions and flag tables come from the existing XML references. Server defaults are replaced with current compiled defaults where registered. For the full server list, including newer cvars, see [registered server cvars](registered-cvars.md).\n\n"
        content += "```{toctree}\n:maxdepth: 1\n\n" + "\n".join(f"{category}/{name}" for name, _ in selected) + "\n```\n"
        write(generated / f"{target}.md", content)

    write(generated / "registered-cvars.md", registered_reference(root, references))
    write(generated / "changelog.md", (root / "CHANGELOG.md").read_text(encoding="utf-8"))
    figures = generated / "figures"
    figures.mkdir(parents=True, exist_ok=True)
    for image in (root / "doc/figures").glob("*.png"):
        shutil.copyfile(image, figures / image.name)


def setup(app):
    app.connect("builder-inited", generate)
    return {"version": "1.0", "parallel_read_safe": True, "parallel_write_safe": True}
