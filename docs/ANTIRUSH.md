# Xmod native Antirush

## English

The AutoAdmin 4.4.5 rules from the supplied `antirush_v2.tgz` are implemented in C++ in Xmod 2.0.4. Lua is not required. The package includes all 30 original map configurations (36 objective points).

### Enable

Install the updated server module and Xmod PK3, then put this in the server configuration:

```cfg
set g_antirush "2"
```

Modes: `0` disables protection; `1` keeps the existing name-based rules in `antirush_objectives.cfg` and the duration from `g_antirushTime`; `2` uses the native AutoAdmin rules below. Do not load the original Antirush Lua module alongside mode 2.

Copy the loose `antirush/` folder from the release ZIP to `<fs_homepath>/<fs_game>/antirush/`. It contains `antirush.cfg` and 30 editable map presets; these server files are deliberately absent from the PK3. Preserve your existing edited rules, GUID permissions and mode cache when updating. Restart the map to reload manually edited files. Maps without configured points have no restrictions.

### Rules and settings

| Setting | Default | Meaning |
| --- | --- | --- |
| `PROTECT_PERCENT` | `1 / 3` | Fraction of map timelimit protected; accepts fractions or decimals from 0 to 1. |
| `ADMIN_LEVEL` | `999` | Legacy exact level grant; explicit admin ACL denials take priority. |
| `USER_LEVEL` | `-1` | Legacy editor level; -1 disables, 0 permits authenticated level-0 users unless explicitly denied. |
| `ANTIRUSH_SCAN_RANGE` | `500` | Objective scan range and initial saved protection radius. |
| `TRICKPLANT_SCAN_RANGE` | `200` | Dynamite scan range and initial trickplant radius. |
| `COVERT_DRAIN_RANGE_MULTIPLIER` | `2` | Multiplier when importing legacy `true` charge-drain rows. |
| `ANTIRUSH_COUNTDOWN_INTERVAL_SECONDS` | `60` | Countdown interval; 0 disables announcements. |
| `START_MESSAGE_DELAY_SECONDS` | `5` | Initial announcement delay. |
| `MESSAGES_ENABLED` | `true` | Public announcements; admin replies remain visible. |
| `ANTIRUSH_LOG_FILE` | `antirush/antirush.log` | Admin edit log; an empty string disables it. |

Human players are killed when taking protected objectives or arming dynamite inside a protected sphere. The pickup is rejected before scoring or map script events. Rejected dynamite is removed before objective events and XP. Bots can pick up objectives but cannot move with them until protection expires. This restriction is separate from `!freeze`; death, team changes, objective loss, disconnects and map changes clear it.

Engineer and Covert Ops charge can be drained at independently configured distances, only for the attacking team. Pauses do not consume protection time. Warmup and intermission are excluded. A zero timelimit or protection fraction disables timed protection. Trickplant spheres remove newly armed dynamite throughout the active match without killing the planter, unless a timed antirush sphere also applies. `!antirush off` disables both kinds of protection and charge drain.

Announcements use `MSG_<name>_CHANNEL` and `MSG_<name>_TEXT`. Names: `START`, `END`, `COUNTDOWN`, `RUSH`, `BOT`, `TRICKPLANT`, `TRICKPLANT_UNKNOWN`, `OBJECTIVE_SAVED`. Channels: `chat`, `cpm`, `cp`, `print`, `off`. Supported placeholders are documented beside each entry in the configuration.

### Commands and permissions

`!aa` and `!antirush` are built-in Xmod admin commands, listed by `!help` and recorded in the admin log. Enable the admin system with `set g_admin "1"`. `/aa` remains a client-console alias and uses the same access checks and logging. All player permissions require completed internal Xmod authentication; userinfo GUIDs and bots do not grant access.

Grant `C/aa` for editing and `C/antirush` for status, on/off and GUID management:

```text
!levedit LEVEL -acl +C/aa -acl +C/antirush
!help aa
!help antirush
```

