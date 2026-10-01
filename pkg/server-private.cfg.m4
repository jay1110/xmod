changequote(<<, >>)dnl
include(<<project.m4>>)dnl
dnl
// __title - private credentials (blank release template)
// EN: Fill in your own credentials locally. server.cfg loads this file automatically.
// DE: Eigene Zugangsdaten lokal eintragen. server.cfg laedt diese Datei automatisch.
// Never publish a completed copy or put it in a PK3, Git, download directory or release ZIP.
// Eine ausgefuellte Datei niemals in PK3, Git, Download-Verzeichnis oder Release-ZIP aufnehmen.
// Keep your existing file on updates. Public release copies contain empty values only.
// Vorhandene Datei bei Updates erhalten. Die Release-Vorlage enthaelt nur leere Werte.
// Gameplay, VPN enable/level settings and log paths belong in xmod.cfg, not here.

// Remote console password. Empty disables password-based RCON access.
// Passwort fuer die Fernkonsole. Leer deaktiviert den Passwortzugang.
set rconpassword ""

// Password required for every player; empty leaves the server public.
// Zugangspasswort fuer alle Spieler; leer bedeutet oeffentlicher Server.
set g_password ""

// Referee login password; empty disables password-based referee login.
// Referee-Passwort; leer deaktiviert den Passwort-Login.
set refereePassword ""

// Password for slots reserved with sv_privateClients in server.cfg.
// Passwort fuer die durch sv_privateClients reservierten Slots.
set sv_privatePassword ""

// Shoutcaster password; empty disables password-based shoutcaster login.
// Shoutcaster-Passwort; leer deaktiviert den Passwort-Login.
set g_shoutcastpassword ""

// vpnapi.io API key. Configure g_vpnBlockerEnabled in xmod.cfg after adding keys.
// API-Key von vpnapi.io. Nach dem Eintragen g_vpnBlockerEnabled in xmod.cfg setzen.
set g_vpnBlockerApiKey1 ""

// ipapi.is API key. See VPN_BLOCKER.md for provider and administration details.
// API-Key von ipapi.is. Anbieter- und Verwaltungsdetails stehen in VPN_BLOCKER.md.
set g_vpnBlockerApiKey2 ""
