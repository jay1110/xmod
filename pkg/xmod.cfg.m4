changequote(<<, >>)dnl
include(<<project.m4>>)dnl
dnl
// __title - complete mod configuration
// EN: Gameplay/admin settings belong here; host settings are in server.cfg.
// DE: Spiel-/Admin-Einstellungen stehen hier; Host-Einstellungen in server.cfg.
// Registered source defaults are used except explicitly marked starter choices.
// Quellcode-Defaults gelten, ausser bei ausdruecklich markierten Startvorgaben.
// server.cfg loads this file, then server-private.cfg, before starting the map.
// Source template is generated with: python build-tools/generate_server_configs.py
// This covers server/game cvars, not personal client (cg_ / r_) settings.
// API keys/passwords belong only in server-private.cfg; never put real values in a PK3/ZIP.
// CVAR_LATCH settings require a map/server restart. Restart after editing Antirush files.
// See ANTIRUSH.md, VPN_BLOCKER.md and JXAC.md for commands and detailed setup.

// ==========================================================================
// Gameplay, administration and server defaults / Spiel, Verwaltung und Server
// ==========================================================================

set fraglimit "0"

// Admin command system; set to 1 to enable (!help, !levedit, etc.).
// Latched: takes effect after restart / Wirksam nach Neustart.
// Starter choice / Startvorgabe; registered default / Quellcode-Default: ""
set g_admin "1"

// enables admin chat visibility for privileged players.
set g_adminChat "1"

// set filename used for admin command logging.
set g_adminLog "admin.log"

// sets maximum number of lives for Allied players.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_alliedmaxlives "0"

// allow players to drop their primary weapon.
set g_allowDropWeapon "0"

// enable/disable alternative stopwatch gametype.
set g_altStopwatchMode "0"

// adjusts the fire delay for weapons in milliseconds.
set g_ammoFireDelayNudge "0"

// adjusts the next-shot delay for weapons in milliseconds.
set g_ammoNextDelayNudge "0"

// set time interval between ammo-pack cabinet respawns.
set g_ammoRechargeTime "60000"

// enables unlimited ammunition for all weapons.
set g_ammoUnlimited "0"

// 0 = off; 1 = legacy name rules; 2 = native Antirush. See ANTIRUSH.md.
set g_antirush "0"

// Protection seconds for legacy mode 1 only. Mode 2 uses antirush/antirush.cfg.
set g_antirushTime "30"

// enable/set bitflags for antiwarp functionality.
set g_antiwarp "1"

// enable friendly artillery zone warnings.
set g_artilleryHints "1"

// enables/disables automatic fireteam placement.
set g_autoFireteams "1"

// sets maximum number of lives for Axis players.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_axismaxlives "0"

set g_banIPs ""

// set banner location.
set g_bannerLocation "0"

// set number of banners to display.
set g_banners "0"

// set the duration of display for each banner.
set g_bannerTime "5"

// the amount of time between Allied team respawns.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_bluelimbotime "30000"

// set active bulletmode.
set g_bulletmode "0"

// set bitflags for bulletmode debugging.
set g_bulletmodeDebug "0"

// set reference bulletmode for comparison.
set g_bulletmodeReference "1"

// set maximum number of bullet trails to render.
set g_bulletmodeTrail "0"

// set campaign filename.
set g_campaignFile ""

// enables kicking of canisters (health and ammo packs).
set g_canisterKick "0"

// sets the kick distance for canisters.
set g_canisterKickDistance "250"

// restricts canister kicking to the pack owner only.
set g_canisterKickOwner "0"

// enable/disable word-censor feature.
set g_censor "0"

// set bitflags for censorship penalties.
set g_censorPenalty "0"

// enables/disables friendly corpse class tealing.
set g_classChange "0"

// override maximum HP per player class.
set g_classesMaxHP "0 0 0 0 0"

// sets the maximum number of complaints a player can receive per map.
set g_complaintlimit "6"

