changequote(<<, >>)dnl
include(<<project.m4>>)dnl
dnl
dnl
dnl
///////////////////////////////////////////////////////////////////////////////
//
// __title (r<<>>__repoLCRev)
//
//
// SAMPLE SERVER CONFIGURATION FILE
//
// This file contains standard Enemy Territory server CVARs.
// For xmod-specific CVARs, see xmod.cfg
//
// If you have any questions regarding a specific cvar, check the included
// documentation.
//
// __copyright
// __website
// __irc
//
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// SECURITY
//
///////////////////////////////////////////////////////////////////////////////

// Server password for all players
// Default: ""
// Players need this password to connect to the server
set g_password ""

// Shoutcaster password
// Default: ""
// Password for shoutcaster/spectator mode with special privileges
set g_shoutcastpassword ""

// RCON password
// Default: ""
// Remote console password for server administration
set rconpassword ""

// Referee password
// Default: "none"
// Password to become a referee with special privileges
set refereePassword ""

// Private slots password
// Default: ""
// Password to access reserved/private player slots
set sv_privatePassword ""

///////////////////////////////////////////////////////////////////////////////
//
// LOGGING
//
///////////////////////////////////////////////////////////////////////////////

// Game log file
// Default: ""
// Path to the game log file (e.g., "games.log")
set g_log ""

// Log options bitflags
// Default: "0"
// Bit 1 (1):  Enable chat logging
// Bit 2 (2):  Enable extended weapon stats logging
// Bit 4 (8):  Enable ban logging
set g_logOptions "0"

// Synchronous logging
// Default: "0"
// 0 = Buffered logging, 1 = Immediate disk writes (slower but safer)
set g_logSync "0"

// Admin log file
// Default: ""
// Path to the admin action log file
set g_adminLog ""

///////////////////////////////////////////////////////////////////////////////
//
// BRANDING & SERVER INFO
//
///////////////////////////////////////////////////////////////////////////////

// Server hostname
// Default: "ETHost"
// The name shown in server browser
set sv_hostname "ETHost"

// Server watermark
// Default: "xmod"
// Watermark displayed on HUD (can be disabled by clients)
set g_watermark "xmod"

// Watermark fade-after time
// Default: "60"
// Seconds after spawn before watermark starts fading
set g_watermarkFadeAfter "60"

// Watermark fade duration
// Default: "60"
// Seconds for watermark fade effect
set g_watermarkFadeTime "60"

// Protest message
// Default: "Visit www.myserver.com to file a protest."
// Message shown to players when filing protests
set g_protestMessage "Visit www.myserver.com to file a protest."

// Kick message
// Default: "You have been kicked for $TIME."
// Message shown when player is kicked ($TIME is replaced with duration)
set g_kickMessage "You have been kicked for $TIME."

// Kick duration
// Default: "2m"
// Default kick time duration (e.g., "30s", "5m", "1h")
set g_kickTime "2m"

///////////////////////////////////////////////////////////////////////////////
//
// MOTD (Message of the Day)
//
///////////////////////////////////////////////////////////////////////////////

// MOTD lines (0-5)
// Default: "" for all
// Messages displayed when players connect
set server_motd0 ""
set server_motd1 ""
set server_motd2 ""
set server_motd3 ""
set server_motd4 ""
set server_motd5 ""

///////////////////////////////////////////////////////////////////////////////
//
// MASTER SERVER REGISTRATION
//
///////////////////////////////////////////////////////////////////////////////

// Master servers for server registration
// Default: sv_master1 = "etmaster.idsoftware.com", others = ""
// Servers where this server will register for public listing
set sv_master1 "etmaster.idsoftware.com"
set sv_master2 ""
set sv_master3 ""
set sv_master4 ""
set sv_master5 ""

///////////////////////////////////////////////////////////////////////////////
//
// NETWORKING
//
///////////////////////////////////////////////////////////////////////////////

// Allow downloads from server
// Default: "1"
// 0 = Disabled, 1 = Enabled
set sv_allowDownload "1"

// Maximum download rate
// Default: "42000"
// Maximum bytes/sec for downloads
set sv_dl_maxRate "42000"

// Flood protection
// Default: "1"
// Prevent command spam flooding
set sv_floodProtect "1"

// Server FPS
// Default: "20"
// Server frames per second (20 or 40 recommended)
set sv_fps "20"

// Server full message
// Default: "Server is full."
// Message shown when server is at capacity
set sv_fullmsg "Server is full."

// Force rate for LAN
// Default: "1"
// Force maximum rate for LAN clients
set sv_lanForceRate "1"

