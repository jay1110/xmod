changequote(<<, >>)dnl
include(<<project.m4>>)dnl
dnl
// __title - host/engine settings and startup
// EN: Copy the three cfg files and antirush/ beside the server module.
// DE: Die drei cfg-Dateien und antirush/ neben das Servermodul kopieren.
// Launch: etlded +set fs_game xmod +set dedicated 2 +exec server.cfg
// Use etlded64 / etlded.exe as appropriate for your installed engine.
// Gameplay, admin and mod logging settings belong in xmod.cfg.
// Passwords and API keys belong only in server-private.cfg; preserve that file on updates.
// Spiel, Adminsystem und Mod-Logs: xmod.cfg. Zugangsdaten: server-private.cfg.
// Vorhandene private Konfigurationen bei Updates erhalten.

// Identity and player slots / Servername und Spielerplaetze
set sv_hostname "Xmod 2.0.4 Server"
set sv_maxclients "20"
set sv_privateClients "0"
set URL ""
set server_motd0 "^3Welcome to Xmod 2.0.4"
set server_motd1 "^7Have fun and respect other players."
set server_motd2 ""
set server_motd3 ""
set server_motd4 ""
set server_motd5 ""

// Networking and downloads / Netzwerk und Downloads
// Bind address/port on the engine command line before the server starts:
// +set net_ip 0.0.0.0 +set net_port 27960
set sv_fps "20"
set sv_maxRate "90000"
set sv_minPing "0"
set sv_maxPing "0"
set sv_timeout "240"
set sv_zombietime "2"
set sv_reconnectlimit "3"
set sv_floodProtect "1"
set sv_lanForceRate "1"
set sv_pure "1"
set sv_allowDownload "1"
set sv_dl_maxRate "42000"
set sv_wwwDownload "0"
set sv_wwwBaseURL ""
set sv_wwwDlDisconnected "0"
set sv_wwwFallbackURL ""
// Enable sv_wwwDownload only after hosting the matching PK3/maps and editing the URL above.
// Example URL: https://your-download-host.example/et (without embedded credentials).
// Master-server addresses are supplied by your engine; override only if needed.
// set sv_master1 "your-master-server.example"

// Engine console log / Engine-Konsolenlog
// Game/admin/JXAC log paths and options are configured in xmod.cfg.
set logfile "2"

// Load each setting from its owning file; credentials are loaded last.
// Jede Einstellung steht nur in einer Datei; Zugangsdaten werden zuletzt geladen.
exec xmod.cfg
exec server-private.cfg

// Start a standard-map rotation / Rotation mit Standardmaps starten
set d1 "set nextmap vstr d2; map oasis"
set d2 "set nextmap vstr d3; map goldrush"
set d3 "set nextmap vstr d1; map fueldump"
set com_watchdog "60"
set com_watchdog_cmd "vstr d1"
vstr d1
