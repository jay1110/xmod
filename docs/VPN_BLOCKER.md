# VPN blocker

Xmod 2.0.4 integrates the providers from PR #192 with background lookups.
Windows uses WinHTTP; Linux and macOS use libcurl. JsonCpp is linked statically,
so no separate JsonCpp DLL/shared library is needed. Browser/WASM and Android
server builds do not perform VPN lookups; native servers still check players
using these clients normally.

Linux servers need the libcurl runtime (`libcurl.so.4`) installed, including
its TLS/CA certificate support. Use the matching architecture for the server
binary. Windows needs no additional HTTP or JsonCpp DLL.

## Native server platforms

The same background checker is used on every native desktop/server target:

| Server target | HTTPS transport | Build |
| --- | --- | --- |
| Windows x86 / x64 | Windows WinHTTP | MSVC / CMake |
| Linux x86 / x86_64 | System libcurl | `PLATFORM=linux` / `PLATFORM=linux64` |
| Linux ARM64 | System libcurl | `PLATFORM=linux-aarch64` |
| macOS Intel / Apple Silicon | macOS system libcurl | `PLATFORM=osx64` / `PLATFORM=osx-arm64` |
| Alpine / musl x86_64 | musl-native libcurl | Separate native `PLATFORM=linux64` build |

The release workflows build these Unix targets and check module loading,
architecture and VPN worker behavior. macOS releases contain both Intel and
Apple Silicon slices; a missing architecture fails the build. Alpine modules
are packaged separately from glibc modules.

Linux distribution names do not select a different VPN implementation. Debian,
Ubuntu, Fedora, openSUSE and other distributions need matching CPU architecture,
C++ runtime, libc and libcurl. Install the distribution's matching libcurl and
CA certificate packages; a 32-bit server needs 32-bit libraries even on a 64-bit
OS. libcurl must provide HTTPS and asynchronous DNS (threaded resolver or
c-ares), so lookup timeouts remain effective.

One ELF binary cannot cover every libc/version combination. The local Ubuntu
22.04 sysroot builds require glibc 2.34+, libstdc++ with `GLIBCXX_3.4.30`, and
OpenSSL-flavour `libcurl.so.4` (`CURL_OPENSSL_4`). Older systems need a native
rebuild against their own libraries. Alpine's musl modules require a matching
musl engine and dependencies; do not mix them with a glibc engine. On musl,
Xmod retains signal metadata in crash logs but has no glibc `execinfo` stack
trace.

Local Windows-hosted cross-builds require no WSL:

```powershell
./build-tools/build-linux32-local.ps1
./build-tools/build-linux64-local.ps1
```

The checker belongs to the **server**. Native servers also check clients using
WASM, Android, or another client OS. Running the server module itself inside a
browser or Android remains unsupported for VPN queries.