// Maximum ping
// Default: "0"
// Maximum allowed ping (0 = no limit)
set sv_maxPing "0"

// Maximum client rate
// Default: "25000"
// Maximum bytes/sec from server to client
set sv_maxRate "25000"

// Maximum clients
// Default: "20"
// Maximum number of players (slots)
set sv_maxclients "20"

// Minimum ping
// Default: "0"
// Minimum required ping (0 = no limit)
set sv_minPing "0"

// Packet delay simulation
// Default: "0"
// Artificial packet delay in ms (for testing)
set sv_packetdelay "0"

// Packet loss simulation
// Default: "0"
// Artificial packet loss percentage (for testing)
set sv_packetloss "0"

// Pad packets
// Default: "0"
// Add padding to packets to reach fixed size
set sv_padPackets "0"

// Private client slots
// Default: "4"
// Number of reserved slots for password holders
set sv_privateClients "4"

// Pure server
// Default: "1"
// 0 = Allow modified pk3s, 1 = Require official pk3s only
set sv_pure "1"

// Reconnect limit
// Default: "3"
// Maximum reconnect attempts within timeout period
set sv_reconnectlimit "3"

// Show average bytes per second
// Default: "0"
// Display average BPS in logs
set sv_showAverageBPS "0"

// Show packet loss
// Default: "0"
// Display packet loss statistics
set sv_showloss "0"

// Client timeout
// Default: "240"
// Seconds before disconnecting idle clients
set sv_timeout "240"

// WWW base URL
// Default: ""
// Base URL for fast downloads via HTTP/FTP
set sv_wwwBaseURL ""

// WWW download while disconnected
// Default: "0"
// Allow downloads while client is disconnected
set sv_wwwDlDisconnected "0"

// WWW download
// Default: "0"
// Enable HTTP/FTP downloads
set sv_wwwDownload "0"

// WWW fallback URL
// Default: ""
// Fallback URL if primary download fails
set sv_wwwFallbackURL ""

// Zombie time
// Default: "2"
// Seconds to wait before removing disconnected players
set sv_zombietime "2"

///////////////////////////////////////////////////////////////////////////////
//
// VOTING
//
///////////////////////////////////////////////////////////////////////////////

// Vote: Allow balanced teams
// Default: "1"
set vote_allow_balancedteams "1"

// Vote: Allow comp settings
// Default: "1"
set vote_allow_comp "1"

// Vote: Allow friendly fire
// Default: "1"
set vote_allow_friendlyfire "1"

// Vote: Allow gametype
// Default: "1"
set vote_allow_gametype "1"

// Vote: Allow generic votes
// Default: "1"
set vote_allow_generic "1"

// Vote: Allow kick
// Default: "1"
set vote_allow_kick "1"

// Vote: Allow map change
// Default: "1"
set vote_allow_map "1"

// Vote: Allow match reset
// Default: "1"
set vote_allow_matchreset "1"

// Vote: Allow match restart
// Default: "1"
set vote_allow_matchrestart "1"

// Vote: Allow mute spectators
// Default: "1"
set vote_allow_mutespecs "1"

// Vote: Allow player muting
// Default: "1"
set vote_allow_muting "1"

// Vote: Allow next map
// Default: "1"
set vote_allow_nextmap "1"

// Vote: Allow public mode
// Default: "1"
set vote_allow_pub "1"

// Vote: Allow referee
// Default: "0"
set vote_allow_referee "0"

// Vote: Allow shuffle teams by XP
// Default: "1"
set vote_allow_shuffleteamsxp "1"

// Vote: Allow swap teams
// Default: "1"
set vote_allow_swapteams "1"

// Vote: Allow time limit
// Default: "0"
set vote_allow_timelimit "0"

// Vote: Allow warmup damage
// Default: "1"
set vote_allow_warmupdamage "1"

// Vote limit
// Default: "5"
// Maximum votes per player per map
set vote_limit "5"

// Vote percentage
// Default: "50"
// Percentage of yes votes needed to pass
set vote_percent "50"

///////////////////////////////////////////////////////////////////////////////
//
// MATCH SETTINGS
//
///////////////////////////////////////////////////////////////////////////////

// Game type
// Default: "2"
// 2 = Objective, 3 = Stopwatch, 4 = Campaign, 5 = Last Man Standing
set g_gametype "2"

// Campaign file
// Default: ""
// Campaign configuration file (for g_gametype 4)
set g_campaignFile ""

// Headshot mode
// Default: "0"
// 0 = Normal, 1 = Headshot only, 2 = Instagib headshots
set g_headshot "0"