// enables country flag display next to player names.
set g_countryflags "1"

// set bitflags for covertops behavior.
set g_covertops "0"

// set amount of time for covertops to recharge.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_covertopsChargeTime "30000"

// controls weapon damage modifications.
set g_damageweapons "0"

// enables XP for weapons damage awarded based on damage inflicted.
set g_damagexp "0"

// enables debugging of the game's server stack.
set g_debugAlloc "0"

// enables debug information for player movement.
set g_debugMove "0"

// enables debugging of the skills system.
set g_debugSkills "0"

// Optional starting skill levels; leave empty for the built-in defaults.
set g_defaultSkills ""

// disables fiendly death complaints for certain weapons.
set g_disableComplaints "0"

// set double jump height multiplier.
set g_djHeight "1.4"

// toggle double jump.
set g_doubleJump "0"

set g_doWarmup "0"

// enables corpse dragging.
set g_dragCorpse "0"

// enables ammo crate drops on field ops death.
set g_dropAmmo "0"

// enables heath pack drops on medic death.
set g_dropHealth "0"

// set how many times per life a player can drop an objective.
set g_dropObj "0"

// enables dual SMG akimbo weapons.
set g_dualSMG "0"

// sets the timer for dynamite in seconds.
set g_dynamiteTime "30"

// enables player tracking to enforce max lives between connects.
set g_enforcemaxlives "1"

// set amount of time for engineer to recharge.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_engineerChargeTime "30000"

// set bitflags for engineer behavior.
set g_engineers "0"

// enables fast player revives.
set g_fastres "0"

// awards a kill to on attacker if their victim suicides.
set g_fear "0"

// filters players joining the server.
set g_filterBan "1"

// removes players from camera views.
set g_filtercams "0"

// enable/disable physics corrections.
set g_fixedphysics "1"

// sets the emulated FPS used for fixed physics.
set g_fixedphysicsfps "125"

// forces a player to go into limbo after a specified amount of time.
set g_forcerespawn "0"

// set bitflags for friendly fire behavior.
set g_friendlyFire "1"

// set general mode of gameplay.
// Latched: takes effect after restart / Wirksam nach Neustart.
// Starter choice / Startvorgabe; registered default / Quellcode-Default: "4"
set g_gametype "2"

// makes all players emit a colored glow.
set g_glow "0"

// enables damage from above.
set g_goomba "0"

// sets the amount of gravity.
set g_gravity "800"

// set bitflags for headshot beahvior.
set g_headshot "0"

// set time interval between ammo-pack cabinet respawns.
set g_healthRechargeTime "10000"

// sets a limit of heavy weapons that can be used at once per team.
set g_heavyWeaponRestriction "100"

// set active hitmode.
set g_hitmode "0"

// set maximum amount of antilag in milliseconds.
set g_hitmodeAntilag "800"

// enable/disable antilag lerping.
set g_hitmodeAntilagLerp "1"

// set bitflags for hitmode debugging.
set g_hitmodeDebug "0"

// set increased torso-box size in inches.
set g_hitmodeFat "0"

// set lifetime of hit ghosting in milliseconds.
set g_hitmodeGhosting "0"

// set reference hitmode for comparison.
set g_hitmodeReference "1"

// set zone for debugging.
set g_hitmodeZone "1"

// set player inactivity limit.
set g_inactivity "0"

// set bitflags for inactivity behavior.
set g_inactivityOptions "0"

// bypass reinforcement wave wait when joining a team.
set g_instantJoinTeam "0"

// enables instant spawning without waiting for the next spawn wave.
set g_instantSpawn "0"

// sets the percentage of 'readied' players needed to end intermission.
set g_intermissionReadyPercent "100"

// sets the intermission duration.
set g_intermissionTime "60"

// set maximum number of unique complaints allowed for a player.
set g_ipcomplaintlimit "3"

// set kick message.
set g_kickMessage "You have been kicked for ^G$TIME^*."