References: [libcurl thread safety and resolver requirements](https://curl.se/libcurl/c/threadsafe.html),
[resolver lifetime during cleanup](https://curl.se/libcurl/c/curl_global_cleanup.html).

## Configuration

Put real API keys in a private server configuration, outside Git:

```cfg
set g_vpnBlockerEnabled "1"
set g_vpnBlockerApiKey1 "YOUR_VPNAPI_IO_KEY"
set g_vpnBlockerApiKey2 "YOUR_IPAPI_IS_KEY"
set g_vpnBlockerMaxLevel "999"
set g_vpnBlockerBanMessageVPN "VPN/proxy connections are not allowed on this server."
set g_vpnBlockerDBPath "vpnblocker.sqlite"
set g_vpnBlockerBanMessageBlacklist "This address is blocked on this server."
```

Either provider can be used on its own by leaving the other key empty.
`g_vpnBlockerEnabled` defaults to `0`. Authenticated admin levels **above**
`g_vpnBlockerMaxLevel` are exempt; the threshold itself is checked. Thus `999`
checks players through level 999. The default threshold is `0`.

## Administration

The commands below use Xmod's normal admin system and their own `C/COMMAND`
privileges. They are listed by `!help` for permitted administrators and run through
the normal admin log (`g_adminLog`). Player administrators must finish internal
GUID authentication first; the server console/RCON is trusted.

| Command | Function |
| --- | --- |
| `!vpn` or `!vpn status` | Show enabled/platform state, configured providers, exemption threshold and check counts; never show keys or player IPs. |
| `!vpn on` | Enable checking and schedule eligible connected players in the background. |
| `!vpn off` | Disable checking and cancel pending decisions. |
| `!vpn check PLAYER` | Queue a fresh check for a connected slot or unique player name, bypassing the temporary result cache. |
| `!vpn check all` | Queue fresh checks for all eligible connected players. |
| `!vpn reload` | Reload the configured SQLite lists after external edits. |
| `!vpn-check IP` | Check one public IPv4 address; report the result privately without kicking a player. |
| `!whitelist show [PAGE]` | List saved IP exemptions, eight per page. |
| `!whitelist add-ip IP REASON` | Exempt one IPv4 address. |
| `!whitelist add-ip-range START END REASON` | Exempt an inclusive IPv4 range. |
| `!whitelist remove-entry ID` | Remove an exemption by its database ID. |
| `!blacklist show [PAGE]` | List saved IP blocks. |
| `!blacklist add-ip IP REASON` | Block one IPv4 address. |
| `!blacklist add-ip-range START END REASON` | Block an inclusive IPv4 range. |
| `!blacklist remove-entry ID` | Remove a block by its database ID. |
| `!nguidlist show [PAGE]` | List GUID exemptions; old Nitmod GUIDs are marked inactive. |
| `!nguidlist add-nguid GUID REASON` | Exempt a confirmed 40-character Xmod GUID. |
| `!nguidlist remove-nguid GUID` | Remove a saved Xmod or legacy Nitmod GUID. |

From the server console/RCON, use the same commands with the `!` prefix.
Grant a level access with `!levedit LEVEL -acl +C/vpn`, remove an explicit grant
with `!levedit LEVEL -acl !C/vpn`, or explicitly deny it with
`!levedit LEVEL -acl -C/vpn`. Existing full-admin privileges also apply.
`!help vpn` shows command syntax. The list/check commands have separate privileges:

```text
!levedit LEVEL -acl +C/vpn -acl +C/vpn-check -acl +C/whitelist -acl +C/blacklist -acl +C/nguidlist
```

`C/vpn` alone does not grant access to the saved lists. Use `!help whitelist`,
`!help blacklist`, `!help nguidlist` and `!help vpn-check` for their syntax.

Manual checks remain asynchronous and obey the same bot/private-address and
authenticated-level exclusions. They never override `g_vpnBlockerMaxLevel`.
A positive result uses the configured disconnect message; it is not a permanent
ban. Configure API keys through the private server configuration, never through
chat/admin commands. `on`/`off` changes the running archived cvar; keep the desired
startup value in the server configuration as well.

## Connection behavior

- Connecting schedules a lookup without waiting for DNS, HTTP or retries.
  The game keeps running while the player joins. A positive result disconnects
  that player from the main server thread; it does not create a permanent ban.
- Bots, localhost and private/reserved addresses are skipped.
- GUID authentication can finish after spawning. A positive result waits up to
  15 seconds after it is observed on a connected client for authentication,
  allowing an authenticated exempt administrator to stay connected.
- Disconnects, reused slots, changed API settings and map changes invalidate
  pending decisions. Background work is cancelled and joined before unloading
  the server module. On Unix, the shared libcurl image stays loaded until the
  process exits because its DNS resolver threads can outlive a cancelled request.
- Provider requests have time and response-size limits. Successful decisions
  are cached temporarily; a provider failure is never treated as evidence of a
  VPN. If there is no positive result and a lookup fails, the connection stays
  open and the server logs that the lookup was unavailable.
- vpnapi.io checks VPN, proxy, Tor and relay flags. ipapi.is also checks bogon,
  crawler, datacenter and abuser flags, matching the original PR's policy.
- Keys and provider URLs are not printed in VPN diagnostic messages.

## Persistent lists and migration from VPN_Blocker 3.1

`g_vpnBlockerDBPath` defaults to `vpnblocker.sqlite` in the server's home/game
directory. An empty path disables local lists. The database is separate from
Xmod's player database and is created when absent. List changes are saved
immediately; they invalidate previous check decisions and re-evaluate connected
players. Local lists work without API keys while `g_vpnBlockerEnabled` is `1`.

For public player addresses, the priority is: authenticated admin-level exemption,
authenticated Xmod GUID exemption, IP whitelist, IP blacklist, then background
API providers. A blacklist hit uses `g_vpnBlockerBanMessageBlacklist`, with a
default message when empty. It disconnects the player; the stored IP rule remains
until removed. Turning the blocker off suspends enforcement without deleting lists.

The original Java tool's SQLite layout (`ip_list` and `nguid_list`) is supported.
Stop the old tool and copy its database into the server's home/game directory as
`vpnblocker.sqlite`, or set the cvar to another filename there. Keep the original
as a backup. Use `!vpn reload` after replacing or editing the file externally.
The release archive contains neither the personal database nor API keys.
Local address lists follow the original tool's IPv4 format; online checks and
`!vpn-check` also accept public IPv6 addresses.

Signed 32-bit IPv4 values from Java are interpreted correctly, including addresses
above `127.255.255.255`. A row with `ip_2 = NULL` covers its single `ip_1` address.
Reasons and IDs are retained. Imported 32-character Nitmod GUIDs remain visible
but **do not grant an exemption in Xmod**. Add the corresponding authenticated
40-character Xmod GUIDs explicitly; never copy an unverified userinfo GUID.

List reads use an in-memory snapshot; database errors are reported rather than
replacing the file. A failed reload keeps the previous valid snapshot. List
mutations use prepared statements and transactions. HTTP requests remain on the
background worker.

## Deutsch

Die Prüfung läuft im Hintergrund und hält den Spielablauf nicht an. Erkannte
VPN-/Proxy-Verbindungen werden anschließend getrennt. Bei API-Fehlern bleibt
die Verbindung bestehen. Bots sowie lokale und private IP-Adressen werden
übersprungen.

Authentifizierte Admins **oberhalb** von `g_vpnBlockerMaxLevel` sind ausgenommen;
mit `999` werden alle Level bis einschließlich 999 geprüft. Die vorhandene
GUID-Anmeldung bekommt bei einem positiven Treffer bis zu 15 Sekunden Zeit.
Verspätete Ergebnisse dürfen nach Disconnect, Slotwechsel oder Mapwechsel
keinen anderen Spieler treffen.

API-Keys nur in einer privaten Serverkonfiguration speichern. Die lokale
SQLite-Verwaltung funktioniert auch ohne API-Key. `g_vpnBlockerDBPath` ist
standardmäßig `vpnblocker.sqlite` im Home-/Mod-Verzeichnis des Servers; ein
leerer Pfad deaktiviert die lokalen Listen. Die Datei ist von Xmods
Spielerdatenbank getrennt und wird bei Bedarf neu angelegt.

Die Befehle gehören zum normalen Adminsystem mit eigenen Rechten `C/BEFEHL` und werden
im Adminlog (`g_adminLog`) erfasst. Spieler müssen vorher ihre interne GUID
authentifiziert haben; Serverkonsole und RCON sind vertrauenswürdig.

| Befehl | Funktion |
| --- | --- |
| `!vpn` oder `!vpn status` | Status, konfigurierte Anbieter, Level-Grenze und Anzahl der Prüfungen anzeigen; keine Keys oder Spieler-IP-Adressen. |
| `!vpn on` | Aktivieren und verbundene, geeignete Spieler im Hintergrund prüfen. |
| `!vpn off` | Deaktivieren und ausstehende Entscheidungen verwerfen. |
| `!vpn check SPIELER` | Slot oder eindeutigen Namen erneut prüfen; der temporäre Ergebnis-Cache wird übersprungen. |
| `!vpn check all` | Alle geeigneten verbundenen Spieler erneut prüfen. |
| `!vpn reload` | SQLite-Listen nach externen Änderungen neu laden. |
| `!vpn-check IP` | Eine öffentliche IPv4-Adresse prüfen; Ergebnis privat ausgeben, keinen Spieler kicken. |
| `!whitelist show [SEITE]` | IP-Freigaben mit je acht Einträgen anzeigen. |
| `!whitelist add-ip IP GRUND` | Einzelne IPv4-Adresse freigeben. |
| `!whitelist add-ip-range START ENDE GRUND` | IPv4-Bereich einschließlich beider Grenzen freigeben. |
| `!whitelist remove-entry ID` | Freigabe anhand ihrer Datenbank-ID entfernen. |
| `!blacklist show [SEITE]` | Gespeicherte IP-Sperren anzeigen. |
| `!blacklist add-ip IP GRUND` | Einzelne IPv4-Adresse sperren. |
| `!blacklist add-ip-range START ENDE GRUND` | IPv4-Bereich sperren. |
| `!blacklist remove-entry ID` | Sperre anhand ihrer Datenbank-ID entfernen. |
| `!nguidlist show [SEITE]` | GUID-Ausnahmen anzeigen; alte Nitmod-GUIDs sind als inaktiv markiert. |
| `!nguidlist add-nguid GUID GRUND` | Authentifizierte Xmod-GUID mit 40 Zeichen freigeben. |
| `!nguidlist remove-nguid GUID` | Gespeicherte Xmod- oder alte Nitmod-GUID entfernen. |

Die gleichen Befehle einschließlich `!` funktionieren in der Serverkonsole
und per RCON. Rechte vergeben: `!levedit LEVEL -acl +C/vpn`; explizite Vergabe
löschen: `!levedit LEVEL -acl !C/vpn`; ausdrücklich verweigern:
`!levedit LEVEL -acl -C/vpn`. `!help vpn` zeigt die Syntax.

Für die gesamte Verwaltung alle fünf Rechte vergeben:

```text
!levedit LEVEL -acl +C/vpn -acl +C/vpn-check -acl +C/whitelist -acl +C/blacklist -acl +C/nguidlist
```

`C/vpn` allein erlaubt keine Listenverwaltung. Die jeweiligen `!help`-Einträge
zeigen alle Parameter. Listenänderungen werden sofort gespeichert und führen
zu einer erneuten Prüfung verbundener Spieler.

Reihenfolge für öffentliche Spieleradressen: authentifizierte Level-Ausnahme,
authentifizierte Xmod-GUID-Ausnahme, IP-Whitelist, IP-Blacklist, danach API-Anbieter.
Ein Blacklist-Treffer trennt den Spieler mit `g_vpnBlockerBanMessageBlacklist`
(bei leerem Wert mit einer Standardmeldung). Die IP-Regel bleibt gespeichert.
`!vpn off` setzt die Durchsetzung aus und erhält alle Listen.

Die SQLite-Datei des alten Java-Programms ist kompatibel. Altes Programm stoppen,
Original als Backup behalten und eine Kopie als `vpnblocker.sqlite` ins
Home-/Mod-Verzeichnis des Servers legen; alternativ dort einen anderen Dateinamen
über die Cvar setzen. Nach externen Änderungen `!vpn reload` ausführen.
Persönliche Datenbanken und API-Keys gehören nicht ins Release-Paket.
Die lokalen Adresslisten verwenden wie das Original IPv4; Online-Prüfungen und
`!vpn-check` akzeptieren zusätzlich öffentliche IPv6-Adressen.

Vorzeichenbehaftete Java-IPv4-Zahlen werden korrekt gelesen. `ip_2 = NULL`
bedeutet eine einzelne Adresse. IDs und Gründe bleiben erhalten. Alte
Nitmod-GUIDs mit 32 Zeichen bleiben sichtbar, sind in Xmod aber **keine gültige
Ausnahme**. Die zugehörigen authentifizierten Xmod-GUIDs mit 40 Zeichen gezielt
neu hinzufügen. Fehlgeschlagenes Neuladen erhält den vorherigen gültigen Stand;
Datenbankfehler überschreiben keine vorhandene Datei. Schreibvorgänge verwenden
gebundene SQL-Parameter und Transaktionen.

Manuelle Prüfungen laufen ebenfalls im Hintergrund und beachten alle Ausnahmen,
auch `g_vpnBlockerMaxLevel`. Ein Treffer trennt den Spieler und erzeugt keinen
dauerhaften Ban. `on`/`off` ändert die laufende, archivierte Cvar; den gewünschten
Startwert zusätzlich in der Serverkonfiguration pflegen. API-Keys niemals über
Chat- oder Adminbefehle eingeben.

Linux-Server benötigen die passende libcurl-Laufzeitbibliothek (`libcurl.so.4`)
mit TLS-Unterstützung und CA-Zertifikaten. Unter Windows wird keine zusätzliche
HTTP- oder JsonCpp-DLL benötigt.

Der native Prüfer gilt für Windows 32/64 Bit, Linux 32/64 Bit und ARM64 sowie
macOS mit Intel und Apple Silicon. Die Release-Workflows prüfen die Module und
den Hintergrund-Worker; Alpine/musl bekommt getrennte Builds. macOS-Pakete
müssen beide Architekturen enthalten.

Bei anderen Linux-Distributionen müssen CPU-Architektur, libc, C++-Laufzeit und
libcurl zusammenpassen. Ein 32-Bit-Server braucht auch auf einem 64-Bit-System
32-Bit-Bibliotheken. Die lokalen Ubuntu-Builds benötigen mindestens glibc 2.34,
`GLIBCXX_3.4.30` und OpenSSL-libcurl (`CURL_OPENSSL_4`). Für ältere Systeme
nativ neu bauen; musl-Module nur mit einem passenden musl-Spielserver nutzen.
Unter musl enthalten Crashlogs weiterhin Signaldaten, aber keinen
glibc-Stacktrace. libcurl braucht HTTPS und asynchrone DNS-Auflösung.

Linux-Builds direkt unter Windows sind mit `build-linux32-local.ps1` und
`build-linux64-local.ps1` möglich. Der VPN-Blocker läuft auf dem Server; der
Client darf weiterhin WASM oder Android verwenden. Ein WASM-/Android-Server
selbst führt keine VPN-Abfragen aus.