// Knife only mode
// Default: "0"
// 0 = Normal, 1 = Knife only
set g_knifeonly "0"

// Panzer war mode
// Default: "0"
// 0 = Normal, 1 = Panzer only
set g_panzerWar "0"

// Sniper war mode
// Default: "0"
// 0 = Normal, 1 = Scoped weapons only
set g_sniperWar "0"

// Match: Late join
// Default: "1"
// Allow joining after match starts
set match_latejoin "1"

// Match: Minimum players
// Default: "0"
// Minimum players required to start match
set match_minplayers "0"

// Match: Mute spectators
// Default: "0"
// Mute spectators during match
set match_mutespecs "0"

// Match: Ready percentage
// Default: "100"
// Percentage of players who must ready up
set match_readypercent "100"

// Match: Timeout count
// Default: "3"
// Number of timeouts per team per match
set match_timeoutcount "3"

// Match: Timeout length
// Default: "180"
// Duration of timeout in seconds
set match_timeoutlength "180"

// Match: Warmup damage
// Default: "1"
// Enable damage during warmup
set match_warmupDamage "1"

///////////////////////////////////////////////////////////////////////////////
//
// TEAMS
//
///////////////////////////////////////////////////////////////////////////////

// Allied respawn time
// Default: "0"
// Custom respawn time for Allies (0 = use map default)
set g_userAlliedRespawnTime "0"

// Axis respawn time
// Default: "0"
// Custom respawn time for Axis (0 = use map default)
set g_userAxisRespawnTime "0"

// Team force balance
// Default: "1"
// Enforce balanced teams
set g_teamForceBalance "1"

// Ammo cabinet recharge time
// Default: "60000"
// Milliseconds between ammo cabinet recharges
set g_ammoRechargeTime "60000"

// Health cabinet recharge time
// Default: "10000"
// Milliseconds between health cabinet recharges
set g_healthRechargeTime "10000"

// Maximum artillery strikes
// Default: "6"
// Maximum artillery strikes per team
set team_maxArtillery "6"

// Maximum landmines
// Default: "20"
// Maximum landmines per team
set team_maxLandMines "20"

// Maximum flamers
// Default: "-1"
// Maximum flamers per team (-1 = no limit)
set team_maxFlamers "-1"

// Maximum grenade launchers
// Default: "-1"
// Maximum grenade launchers per team (-1 = no limit)
set team_maxGrenLaunchers "-1"

// Maximum shotguns
// Default: "-1"
// Maximum M97 shotguns per team (-1 = no limit)
set team_maxM97s "-1"

// Maximum MG42s
// Default: "-1"
// Maximum MG42s per team (-1 = no limit)
set team_maxMG42s "-1"

// Maximum mortars
// Default: "-1"
// Maximum mortars per team (-1 = no limit)
set team_maxMortars "-1"

// Maximum panzers
// Default: "-1"
// Maximum panzerfausts per team (-1 = no limit)
set team_maxPanzers "-1"

// Maximum players per team
// Default: "0"
// Maximum players per team (0 = sv_maxclients/2)
set team_maxplayers "0"

// Maximum medics
// Default: "-1"
// Maximum medics per team (-1 = no limit)
set team_maxMedics "-1"

// Maximum engineers
// Default: "-1"
// Maximum engineers per team (-1 = no limit)
set team_maxEngineers "-1"

// Maximum field ops
// Default: "-1"
// Maximum field ops per team (-1 = no limit)
set team_maxFieldOps "-1"

// Maximum covert ops
// Default: "-1"
// Maximum covert ops per team (-1 = no limit)
set team_maxCovertOps "-1"

// Team controls
// Default: "1"
// Allow team controls (1 = no controls, meaning players can't control)
set team_nocontrols "1"

///////////////////////////////////////////////////////////////////////////////
//
// GAMEPLAY
//
///////////////////////////////////////////////////////////////////////////////

// Friendly fire
// Default: "1"
// 0 = Off, 1 = On
set g_friendlyFire "1"

// Alternative stopwatch mode
// Default: "0"
// Use alternative stopwatch scoring
set g_altStopwatchMode "0"

// No team switching
// Default: "0"
// Prevent players from switching teams
set g_noTeamSwitching "0"

// Force respawn
// Default: "0"
// Force players to respawn immediately
set g_forcerespawn "0"

// Gravity
// Default: "800"
// World gravity value
set g_gravity "800"

// Speed
// Default: "320"
// Player movement speed
set g_speed "320"

// Knockback
// Default: "1000"
// Knockback force from damage
set g_knockback "1000"