// set duration to ban kicked players.
set g_kickTime "2m"

// enables kill assistance tracking and announcements.
set g_killAssistances "1"

// set killing spree mode.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_killingSpree "0"

// Space-separated kill counts; empty uses built-in spree thresholds.
set g_killSpreeLevels ""

// enable/disable knife-only game mode.
set g_knifeonly "0"

// set knockback effect.
set g_knockback "1000"

// enable/disable landmine and tripmine cleanup upon owner disconnect.
set g_landminetimeout "1"

// graduated levels of battlesense XP.
set g_levels_battlesense ""

// graduated levels of covertops XP.
set g_levels_covertops ""

// graduated levels of engineer XP.
set g_levels_engineer ""

// graduated levels of fieldops XP.
set g_levels_fieldops ""

// graduated levels of lightweapons XP.
set g_levels_lightweapons ""

// graduated levels of medic XP.
set g_levels_medic ""

// graduated levels of soldier XP.
set g_levels_soldier ""

// enable/disable same-team spectator restriction.
set g_lms_followTeamOnly "1"

// enable/disable locked teams during match play.
set g_lms_lockTeams "0"

// set maximum number of matches to play before nextmap.
set g_lms_matchlimit "2"

// set maximum number of rounds to play before match ends.
set g_lms_roundlimit "3"

// enable/disable passive team balancing.
set g_lms_teamForceBalance "1"

// set game log output file.
// Starter choice / Startvorgabe; registered default / Quellcode-Default: ""
set g_log "games.log"

// Flags: 1 = date/time, 2 = weapon stats, 4 = reserved, 8 = bans, 16 = legacy date/time, 32 = objectives.
// Starter choice / Startvorgabe; registered default / Quellcode-Default: "0"
set g_logOptions "33"

// enable/disable log file sync.
set g_logSync "0"

// Space-separated death counts; empty uses built-in losing-spree thresholds.
set g_loseSpreeLevels ""

// set amount of time for fieldops to recharge.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_LTChargeTime "40000"

// Optional directory for per-map cfg files; empty disables per-map overrides.
set g_mapConfigs ""

// 0 = off; 1 = SQLite map records. Commands: !records and !stats.
set g_mapRecords "1"

// Loose map-script directory; the starter selects the supplied mapscripts folder.
// Starter choice / Startvorgabe; registered default / Quellcode-Default: ""
set g_mapScriptDirectory "mapscripts"

// sets the maximum number of players that can be in the game at one time.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_maxGameClients "0"

// sets maximum number of lives for all players.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_maxlives "0"

// sets the penalty for a player after their lives have run out.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_maxlivesRespawnPenalty "0"

// set amount of time for medic to recharge.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_medicChargeTime "45000"

// set bitflags for medic behavior.
set g_medics "0"

// set self-healing delay for medic in milliseconds.
set g_medicSelfHealDelay "0"

set g_minGameClients "8"

// set various bitflags.
set g_misc "0"

// enable picture-in-picture missile cameras for projectile weapons.
set g_missileCams "0"

// adjusts the speed of movers.
set g_moverScale "1.0"

// sets the time window in milliseconds for counting consecutive revives as a multi-revive.
set g_multiReviveTime "2000"

// specifies how long a mute should last.
set g_muteTime "0"

// prevents players from attacking while spawn-invulnerable.
set g_noAttackInvul "0"

// disables class ability charge bars, giving unlimited ability usage.
set g_noCharge "0"

// disables weapon reloading, giving unlimited clip ammunition.
set g_noReload "0"

// disables team switching during a match.
set g_noTeamSwitching "0"

// Supported-client OS/architecture bitmask advertised to the server browser.
set g_oss "2047"

// sets the multiplier of throw distance for ammo and health packs.
set g_packDistance "1"

// enable/disable panzer-war game mode.
set g_panzerWar "0"

