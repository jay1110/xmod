# Server setup / Server einrichten

## English

### Install the release

Copy the Xmod PK3 and the server module matching your engine's operating system
and architecture into its `xmod` mod directory. Use the loose `server.cfg`,
`xmod.cfg`, `server-private.cfg` and `antirush/` files from the release ZIP.
The Antirush directory belongs on the server filesystem and is absent from the
PK3 so administrators can edit map rules directly.

The server's writable mod directory is `<fs_homepath>/<fs_game>`; with
`fs_game xmod`, this is `<fs_homepath>/xmod`. Install the configuration and
editable server data there. Keep the PK3/client files accessible through the
engine's normal mod search path. Preserve existing user/level databases,
custom Antirush rules and private settings when updating.

Start the dedicated engine with the mod and server configuration, for example:

```text
etlded64 +set fs_game xmod +exec server.cfg
```

Use your engine's actual executable name and set `fs_homepath` explicitly if
you keep the writable data outside its default directory. The chosen map must
be installed in the engine's search path.

### Configuration order

| File | Purpose |
| --- | --- |
| `server.cfg` | Host, network, MOTD and map-rotation setup. Executes the next two files before starting the map. |
| `xmod.cfg` | Gameplay, administration, logging and other mod cvars. This is the file to edit for mod behavior. |
| `server-private.cfg` | Passwords and VPN provider keys. Loaded last so public defaults cannot overwrite private values. |

The seven credential fields in the supplied private template are empty:
`rconpassword`, `g_password`, `refereePassword`, `sv_privatePassword`,
`g_shoutcastpassword`, `g_vpnBlockerApiKey1` and `g_vpnBlockerApiKey2`.
Fill only the credentials needed by your server. Preserve this local file
when installing updates and keep it out of Git and client PK3s.

The [registered server cvars](_generated/registered-cvars.md) show the defaults
compiled into the current source. The release templates also apply documented
starter settings, such as enabling the admin system. Personal client settings
(`cg_`, renderer settings and key bindings) belong to the client configuration.
Latched cvars take effect after the required map/server restart.

### Administration and logs

`set g_admin "1"` enables the admin system. `!help` lists commands available to
the authenticated user; `!help COMMAND` shows syntax and its privilege name.
The server console/RCON is trusted. Player permissions use confirmed internal
Xmod GUIDs.

Use the dedicated guides for [Antirush](ANTIRUSH.md), the
[VPN blocker and persistent lists](VPN_BLOCKER.md) and [JXAC](JXAC.md).
These commands are part of the normal admin system, with individual `C/...`
privileges and admin logging.

The relative log defaults resolve inside the writable mod directory:

- `g_adminLog "admin.log"` writes `<fs_homepath>/<fs_game>/admin.log`.
- `jxac_logFile "jxac/jxac.log"` writes the JXAC log in its `jxac` subdirectory.

Existing explicit absolute log paths remain supported. Legacy paths beginning
with the current mod folder, such as `xmod/admin.log`, are normalized to avoid
an extra nested `xmod` directory.

## Deutsch

### Installation

PK3 und das zur Server-Engine passende Modul in den Mod-Ordner kopieren.
`server.cfg`, `xmod.cfg`, `server-private.cfg` und `antirush/` kommen als lose
Dateien aus dem Release-ZIP auf den Server. `antirush/` liegt nicht in der PK3,
damit Map-Regeln direkt bearbeitet werden können.

Der beschreibbare Mod-Ordner ist `<fs_homepath>/<fs_game>`, normalerweise
`<fs_homepath>/xmod`. Dort Konfiguration und veränderliche Serverdaten ablegen.
Vorhandene Benutzer-/Level-Datenbanken, eigene Antirush-Regeln und private
Einstellungen bei Updates erhalten. Der Serverstart erfolgt beispielsweise
mit `etlded64 +set fs_game xmod +exec server.cfg`; den Dateinamen der Engine
und bei Bedarf `fs_homepath` an die eigene Installation anpassen.

### Konfiguration und Verwaltung

`server.cfg` enthält Host-, Netzwerk-, MOTD- und Maprotationseinstellungen.
Es lädt zuerst `xmod.cfg` für Mod-/Gameplay-/Admin-/Log-Einstellungen und danach
`server-private.cfg` für Passwörter und VPN-Keys. Erst anschließend startet die
Maprotation. Jede aktive Cvar hat in den Vorlagen einen zuständigen Ort.

Die private Vorlage enthält sieben leere Zugangsdatenfelder. Nur benötigte
Werte eintragen, die vorhandene Datei bei Updates erhalten und keine echten
Keys oder Passwörter in Git oder eine PK3 übernehmen. Die
[Cvar-Referenz](_generated/registered-cvars.md) zeigt die Quellcode-Defaults;
die Release-Vorlagen enthalten zusätzlich dokumentierte Starteinstellungen.

`g_admin 1` aktiviert das Adminsystem. `!help` und `!help BEFEHL` zeigen die
verfügbaren Befehle und Rechte. Spieler benötigen eine bestätigte interne
GUID; Serverkonsole/RCON sind vertrauenswürdig. Antirush, VPN und JXAC sind
in den verlinkten Anleitungen beschrieben.

`admin.log` und `jxac/jxac.log` liegen relativ zu `<fs_homepath>/<fs_game>`.
Explizite absolute Pfade funktionieren weiterhin. Ein alter führender
Mod-Ordner wie `xmod/admin.log` erzeugt keinen zusätzlichen `xmod`-Unterordner.
