#!/usr/bin/env python3
"""Generate and check the complete Xmod server-cvar template from registrations."""

import argparse
import ast
from dataclasses import dataclass
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
STRING = r'"(?:[^"\\]|\\.)*"'
INTERNAL = {
    "dedicated", "developer", "sv_cheats", "P", "g_currentRound",
    "g_nextTimeLimit", "z_serverflags", "cg_letterbox", "bot_enable",
    "nextmap", "nextcampaign", "g_test", "server_autoconfig",
}
HOST_CVARS = {"sv_maxclients", "sv_fps", "sv_maxrate", "url"} | {
    "server_motd" + str(index) for index in range(6)
}
PRIVATE_CVARS = {
    "rconpassword", "g_password", "refereepassword", "sv_privatepassword",
    "g_shoutcastpassword", "g_vpnblockerapikey1", "g_vpnblockerapikey2",
}
# Preserve the previous starter's practical choices in the file owning each
# setting. Any difference from the registered default is explicitly annotated.
STARTER_OVERRIDES = {
    "g_admin": "1", "g_gametype": "2", "g_log": "games.log",
    "g_logOptions": "33", "g_mapScriptDirectory": "mapscripts",
    "g_teamForceBalance": "1", "omnibot_enable": "0",
}
NOTES = {
    "g_admin": "Admin command system; set to 1 to enable (!help, !levedit, etc.).",
    "g_antirush": "0 = off; 1 = legacy name rules; 2 = native Antirush. See ANTIRUSH.md.",
    "g_antirushTime": "Protection seconds for legacy mode 1 only. Mode 2 uses antirush/antirush.cfg.",
    "g_mapRecords": "0 = off; 1 = SQLite map records. Commands: !records and !stats.",
    "g_logOptions": "Flags: 1 = date/time, 2 = weapon stats, 4 = reserved, 8 = bans, 16 = legacy date/time, 32 = objectives.",
    "g_vpnBlockerEnabled": "0 = off; 1 = asynchronous VPN/blacklist checks. See VPN_BLOCKER.md.",
    "g_vpnBlockerApiKey1": "VPN provider key. Leave empty here; put private keys in server-private.cfg.",
    "g_vpnBlockerApiKey2": "ipapi.is provider key. Leave empty here; put private keys in server-private.cfg.",
    "g_vpnBlockerMaxLevel": "Authenticated admin levels ABOVE this value bypass VPN checks.",
    "g_vpnBlockerDBPath": "Separate SQLite VPN database for IP allow/block lists and authenticated GUID exceptions; compatible with the old JAR schema.",
    "g_vpnBlockerBanMessageVPN": "Connection rejection message for detected VPN/proxy addresses.",
    "g_vpnBlockerBanMessageBlacklist": "Connection rejection message for manually blacklisted addresses; empty uses the built-in fallback.",
    "g_weaponScriptsDir": "Custom weapon script directory. Must be distributed to clients; restart the map after changing.",
    "g_userConfig": "SQLite user database filename. Keep the existing database when updating.",
    "g_oss": "Supported-client OS/architecture bitmask advertised to the server browser.",
    "omnibot_enable": "Enable native Omni-bot support. Set omnibot_path to the matching bot installation.",
    "omnibot_path": "Directory containing the architecture-matched Omni-bot library.",
    "g_mapConfigs": "Optional directory for per-map cfg files; empty disables per-map overrides.",
    "g_mapScriptDirectory": "Loose map-script directory; the starter selects the supplied mapscripts folder.",
    "g_killSpreeLevels": "Space-separated kill counts; empty uses built-in spree thresholds.",
    "g_loseSpreeLevels": "Space-separated death counts; empty uses built-in losing-spree thresholds.",
    "g_defaultSkills": "Optional starting skill levels; leave empty for the built-in defaults.",
    "jxac_cvarFile": "Optional legacy JXAC cvar-list file. See JXAC.md for current rule files.",
    "jxac_forceCvarFile": "Forced client cvars; reload with !jxac_reload after editing.",
    "jxac_md5File": "Local MD5 denylist; GAMEHACK screenshot then kick or jxac_autoBan, no masterserver. Reload: !jxac_reload.",
    "jxac_autoBan": "Permanent SQLite admin ban after cheat screenshot completes or fails. 0: GAMEHACK still kicks; 1: ban. Requires authenticated identity and writable database.",
    "jxac_moduleScan": "Loaded-module hashes and Windows memory fingerprints. !jxac_status shows reported scan coverage; see JXAC.md.",
}


@dataclass
class Cvar:
    name: str
    default: str
    flags: str
    source: str
    line: int

    @property
    def writable(self):
        return not ("CVAR_ROM" in self.flags or "CVAR_CHEAT" in self.flags or self.name in INTERNAL)


def without_comments(text):
    # Preserve strings (URLs, message text) and line numbers while removing C++ comments.
    return re.sub(STRING + r"|//[^\n]*|/\*[\s\S]*?\*/",
                  lambda m: m[0] if m[0].startswith('"') else "\n" * m[0].count("\n"), text)