// allow players to pick up any weapon regardless of class.
set g_pickAnyWeapon "0"

// enables players to play dead.
set g_playDead "0"

// enables the use of poison syringes.
set g_poisonSyringes "0"

// enables private messaging.
set g_privateMessages "0"

// enable/disable extended prone dela.
set g_proneDelay "0"

// sets a short footer message for players disconnected as punishment.
set g_protestMessage "Visit ^/www.myserver.com^* to file a protest"

// the amount of time between Axis team respawns.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_redlimbotime "30000"

// sets the percentage of friendly fire to reflect to the attacker.
set g_reflectFriendlyFire "100"

// enables the revenge feature which announces when a player kills someone who recently killed them.
set g_revenge "0"

// controls revive spree announcement options.
set g_reviveSpreeOptions "1"

// enables persistent stats across all the maps in a campaign.
set g_saveCampaignStats "1"

// enable/disable text shortcuts.
set g_shortcuts "0"

// set player shoving distance.
set g_shove "0"

// enable/disable supression of Z-axis shoving.
set g_shoveNoZ "0"

set g_shutdownExit "0"

// set bitflags for 5th-level battle-sense skill.
set g_sk5_battle "1"

// set bitflags for 5th-level covertops skill.
set g_sk5_cvops "7"

// set bitflags for 5th-level engineer skill.
set g_sk5_eng "127"

// set bitflags for 5th-level fieldops skill.
set g_sk5_fdops "3"

// set bitflags for 5th-level light-weapons skill.
set g_sk5_lightweap "1"

// set bitflags for 5th-level medic skill.
set g_sk5_medic "243"

// set bitflags for 5th-level soldier skill.
set g_sk5_soldier "7"

// set bitflags for skills related behavior.
set g_skills "0"

// set client /kill behavior mode.
set g_slashKill "0"

// enable/disable missed client frames smoothing.
set g_smoothClients "1"

// set bitflags for server floating point value snapping.
set g_snap "7"

// enable/disable sniper-war game mode.
set g_sniperWar "0"

// set amount of time for soldier to recharge.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_soldierChargeTime "20000"

// set bitflags for soldier behavior.
set g_soldiers "0"

// sets spawn invulnerability period for players.
set g_spawnInvul "3"

// enables noclip during spawn invulnerability, allowing players to pass through teammates.
set g_spawnInvulNoClip "0"

// set bitflags for spectator actions.
set g_spectator "0"

// set spectator inactivity limit.
set g_spectatorInactivity "0"

// controls whether spectator names are visible to other players.
set g_spectatorNames "1"

// set player baseline speed.
set g_speed "320"

// sets the delay in seconds before a player can switch teams again.
set g_teamChangeDelay "0"

// set friendly-fire tolerance minimum hits.
set g_teamDamageMinHits "6"

// set friendly-fire tolerance percentage.
set g_teamDamageRestriction "0"

// force team balance.
// Starter choice / Startvorgabe; registered default / Quellcode-Default: "0"
set g_teamForceBalance "1"

// enable/disable true ping calculation.
set g_truePing "1"

// the amount of time between Allied team respawns.
set g_userAlliedRespawnTime "0"

// the amount of time between Axis team respawns.
set g_userAxisRespawnTime "0"

// SQLite user database filename. Keep the existing database when updating.
set g_userConfig "xmod.db"

set g_userTimeLimit "0"

// set maximum number of voice chats per 30 second period.
set g_voiceChatsAllowed "4"

// sets warmup period before match begins.
set g_warmup "60"

// sets the warning level at which a player is automatically banned.
set g_warnBanLevel "100"

// enables automatic decay of player warning levels over time.
set g_warnDecay "1"

// sets the warning level at which a player is automatically muted.
set g_warnMuteLevel "50"

// set server watermark used for client display.
set g_watermark "xmod"

// set amount of time before watermark begins to fade.
set g_watermarkFadeAfter "60"

// set amount of time to fade watermark.
set g_watermarkFadeTime "60"