Existing exact `ADMIN_LEVEL`, `USER_LEVEL` and `guids.cfg` grants remain compatible. Explicit level/user denials (`-C/aa`, `-C/antirush`, `-@ALL`, `-@COMMANDS`) take priority. GUID grants only permit editing. Adding/removing GUIDs requires both editor and management access. `!aa ...` works with all editor subcommands below.

| Command | Action |
| --- | --- |
| `/aa help`, `/aa info` | Help, timing and configured maps. |
| `/aa antirush` | List nearby objectives with engine entity IDs. |
| `/aa antirush <entity ID>` | Save an objective protection sphere. |
| `/aa trickplant [entity ID]` | List nearby dynamite or save a trickplant sphere. |
| `/aa list`, `/aa distance` | Saved point IDs, radii and distances. |
| `/aa edit <point ID> <radius or name>` | Change protection radius or objective name; quote multiword names. |
| `/aa edit <point ID> c <radius>` | Covert Ops charge drain distance; 0 disables. |
| `/aa edit <point ID> e <radius>` | Engineer charge drain distance; 0 disables. |
| `/aa remove <point ID>` | Remove a saved point. |
| `/aa addguid <player slot>` | Authorize a connected authenticated human; editor and management access. |
| `/aa removeguid [list ID]` | List GUID permissions or remove an entry from your last list; editor and management access. |
| `!antirush [status]` | Show selected mode, map and remaining active protection time. |
| `!antirush on`, `!antirush off` | Save native protection on/off and select `g_antirush 2`; management access. |

The server console/RCON accepts `!aa ...`, `!antirush status|on|off` and the plain `aa`/`antirush` aliases. Console scans list all matching entities because the console has no player position. `;aa` and `;antirush` are chat aliases using the normal admin dispatch. On/off saves the switch in `settings.cache`; also set `g_antirush 2` in the server configuration so a future server launch does not override the selected feature mode.

Nitmod GUIDs cannot be converted to Xmod identities. The original authorization file, logs and saved mode are intentionally not imported. Reauthorize connected users with `aa addguid <slot>`.

### Map and runtime files

`antirush/maps/<map>.cfg`:

```ini
[antirush]
"Protected objective" 100 200 300 500 750 0
[trickplant]
100 200 300 200
```

Antirush rows contain name, x/y/z, protection radius, Covert Ops radius and Engineer radius. Four-number rows have no charge drain. A legacy fifth value `true` enables both class radii using the configured multiplier; `false` disables them. Trickplant rows contain x/y/z and radius. Editor IDs list antirush entries first, then trickplant entries.

Edits are saved under `<fs_homepath>/<fs_game>/antirush/`. GUID permissions are stored in `guids.cfg`, and the on/off switch in `settings.cache`. Writes replace files atomically and preserve the previous local file as `.bak`. A failed write leaves active settings unchanged. Invalid/unreadable map, GUID or mode files disable editing of that file until repaired and reloaded, preventing a partial load from overwriting existing data. Objective enforcement does not depend on game logging or SQLite.

## Deutsch

Die AutoAdmin-4.4.5-Regeln aus der gelieferten `antirush_v2.tgz` sind in Xmod 2.0.4 fest in C++ eingebaut. Lua wird nicht benötigt. Alle 30 ursprünglichen Map-Konfigurationen mit 36 Objective-Punkten sind enthalten.

### Aktivieren

Aktualisiertes Servermodul und Xmod-PK3 installieren und in der Serverkonfiguration eintragen:

```cfg
set g_antirush "2"
```

`0` deaktiviert den Schutz. `1` behält die bisherigen Namensregeln aus `antirush_objectives.cfg` mit der Zeit aus `g_antirushTime`. `2` aktiviert die native AutoAdmin-Variante. Das ursprüngliche Antirush-Lua-Modul nicht zusätzlich laden.

Den losen Ordner `antirush/` aus der Release-ZIP nach `<fs_homepath>/<fs_game>/antirush/` kopieren. Er enthält `antirush.cfg` und 30 bearbeitbare Map-Vorgaben; diese Serverdateien liegen absichtlich nicht in der PK3. Beim Aktualisieren eigene Regeln, GUID-Rechte und den Modus-Cache erhalten. Manuell geänderte Dateien werden beim nächsten Mapstart geladen. Maps ohne konfigurierte Punkte werden nicht eingeschränkt.