// Smooth clients
// Default: "1"
// Smooth client movement prediction
set g_smoothClients "1"

// Inactivity timeout
// Default: "0"
// Seconds before kicking inactive players (0 = disabled)
set g_inactivity "0"

// Spectator inactivity timeout
// Default: "0"
// Seconds before kicking inactive spectators (0 = disabled)
set g_spectatorInactivity "0"

// Warmup
// Default: "30"
// Warmup time in seconds before match
set g_warmup "30"

// Anti-warp
// Default: "1"
// Enable anti-warp for laggy players
set g_antiwarp "1"

// Fixed physics
// Default: "1"
// Use fixed framerate physics
set g_fixedPhysics "1"

// Fixed physics FPS
// Default: "125"
// Physics framerate when fixed physics enabled
set g_fixedPhysicsFPS "125"

// Drag corpse
// Default: "1"
// Allow dragging corpses
set g_dragCorpse "1"

// Heavy weapon restriction
// Default: "100"
// Percentage of players allowed to use heavy weapons
set g_heavyWeaponRestriction "100"

// Auto fireteams
// Default: "1"
// Automatically create fireteams
set g_autoFireteams "1"

// Complaint limit
// Default: "6"
// Number of complaints before action
set g_complaintlimit "6"

// IP complaint limit
// Default: "3"
// Complaints from same IP before action
set g_ipcomplaintlimit "3"

// Disable complaints
// Default: "0"
// Completely disable complaint system
set g_disableComplaints "0"

// Filter cams
// Default: "0"
// Filter camera views for spectators
set g_filtercams "0"

// Filter ban
// Default: "1"
// Ban players matching ban filter
set g_filterBan "1"

// Maximum lives
// Default: "0"
// Maximum lives per player (0 = unlimited)
set g_maxlives "0"

// Allied maximum lives
// Default: "0"
// Maximum lives for Allied team (0 = use g_maxlives)
set g_alliedmaxlives "0"

// Axis maximum lives
// Default: "0"
// Maximum lives for Axis team (0 = use g_maxlives)
set g_axismaxlives "0"

// Enforce maximum lives
// Default: "1"
// Kick players who exceed max lives
set g_enforcemaxlives "1"

// Maximum lives respawn penalty
// Default: "0"
// Respawn time penalty when out of lives
set g_maxlivesRespawnPenalty "0"

// Voice chats allowed
// Default: "4"
// Number of voice commands per period
set g_voiceChatsAllowed "4"

// Landmine timeout
// Default: "1"
// Mines explode and remove after timer (0 = permanent)
set g_landminetimeout "1"

// Dynamite time
// Default: "30"
// Seconds before dynamite explodes
set g_dynamiteTime "30"

// Intermission time
// Default: "30"
// Seconds of intermission before next map
set g_intermissionTime "30"

// Intermission ready percent
// Default: "75"
// Percentage of players needed to skip intermission
set g_intermissionReadyPercent "75"

// Maximum game clients
// Default: "0"
// Maximum active players (0 = no limit)
set g_maxGameClients "0"

// Save campaign stats
// Default: "1"
// Save stats between campaign maps
set g_saveCampaignStats "1"

// Map configs directory
// Default: "mapconfigs"
// Directory for per-map configuration files
set g_mapConfigs "mapconfigs"

// Map script directory
// Default: "mapscripts"
// Directory for map scripts
set g_mapScriptDirectory "mapscripts"

// LMS: Team force balance
// Default: "1"
set g_lms_teamForceBalance "1"

// LMS: Lock teams
// Default: "0"
set g_lms_lockTeams "0"

// LMS: Round limit
// Default: "3"
set g_lms_roundlimit "3"

// LMS: Match limit
// Default: "2"
set g_lms_matchlimit "2"

// LMS: Follow team only
// Default: "1"
set g_lms_followTeamOnly "1"

///////////////////////////////////////////////////////////////////////////////
//
// PUNKBUSTER
//
///////////////////////////////////////////////////////////////////////////////

// Enable PunkBuster
// Default: Disabled
// Uncomment to enable PunkBuster anti-cheat
// pb_sv_enable

///////////////////////////////////////////////////////////////////////////////
//
// XMOD CONFIGURATION
//
///////////////////////////////////////////////////////////////////////////////

// Load xmod-specific configuration
exec xmod.cfg

///////////////////////////////////////////////////////////////////////////////
//
// MAP ROTATION
//
///////////////////////////////////////////////////////////////////////////////

// Simple repeating map setup
// Configure your map rotation below
set nextmap "map fueldump"
set com_watchdog_cmd "map fueldump"
vstr com_watchdog_cmd
