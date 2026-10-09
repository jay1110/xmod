# Xmod 2.0.4

## English

Updated: 2026-10-09 · Unreleased

- **Fixed:**
  - Fixed macOS architecture checks and Universal packaging in GitHub Actions; added missing Alpine Linux headers.
  - Fixed configuration documentation lookup on case-sensitive filesystems to unblock Linux documentation builds.
  - Removed a duplicate cvar reference that broke Read the Docs builds on Linux.
  - Store relative admin and JXAC logs in the mod directory; default JXAC logs use `jxac/jxac.log`.
  - Prevented duplicate archive members when rebuilding WASM libraries.
  - Moved VPN API checks off the game thread; cancelled stale decisions after disconnects and map changes.
  - Isolated Unix VPN socket signals and kept DNS resolver code loaded across map changes; preserved musl crash logs.
  - Preserved pointer-sized native engine arguments, including macOS ARM64; corrected musl bot compilation.
  - Prevented repeated Daybreak countdown completion and separated bomb and secret-room counters.
  - Protected `level.db` against overwrites after load errors; atomic saves with a backup.
  - Corrected bomb models, textures and weapon icons.
  - Fixed loading issues caused by oversized MD3 surfaces.
  - Both bombs can be thrown on either team after `/give all`.
  - Fixed Limbo and intermission panels drifting after map changes.
  - Fireteam data is correctly reset and reloaded after `vid_restart`.
  - Corrected the PPSH graphic in the Limbo menu.
  - Applied custom weapon settings consistently on the server and client, including zero values and disabled flags.
  - Reapplied forced cvars continuously and restored them after client restarts.
  - Preserved chat text and special characters; limited JXAC bulk traffic and adapted UDP screenshot chunks to engine limits.
  - Fixed false JXAC heartbeat timeouts after connecting, map changes and client restarts; cleared stale client state.
  - Persisted timed and permanent `!nospam` restrictions, sharing one cooldown across text and voice channels.

- **Modified:**
  - Separate host, mod and private settings across three configs without duplicate assignments.
  - Ship editable Antirush rules beside the server modules, outside the PK3.
  - Updated mod version, build metadata and PK3/release names to 2.0.4.
  - Separate Axis and Allies bombs with dedicated models and weapon scripts.
  - Nitmod-style intermission with twelve awards, icons and result values.
  - The game world remains visible behind the intermission menu.
  - Logged objective names for planting, defusing, dropping, returning, capturing and destruction.
  - `g_logOptions`: flag 1 adds local date/time, flag 4 stays unused; flag 32 enables objective logs.

- **Added:**
  - Complete `server.cfg`, `xmod.cfg` and a blank `server-private.cfg` template in release archives.
  - Sphinx documentation and Read the Docs configuration with searchable command and cvar references.
  - Built-in Antirush and VPN admin commands with permissions, help and audit logs.
  - Persistent VPN IP/range lists and authenticated Xmod GUID exemptions, compatible with the old Java database.
  - Native Antirush with 30 map presets, objective/trickplant protection, bot holds, class charge drain and an authenticated GUID editor.
  - Integrated PR #192 VPN providers with bundled JsonCpp and native Windows, Linux and macOS support.
  - Extended VPN release builds to Linux x86/x64/ARM64, macOS Intel/Apple Silicon and separate Alpine/musl modules.
  - `weaponcard03.tga` for the PPSH in the Limbo menu.
  - Round counters for sprees, healing, revives and class achievements.
  - Local Linux 32/64-bit builds on Windows without WSL.
  - Updated Windows, Linux 32/64-bit and WASM32 builds.
  - Gitignore rules for the test environment, local tests, screenshots and logs.
  - `/records`, `!records` and `!stats`, with SQLite map records controlled by `g_mapRecords` (default: 1).

- **Removed:**
  - Obsolete bomb models replaced by the new models.

## Deutsch

Stand: 09.10.2026 · Noch nicht veröffentlicht