### Regeln und Einstellungen

| Einstellung | Standard | Bedeutung |
| --- | --- | --- |
| `PROTECT_PERCENT` | `1 / 3` | Geschützter Anteil des Map-Zeitlimits; Bruch oder Dezimalzahl zwischen 0 und 1. |
| `ADMIN_LEVEL` | `999` | Bisherige exakte Level-Freigabe; ausdrückliche Admin-ACL-Verbote haben Vorrang. |
| `USER_LEVEL` | `-1` | Bisheriges Editor-Level; -1 deaktiviert, 0 erlaubt authentifizierte Level-0-Spieler ohne ausdrückliches Verbot. |
| `ANTIRUSH_SCAN_RANGE` | `500` | Objective-Suchbereich und anfänglicher Schutzradius. |
| `TRICKPLANT_SCAN_RANGE` | `200` | Dynamit-Suchbereich und anfänglicher Trickplant-Radius. |
| `COVERT_DRAIN_RANGE_MULTIPLIER` | `2` | Multiplikator für alte Map-Zeilen mit `true`. |
| `ANTIRUSH_COUNTDOWN_INTERVAL_SECONDS` | `60` | Countdown-Abstand; 0 deaktiviert Ansagen. |
| `START_MESSAGE_DELAY_SECONDS` | `5` | Verzögerung der Startmeldung. |
| `MESSAGES_ENABLED` | `true` | Öffentliche Ansagen; Befehlsantworten bleiben sichtbar. |
| `ANTIRUSH_LOG_FILE` | `antirush/antirush.log` | Änderungsprotokoll; leerer Text deaktiviert es. |

Menschliche Spieler sterben bei geschützter Objective-Aufnahme oder beim Scharfschalten von Dynamit im Schutzbereich. Aufnahme und Dynamit werden vor Punkten, XP und Mapskript-Ereignissen abgefangen. Bots dürfen Objectives aufnehmen, können damit während der Schutzzeit aber nicht laufen. Diese Sperre ist unabhängig von `!freeze` und endet auch bei Tod, Teamwechsel, Objective-Verlust, Disconnect oder Mapwechsel.

Engineer- und Covert-Ops-Ladung lässt sich in getrennten Radien entziehen, ausschließlich beim angreifenden Team. Pausen verbrauchen keine Schutzzeit. Warmup und Intermission zählen nicht. Zeitlimit oder Schutzanteil 0 deaktivieren den zeitlichen Schutz. Trickplant-Bereiche entfernen neu scharfgeschaltetes Dynamit während des gesamten laufenden Matches, ohne den Spieler zu töten; überschneidender zeitlicher Antirush-Schutz hat Vorrang. `!antirush off` deaktiviert beide Schutzarten und den Ladungsentzug.

Ansagen verwenden `MSG_<Name>_CHANNEL` und `MSG_<Name>_TEXT`. Namen: `START`, `END`, `COUNTDOWN`, `RUSH`, `BOT`, `TRICKPLANT`, `TRICKPLANT_UNKNOWN`, `OBJECTIVE_SAVED`. Kanäle: `chat`, `cpm`, `cp`, `print`, `off`. Die verfügbaren Platzhalter stehen bei den jeweiligen Konfigurationseinträgen.

### Befehle und Rechte

`!aa` und `!antirush` sind eingebaute Xmod-Adminbefehle mit `!help`-Einträgen und Adminprotokollierung. Das Adminsystem mit `set g_admin "1"` aktivieren. `/aa` bleibt als Alias in der Spielkonsole erhalten und verwendet dieselben Rechteprüfungen und Logs. Spielerrechte gelten erst nach interner Xmod-Authentifizierung; Userinfo-GUIDs und Bots vergeben keine Rechte.

`C/aa` erlaubt Bearbeitung, `C/antirush` erlaubt Status, Ein/Aus und GUID-Verwaltung:

```text
!levedit LEVEL -acl +C/aa -acl +C/antirush
!help aa
!help antirush
```