def registrations():
    result = {}
    macros = {}
    for source in ("src/bgame/bg_public.h", "src/game/g_xmod.h"):
        for name, value in re.findall(r'^#define\s+(\w+)\s+(' + STRING + ')',
                                      (ROOT / source).read_text(encoding="utf-8"), re.M):
            macros[name] = ast.literal_eval(value)

    def add(name, value, flags, source, line):
        if value.startswith('"'):
            default = ast.literal_eval(value)
        elif value in macros:
            default = macros[value]
        elif "CVAR_ROM" in flags:
            default = "<build-dependent>"
        else:
            raise ValueError(f"Unresolved default {value} for {name} in {source}:{line}")
        key = name.lower()
        entry = Cvar(name, default, flags.strip(), source, line)
        if key in result:
            if result[key].default != default:
                raise ValueError(f"Conflicting registered defaults for {name}")
            result[key].flags += " | " + entry.flags
        else:
            result[key] = entry

    source = "src/game/g_main.cpp"
    text = without_comments((ROOT / source).read_text(encoding="utf-8"))
    begin = text.index("gameCvarTable[] = {")
    end = text.index("\n};", begin)
    table = text[begin:end]
    pattern = r'\{\s*[^,]+,\s*("\w+")\s*,\s*(' + STRING + r'|\w+)\s*,\s*([^,}]+)'
    matches = list(re.finditer(pattern, table))
    if table.count("{") - 1 != len(matches):
        raise ValueError("A gameCvarTable entry was not understood")
    for match in matches:
        add(ast.literal_eval(match[1]), match[2], match[3], source,
            text.count("\n", 0, begin + match.start()) + 1)

    source = "src/game/static.cpp"
    text = without_comments((ROOT / source).read_text(encoding="utf-8"))
    pattern = r'\bCvar\s+\w+\s*\(\s*("\w+")\s*,\s*(' + STRING + r')\s*([^;]*?)\);'
    matches = list(re.finditer(pattern, text))
    if len(re.findall(r'\bCvar\s+\w+\s*\(', text)) != len(matches):
        raise ValueError("A static Cvar registration was not understood")
    for match in matches:
        tail = match[3].lstrip()
        flags = tail[1:].split(",", 1)[0].strip() if tail.startswith(",") else "0"
        add(ast.literal_eval(match[1]), match[2], flags, source, text.count("\n", 0, match.start()) + 1)

    # Registered by map-name lookup helpers rather than gameCvarTable.
    add("mapname", '""', "CVAR_ROM | CVAR_SERVERINFO", "src/game/g_cmds.cpp", 0)
    return sorted(result.values(), key=lambda item: item.name.lower())


def description(name):
    if name in NOTES:
        return NOTES[name]
    source = ROOT / "doc/cvar" / f"cvar.{name}.xml"
    if source.exists():
        match = re.search(r"<refpurpose>(.*?)</refpurpose>", source.read_text(encoding="utf-8"), re.S)
        if match:
            value = re.sub(r"<[^>]+>", "", match[1])
            value = re.sub(r"&([\w-]+);", r"\1", value)
            return " ".join(value.split()).rstrip(".") + "."
    return ""


def quote(value):
    if any(c in value for c in '\r\n"'):
        raise ValueError("Cvar default cannot be safely represented as an ET cfg string")
    return '"' + value + '"'