- **Fixed:**
  - macOS-Architekturprüfung und Universal-Pakete in GitHub Actions korrigiert; fehlende Alpine-Linux-Header ergänzt.
  - Config-Dokumentation auch auf Dateisystemen mit Groß-/Kleinschreibung korrekt gefunden; Linux-Dokumentationsbuilds repariert.
  - Doppelte Cvar-Referenz entfernt, die Read-the-Docs-Builds unter Linux verhinderte.
  - Relative Admin- und JXAC-Logs im Mod-Verzeichnis abgelegt; JXAC verwendet standardmäßig `jxac/jxac.log`.
  - Doppelte Archiveinträge beim erneuten Bauen der WASM-Bibliotheken verhindert.
  - VPN-API-Prüfungen in den Hintergrund verlegt; veraltete Ergebnisse nach Disconnects und Mapwechseln verworfen.
  - Unix-VPN-Socketsignale getrennt und DNS-Code bei Mapwechseln geladen gehalten; musl-Crashlogs erhalten.
  - Pointerbreite native Engine-Argumente erhalten, auch unter macOS ARM64; musl-Botkompilierung korrigiert.
  - Erneuten Daybreak-Countdown-Abschluss verhindert; Bomben- und Geheimraumzähler getrennt.
  - `level.db` gegen Überschreiben nach Ladefehlern geschützt; atomisches Speichern mit Backup.
  - Bombenmodelle, Texturen und Waffenicons korrigiert.
  - Ladeprobleme durch zu große MD3-Modellflächen behoben.
  - Beide Bomben nach `/give all` unabhängig vom Team verwendbar.
  - Verschieben von Limbo- und Intermission-Panels bei Mapwechseln behoben.
  - Fireteam-Daten nach `vid_restart` korrekt zurückgesetzt und neu geladen.
  - Fehlerhafte PPSH-Grafik im Limbo-Menü korrigiert.
  - Eigene Waffeneinstellungen auf Server und Client durchgängig angewendet, einschließlich Nullwerten und deaktivierten Flags.
  - Erzwungene Cvars laufend durchgesetzt und nach Client-Neustarts wiederhergestellt.
  - Chattext und Sonderzeichen erhalten; JXAC-Massendaten gedrosselt und UDP-Screenshot-Blöcke an Engine-Grenzen angepasst.
  - Falsche JXAC-Heartbeat-Timeouts nach Verbindung, Mapwechseln und Client-Neustarts behoben; veraltete Clientzustände bereinigt.
  - Zeitliche und dauerhafte `!nospam`-Sperren gespeichert; gemeinsame Wartezeit für Text- und Sprachkanäle.

- **Modified:**
  - Server-, Mod- und private Einstellungen ohne doppelte Zuweisungen auf drei Configs verteilt.
  - Bearbeitbare Antirush-Regeln neben den Servermodulen statt in der PK3 ausgeliefert.
  - Modversion, Build-Metadaten und PK3-/Release-Namen auf 2.0.4 angehoben.
  - Eigene Bomben für Axis und Allies mit getrennten Modellen und Waffenskripten.
  - Intermission mit zwölf Auszeichnungen, Icons und Ergebniswerten im Nitmod-Stil.
  - Spielwelt bleibt hinter dem Intermission-Menü sichtbar.
  - Objective-Namen beim Legen, Entschärfen, Fallenlassen, Zurückbringen, Erobern und Zerstören protokolliert.
  - `g_logOptions`: Flag 1 ergänzt Datum/Uhrzeit, Flag 4 bleibt unbenutzt; Flag 32 aktiviert Objective-Logs.

- **Added:**
  - Vollständige `server.cfg`, `xmod.cfg` und leere Vorlage `server-private.cfg` in Release-Archiven.
  - Sphinx-Dokumentation und Read-the-Docs-Konfiguration mit durchsuchbarer Befehls- und Cvar-Referenz.
  - Antirush- und VPN-Befehle im Adminsystem mit Rechten, Hilfe und Protokollierung.
  - Gespeicherte VPN-IP-/Bereichslisten und authentifizierte Xmod-GUID-Ausnahmen, kompatibel mit der alten Java-Datenbank.
  - Natives Antirush mit 30 Map-Vorgaben, Objective-/Trickplant-Schutz, Bot-Sperren, Klassenladungsentzug und Editor mit authentifizierten GUID-Rechten.
  - VPN-Anbieter aus PR #192 mit gebündeltem JsonCpp und nativer Windows-, Linux- und macOS-Unterstützung übernommen.
  - VPN-Release-Builds um Linux x86/x64/ARM64, macOS Intel/Apple Silicon und getrennte Alpine-/musl-Module erweitert.
  - `weaponcard03.tga` für die PPSH im Limbo-Menü.
  - Rundenzähler für Spree-, Heilungs-, Wiederbelebungs- und Klassenleistungen.
  - Lokale Linux-32-/64-Bit-Builds unter Windows ohne WSL.
  - Aktualisierte Windows-, Linux-32-/64-Bit- und WASM32-Builds.
  - Gitignore-Regeln für Testumgebung, lokale Tests, Screenshots und Logs.
  - `/records`, `!records` und `!stats` mit SQLite-Maprekorden über `g_mapRecords` (Standard: 1).

- **Removed:**
  - Alte, ersetzte Bombenmodelle.