Bisherige Freigaben über das exakte `ADMIN_LEVEL`, `USER_LEVEL` oder `guids.cfg` bleiben kompatibel. Ausdrückliche Level-/Spielerverbote (`-C/aa`, `-C/antirush`, `-@ALL`, `-@COMMANDS`) haben Vorrang. GUID-Freigaben erlauben nur Bearbeitung. GUIDs hinzufügen/entfernen benötigt Bearbeitungs- und Verwaltungszugriff. Alle Editor-Unterbefehle unten funktionieren auch mit `!aa ...`.

| Befehl | Funktion |
| --- | --- |
| `/aa help`, `/aa info` | Hilfe, Schutzzeit und konfigurierte Maps. |
| `/aa antirush [Entity-ID]` | Nahe Objectives anzeigen oder Schutzpunkt speichern. |
| `/aa trickplant [Entity-ID]` | Nahes Dynamit anzeigen oder Trickplant-Punkt speichern. |
| `/aa list`, `/aa distance` | Gespeicherte Punkt-IDs, Radien und Entfernungen. |
| `/aa edit <Punkt-ID> <Radius oder Name>` | Schutzradius oder Objective-Namen ändern; mehrteilige Namen in Anführungszeichen. |
| `/aa edit <Punkt-ID> c <Radius>` | Covert-Ops-Ladungsentzug; 0 deaktiviert. |
| `/aa edit <Punkt-ID> e <Radius>` | Engineer-Ladungsentzug; 0 deaktiviert. |
| `/aa remove <Punkt-ID>` | Schutzpunkt löschen. |
| `/aa addguid <Spielerslot>` | Verbundenen, authentifizierten Menschen berechtigen; Bearbeitungs- und Verwaltungszugriff. |
| `/aa removeguid [Listen-ID]` | Rechte anzeigen oder GUID aus der letzten Liste entfernen; Bearbeitungs- und Verwaltungszugriff. |
| `!antirush [status]` | Modus, Map und verbleibende aktive Schutzzeit anzeigen. |
| `!antirush on`, `!antirush off` | Schutzschalter speichern und `g_antirush 2` auswählen; Verwaltungszugriff. |

Serverkonsole/RCON unterstützt `!aa ...`, `!antirush status|on|off` sowie die Aliase `aa`/`antirush`. Konsolen-Suchen zeigen alle passenden Entities, da die Konsole keine Spielerposition hat. `;aa` und `;antirush` laufen im Chat über die normale Adminverarbeitung. Ein/Aus wird in `settings.cache` gespeichert; zusätzlich `g_antirush 2` in der Serverkonfiguration setzen, damit ein späterer Serverstart den gewählten Funktionsmodus nicht überschreibt.

Nitmod-GUIDs lassen sich nicht in Xmod-Identitäten umrechnen. Alte Berechtigungen, Logs und der gespeicherte Modus werden daher nicht übernommen. Spieler über `aa addguid <Slot>` neu berechtigen.

### Map- und Laufzeitdateien

Das Dateiformat oben gilt unter `antirush/maps/<Map>.cfg`: Objective-Name, x/y/z, Schutzradius, Covert-Ops-Radius, Engineer-Radius. Vier Zahlen bedeuten keinen Ladungsentzug. Ein alter fünfter Wert `true` aktiviert beide Klassenradien mit dem Multiplikator; `false` deaktiviert sie. Trickplant-Zeilen enthalten x/y/z und Radius. Punkt-IDs zählen zuerst Antirush-, danach Trickplant-Einträge.

Änderungen liegen unter `<fs_homepath>/<fs_game>/antirush/`. `guids.cfg` enthält Berechtigungen, `settings.cache` den Ein-/Aus-Schalter. Dateien werden atomisch ersetzt; die vorherige lokale Fassung bleibt als `.bak`. Bei Schreibfehlern bleibt der aktive Zustand erhalten. Fehlerhafte oder unlesbare Map-, GUID- oder Modusdateien lassen sich erst nach Reparatur und Neuladen bearbeiten. Dadurch überschreibt eine unvollständig geladene Datei keine vorhandenen Daten. Die Schutzregeln benötigen weder Spiel-Logging noch SQLite.