def generate():
    cvars = registrations()
    lines = [
        "changequote(<<, >>)dnl", "include(<<project.m4>>)dnl", "dnl",
        "// __title - complete mod configuration",
        "// EN: Gameplay/admin settings belong here; host settings are in server.cfg.",
        "// DE: Spiel-/Admin-Einstellungen stehen hier; Host-Einstellungen in server.cfg.",
        "// Registered source defaults are used except explicitly marked starter choices.",
        "// Quellcode-Defaults gelten, ausser bei ausdruecklich markierten Startvorgaben.",
        "// server.cfg loads this file, then server-private.cfg, before starting the map.",
        "// Source template is generated with: python build-tools/generate_server_configs.py",
        "// This covers server/game cvars, not personal client (cg_ / r_) settings.",
        "// API keys/passwords belong only in server-private.cfg; never put real values in a PK3/ZIP.",
        "// CVAR_LATCH settings require a map/server restart. Restart after editing Antirush files.",
        "// See ANTIRUSH.md, VPN_BLOCKER.md and JXAC.md for commands and detailed setup.",
        "",
    ]
    groups = [
        ("Gameplay, administration and server defaults / Spiel, Verwaltung und Server", lambda c: c.writable and not c.name.lower().startswith(("jxac_", "g_vpn", "omnibot_", "vote_", "team_", "match_", "server_motd"))),
        ("Team, match and voting settings / Teams, Match und Abstimmungen", lambda c: c.writable and c.name.lower().startswith(("vote_", "team_", "match_", "server_motd"))),
        ("Omni-bot", lambda c: c.writable and c.name.lower().startswith("omnibot_")),
        ("JXAC", lambda c: c.writable and c.name.lower().startswith("jxac_")),
        ("VPN blocker / VPN-Blocker", lambda c: c.writable and c.name.lower().startswith("g_vpn")),
    ]
    owned_cvars = [c for c in cvars if c.name.lower() not in HOST_CVARS | PRIVATE_CVARS]
    for title, matches in groups:
        lines.extend(["// " + "=" * 74, "// " + title, "// " + "=" * 74, ""])
        for cvar in filter(matches, owned_cvars):
            note = description(cvar.name)
            if note:
                lines.append("// " + note)
            if "CVAR_LATCH" in cvar.flags:
                lines.append("// Latched: takes effect after restart / Wirksam nach Neustart.")
            value = STARTER_OVERRIDES.get(cvar.name, cvar.default)
            if value != cvar.default:
                lines.append("// Starter choice / Startvorgabe; registered default / Quellcode-Default: " + quote(cvar.default))
            lines.extend([f"set {cvar.name} {quote(value)}", ""])
    lines.extend([
        "// " + "=" * 74,
        "// Lua extension cvars (read directly, not registered with a default)",
        "// Lua-Erweiterungen (direkt gelesen, ohne registrierten Default)",
        "// " + "=" * 74,
        '// Space-separated Lua scripts; native Antirush does not need a Lua module.',
        'set lua_modules ""',
        '// Optional module-signature allow-list; empty permits the configured modules.',
        'set lua_allowedModules ""',
        "",
        "// Host and private settings are configured once in their owning files.",
        "// Host- und Zugangsdaten werden nur in ihrer eigenen Datei gesetzt.",
    ])
    for cvar in (c for c in cvars if c.name.lower() in HOST_CVARS | PRIVATE_CVARS):
        owner = "server-private.cfg" if cvar.name.lower() in PRIVATE_CVARS else "server.cfg"
        lines.append(f"// {cvar.name}: {owner}")
    lines.extend([
        "",
        "// " + "=" * 74,
        "// Read-only, engine/session state and development cvars (reference only)",
        "// Nur lesbar, Engine-/Matchstatus und Entwicklung (nur Referenz)",
        "// Do not enable these lines in a production configuration.",
        "// Diese Zeilen in einer Produktionskonfiguration nicht aktivieren.",
        "// " + "=" * 74,
    ])
    for cvar in (c for c in cvars if not c.writable):
        reason = "read-only" if "CVAR_ROM" in cvar.flags else "cheat/development" if "CVAR_CHEAT" in cvar.flags else "engine/session state"
        lines.append(f"// {cvar.name} {quote(cvar.default)} ({reason})")
    lines.append("")
    return "\n".join(lines), cvars


def active_settings(text):
    result = {}
    for name, value in re.findall(r'^\s*set\s+(\w+)\s+"([^"\r\n]*)"\s*$', text, re.M):
        key = name.lower()
        if key in result:
            raise ValueError("Duplicate active setting: " + name)
        result[key] = value
    return result


def validate_templates():
    registry = registrations()
    all_values = {}
    owners = {}
    counts = {}
    for filename in ("server.cfg", "xmod.cfg", "server-private.cfg"):
        text = (ROOT / "pkg" / (filename + ".m4")).read_text(encoding="utf-8-sig")
        values = active_settings(text)
        counts[filename] = len(values)
        for name, value in values.items():
            if name in owners:
                raise ValueError(f"Duplicate {name}: {owners[name]} and {filename}")
            all_values[name] = value
            owners[name] = filename
        if filename == "server-private.cfg":
            if set(values) != PRIVATE_CVARS or any(values.values()):
                raise ValueError("Private release template must contain exactly the seven blank credentials")
    for cvar in registry:
        key = cvar.name.lower()
        if cvar.writable:
            expected_owner = "server-private.cfg" if key in PRIVATE_CVARS else "server.cfg" if key in HOST_CVARS else "xmod.cfg"
            if owners.get(key) != expected_owner:
                raise ValueError(f"Missing or misplaced {cvar.name}; expected {expected_owner}")
            if expected_owner == "xmod.cfg" and all_values[key] != STARTER_OVERRIDES.get(cvar.name, cvar.default):
                raise ValueError(f"Default/starter mismatch for {cvar.name}")
        elif key in owners:
            raise ValueError("Read-only/state/development setting must not be active: " + cvar.name)
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Fail if the tracked template is stale.")
    args = parser.parse_args()
    content, cvars = generate()
    target = ROOT / "pkg/xmod.cfg.m4"
    if args.check:
        if target.read_text(encoding="utf-8") != content:
            raise SystemExit("pkg/xmod.cfg.m4 is stale; run generate_server_configs.py")
    else:
        target.write_text(content, encoding="utf-8", newline="\n")
    counts = validate_templates()
    print(f"Verified {len(cvars)} unique registered cvars: {sum(c.writable for c in cvars)} configurable, "
          f"{sum(not c.writable for c in cvars)} documented as read-only/state/development; plus 2 Lua extension cvars.")
    print("No active duplicates; settings by file: " + ", ".join(f"{name}={count}" for name, count in counts.items()))


if __name__ == "__main__":
    main()