// set bitflags for various weapons behavior.
set g_weapons "0"

// Custom weapon script directory. Must be distributed to clients; restart the map after changing.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_weaponScriptsDir ""

// enable/disable special weapons.
set g_weaponsenable "0"

// enable/disable &rtcw:name; rate of fire.
set g_wolfrof "0"

// set XP-limit action.
set g_xpCap "0"

// set XP-limit amount.
set g_xpMax "0"

// enable/disable XP-save feature.
// Latched: takes effect after restart / Wirksam nach Neustart.
set g_xpSave "0"

// set XP-save duration.
set g_xpSaveTimeout "1h"

set pmove_fixed "0"

set pmove_msec "8"

set timelimit "0"

// ==========================================================================
// Team, match and voting settings / Teams, Match und Abstimmungen
// ==========================================================================

// enable/disable allowing players to join a match in progress.
set match_latejoin "1"

// set minimum number of players required for match to begin.
set match_minplayers "4"

// enable/disable muting of spectators.
set match_mutespecs "0"

// set percentage of players required to be ready.
set match_readypercent "100"

// set maximum number of times non-referees can pause the match.
set match_timeoutcount "3"

// set duration of player-timeout.
set match_timeoutlength "180"

// enable/disable damage during warmup.
set match_warmupDamage "1"

// sets the maximum number of artillery or airstrikes per minute.
set team_maxArtillery "6"

// sets the maximum number of covert-ops per team.
set team_maxCovertOps "-1"

// sets the maximum number of engineers per team.
set team_maxEngineers "-1"

// sets the maximum number of field-ops per team.
set team_maxFieldOps "-1"

// sets the maximum number of flamethrowers per team.
set team_maxFlamers "-1"

// sets the maximum number of grenade launchers per team.
set team_maxGrenLaunchers "-1"

// sets the maximum number of landmines per team.
set team_maxLandmines "10"

// sets the maximum number of M97s per team.
set team_maxM97s "-1"

// sets the maximum number of medics per team.
set team_maxMedics "-1"

// sets the maximum number of MG42s per team.
set team_maxMG42s "-1"

// sets the maximum number of mortars per team.
set team_maxMortars "-1"

// sets maximum number of panzerfausts per team.
set team_maxPanzers "-1"

// sets maximum number of players per team.
set team_maxplayers "0"

set team_maxPPSHs "-1"

// sets the maximum number of tripmines per team.
set team_maxTripmines "3"

// enable/disable arbitrary control of teams.
set team_nocontrols "1"

// enable/disable balanced teams.
set vote_allow_balancedteams "1"

// enable/disable competition settings.
set vote_allow_comp "1"

// enable/disable friendly-fire.
set vote_allow_friendlyfire "1"

// enable/disable gametype.
set vote_allow_gametype "1"

// enable/disable generic.
set vote_allow_generic "1"

// enable/disable kick.
set vote_allow_kick "1"

// enable/disable map.
set vote_allow_map "1"

// enable/disable matchreset.
set vote_allow_matchreset "1"

// enable/disable matchrestart.
set vote_allow_matchrestart "1"

// enable/disable mutespecs.
set vote_allow_mutespecs "1"

// enable/disable muting.
set vote_allow_muting "1"

// enable/disable nextmap.
set vote_allow_nextmap "1"

// enable/disable pub.
set vote_allow_pub "1"

// enable/disable referee.
set vote_allow_referee "0"

// enable/disable shuffleteamsxp.
set vote_allow_shuffleteamsxp "1"

// allows players to call a vote to start the match.
set vote_allow_startmatch "1"

// enable/disable swapteams.
set vote_allow_swapteams "1"

// enable/disable timelimit.
set vote_allow_timelimit "0"

// enable/disable warmupdamage.
set vote_allow_warmupdamage "1"

// set maximum number of times a vote may be called.
set vote_limit "5"

// sets the minimum percentage of eligible voters that must vote for a vote to be valid.
set vote_minPercent "0"

// set percentage of votes required for it to pass.
set vote_percent "50"

// enables vote-based voting where votes are counted based on the total number of voters rather than total eligible players.
set vote_voteBased "0"

// ==========================================================================
// Omni-bot
// ==========================================================================

// Enable native Omni-bot support. Set omnibot_path to the matching bot installation.
// Starter choice / Startvorgabe; registered default / Quellcode-Default: "1"
set omnibot_enable "0"

set omnibot_flags "0"

// Directory containing the architecture-matched Omni-bot library.
set omnibot_path ""

// ==========================================================================
// JXAC
// ==========================================================================

// enables JXAC anti-tamper protection to detect modifications to the client game module.
set jxac_antiTamper "1"

// enables automatic banning of players detected cheating by JXAC.
set jxac_autoBan "0"

// enables automatic kicking of players detected cheating by JXAC.
set jxac_autoKick "1"

// sets the configuration file containing CVAR names to scan for on clients during CVAR scan cycles.
set jxac_cheatCvarFile "jxac/jxac_cvarscan.cfg"

// sets the configuration file containing the cheat signature database for JXAC module scanning.
set jxac_cheatDbFile "jxac/jxac_cheats.cfg"

// sets the configuration file containing cheat signature definitions for JXAC detection.
set jxac_cheatFile "jxac/jxac_cheats.cfg"

// enables JXAC CVAR checking to detect known cheat CVARs on clients.
set jxac_checkCvars "1"

// enables JXAC speedhack detection.
set jxac_checkSpeedhack "0"

// enables JXAC wallhack detection.
set jxac_checkWallhack "1"

// Optional legacy JXAC cvar-list file. See JXAC.md for current rule files.
set jxac_cvarFile "jxac/jxac_cvars.cfg"

// enables JXAC CVAR scanning to detect cheat CVARs injected by external programs.
set jxac_cvarScan "0"

// sets the delay in milliseconds between individual CVAR scan requests sent to the client.
set jxac_cvarScanDelay "750"

// sets the interval in milliseconds between complete CVAR scan cycles.
set jxac_cvarScanInterval "300000"

// sets the maximum number of CVAR scan warnings before a player is kicked or banned.
set jxac_cvarScanMaxWarnings "1"

// sets the initial wait time in milliseconds before JXAC begins CVAR scanning after a client connects.
set jxac_cvarScanWait "10000"

// enables the JXAC anticheat system.
set jxac_enable "1"

// Forced client cvars; reload using the documented JXAC commands after editing.
set jxac_forceCvarFile "jxac/jxac_forcecvar.cfg"

// sets the timeout in milliseconds for JXAC client heartbeats.
set jxac_heartbeatTimeout "60000"

// sets the filename for the JXAC anticheat log.
set jxac_logFile "jxac/jxac.log"

// enables JXAC module scanning to detect known cheat modules loaded by clients.
set jxac_moduleScan "1"

// sets the directory where JXAC anticheat screenshots are saved.
set jxac_screenshotPath "jxac/screenshots/"

// sets the JPEG quality for anticheat screenshots.
set jxac_screenshotQuality "85"

// ==========================================================================
// VPN blocker / VPN-Blocker
// ==========================================================================

// Connection rejection message for manually blacklisted addresses; empty uses the built-in fallback.
set g_vpnBlockerBanMessageBlacklist ""

// Connection rejection message for detected VPN/proxy addresses.
set g_vpnBlockerBanMessageVPN "VPN/proxy connections are not allowed on this server."

// Separate SQLite VPN database for IP allow/block lists and authenticated GUID exceptions; compatible with the old JAR schema.
set g_vpnBlockerDBPath "vpnblocker.sqlite"

// 0 = off; 1 = asynchronous VPN/blacklist checks. See VPN_BLOCKER.md.
set g_vpnBlockerEnabled "0"

// Authenticated admin levels ABOVE this value bypass VPN checks.
set g_vpnBlockerMaxLevel "0"

// ==========================================================================
// Lua extension cvars (read directly, not registered with a default)
// Lua-Erweiterungen (direkt gelesen, ohne registrierten Default)
// ==========================================================================
// Space-separated Lua scripts; native Antirush does not need a Lua module.
set lua_modules ""
// Optional module-signature allow-list; empty permits the configured modules.
set lua_allowedModules ""

// Host and private settings are configured once in their owning files.
// Host- und Zugangsdaten werden nur in ihrer eigenen Datei gesetzt.
// g_password: server-private.cfg
// g_shoutcastpassword: server-private.cfg
// g_vpnBlockerApiKey1: server-private.cfg
// g_vpnBlockerApiKey2: server-private.cfg
// refereePassword: server-private.cfg
// server_motd0: server.cfg
// server_motd1: server.cfg
// server_motd2: server.cfg
// server_motd3: server.cfg
// server_motd4: server.cfg
// server_motd5: server.cfg
// sv_fps: server.cfg
// sv_maxclients: server.cfg
// sv_maxRate: server.cfg
// sv_privatepassword: server-private.cfg
// URL: server.cfg

// ==========================================================================
// Read-only, engine/session state and development cvars (reference only)
// Nur lesbar, Engine-/Matchstatus und Entwicklung (nur Referenz)
// Do not enable these lines in a production configuration.
// Diese Zeilen in einer Produktionskonfiguration nicht aktivieren.
// ==========================================================================
// bot_enable "0" (engine/session state)
// cg_letterbox "0" (engine/session state)
// dedicated "0" (engine/session state)
// developer "0" (engine/session state)
// g_alliedmapxp "0" (read-only)
// g_alliedwins "0" (read-only)
// g_antilag "1" (read-only)
// g_axismapxp "0" (read-only)
// g_axiswins "0" (read-only)
// g_balancedteams "0" (read-only)
// g_currentCampaign "" (read-only)
// g_currentCampaignMap "0" (read-only)
// g_currentRound "0" (engine/session state)
// g_debugConstruct "0" (cheat/development)
// g_debugDamage "0" (cheat/development)
// g_lms_currentMatch "0" (read-only)
// g_movespeed "76" (cheat/development)
// g_needpass "0" (read-only)
// g_nextTimeLimit "0" (engine/session state)
// g_oldCampaign "" (read-only)
// g_restarted "0" (read-only)
// g_scriptDebug "0" (cheat/development)
// g_scriptDebugLevel "0" (cheat/development)
// g_scriptName "" (cheat/development)
// g_swapteams "0" (read-only)
// g_test "0" (engine/session state)
// g_xmod_buildTarget "<build-dependent>" (read-only)
// g_xmod_repoLCDate "<build-dependent>" (read-only)
// g_xmod_repoLCRev "<build-dependent>" (read-only)
// g_xmod_repoUUID "<build-dependent>" (read-only)
// g_xmod_title "<build-dependent>" (read-only)
// gamedate "<build-dependent>" (read-only)
// gamename "<build-dependent>" (read-only)
// gameState "-1" (read-only)
// mapname "" (read-only)
// mod_binary "<build-dependent>" (read-only)
// mod_url "<build-dependent>" (read-only)
// mod_version "<build-dependent>" (read-only)
// nextcampaign "" (engine/session state)
// nextmap "" (engine/session state)
// omnibot_playing "0" (read-only)
// P "" (engine/session state)
// server_autoconfig "0" (engine/session state)
// sv_cheats "" (engine/session state)
// sv_cpu "" (read-only)
// sv_mapname "" (read-only)
// sv_tempBanMessage "" (read-only)
// sv_uptime "" (read-only)
// sv_uptimeStamp "-1" (read-only)
// voteFlags "0" (read-only)
// z_serverflags "0" (engine/session state)
