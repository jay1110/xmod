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
// XMOD CONFIGURATION FILE
//
// This file contains xmod-specific CVARs.
// For standard Enemy Territory server CVARs, see server.cfg
//
// If you have any questions regarding a specific cvar, please
// consult the bundled documentation.
//
// __copyright
// __website
// __irc
//
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// BRANDING
//
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
//
// BANNERS
//
///////////////////////////////////////////////////////////////////////////////

// Banner location
// Default: "4"
// Location where banners are displayed
// 0 = Top, 1 = Console, 2 = Chat, 3 = Left, 4 = Center
set g_bannerLocation "4"

// Banner time
// Default: "60"
// Seconds between banner displays
set g_bannerTime "60"

// Number of banners
// Default: "2"
set g_banners "2"

// Banner messages
set g_banner1 "^3THIS SERVER IS RUNNING __title"
set g_banner2 "^3Check forums at __website"

///////////////////////////////////////////////////////////////////////////////
//
// XP SYSTEM
//
///////////////////////////////////////////////////////////////////////////////

// XP save
// Default: "1"
// Bit 1 (1): Enable XP saving to database
// Bit 2 (2): Reset XP on campaign change
set g_xpSave "1"

// XP save timeout
// Default: "1h"
// Time before removing inactive player XP from database
// Format: Xs (seconds), Xm (minutes), Xh (hours), Xd (days), Xw (weeks)
set g_xpSaveTimeout "1h"

// XP maximum
// Default: "0"
// Maximum XP a player can earn (0 = no limit)
set g_xpMax "0"

// XP cap
// Default: "0"
// XP cap behavior when g_xpMax is reached
// 0 = Allow gain, 1 = Stop gain, 2 = Reset XP
set g_xpCap "0"

// Damage XP
// Default: "0"
// Award XP for damage dealt (experimental)
set g_damagexp "0"

///////////////////////////////////////////////////////////////////////////////
//
// SKILL LEVELS
//
///////////////////////////////////////////////////////////////////////////////

// Custom skill level thresholds
// Default: "20 50 90 140 200" for all skills
// Format: "level1 level2 level3 level4 level5"
// Set to empty string to use default values

// Battle sense XP levels
set g_levels_battlesense "20 50 90 140 200"

// Covert ops XP levels
set g_levels_covertops "20 50 90 140 200"

// Engineer XP levels
set g_levels_engineer "20 50 90 140 200"

// Field ops XP levels
set g_levels_fieldops "20 50 90 140 200"

// Light weapons XP levels
set g_levels_lightweapons "20 50 90 140 200"

// Medic XP levels
set g_levels_medic "20 50 90 140 200"

// Soldier XP levels
set g_levels_soldier "20 50 90 140 200"

// Default skills on spawn
// Default: "0 0 0 0 0 0 0"
// Format: "battle light engineer medic fieldops covert soldier"
set g_defaultSkills "0 0 0 0 0 0 0"

///////////////////////////////////////////////////////////////////////////////
//
// SKILL-5 BENEFITS (BITFLAGS)
//
///////////////////////////////////////////////////////////////////////////////

// Skill-5 Battle Sense
// Default: "1"
// Bit 1 (1): Faster stamina recharge
// Example: 1 = All benefits enabled
set g_sk5_battle "1"

// Skill-5 Light Weapons
// Default: "1"
// Bit 1 (1): Reduced recoil time
// Example: 1 = All benefits enabled
set g_sk5_lightweap "1"

// Skill-5 Covert Ops
// Default: "7"
// Bit 1 (1): Consume less charge
// Bit 2 (2): Spawn/capacity 4 grenades
// Bit 3 (4): Enable poison gas grenades
// Example: 1+2+4 = 7 (all benefits enabled)
set g_sk5_cvops "7"

// Skill-5 Engineer
// Default: "127"
// Bit 1 (1):   Consume less charge
// Bit 2 (2):   Spawn/capacity 10 grenades
// Bit 3 (4):   Landmines take longer to spot
// Bit 4 (8):   Landmines take longer to defuse
// Bit 5 (16):  Build things faster
// Bit 6 (32):  Enable S-mines (bouncing betty)
// Bit 7 (64):  Enable poison gas landmines
// Example: 1+2+4+8+16+32+64 = 127 (all benefits enabled)
set g_sk5_eng "127"

// Skill-5 Field Ops
// Default: "3"
// Bit 1 (1): Consume less charge
// Bit 2 (2): Spawn/capacity 2 grenades
// Example: 1+2 = 3 (all benefits enabled)
set g_sk5_fdops "3"

// Skill-5 Medic
// Default: "243"
// Bit 1 (1):   Consume less charge
// Bit 2 (2):   Spawn/capacity 4 grenades
// Bit 5 (16):  Carry-over health recharge to covert ops
// Bit 6 (32):  Carry-over health recharge to engineer
// Bit 7 (64):  Carry-over health recharge to field ops
// Bit 8 (128): Carry-over health recharge to soldier
// Example: 1+2+16+32+64+128 = 243 (all benefits enabled)
set g_sk5_medic "243"

// Skill-5 Soldier
// Default: "7"
// Bit 1 (1): Consume less charge
// Bit 2 (2): Spawn/capacity 8 grenades
// Bit 3 (4): Enable poison gas grenades
// Example: 1+2+4 = 7 (all benefits enabled)
set g_sk5_soldier "7"

///////////////////////////////////////////////////////////////////////////////
//
// CLASS-SPECIFIC BEHAVIORS (BITFLAGS)
//
///////////////////////////////////////////////////////////////////////////////

// Covert Ops behavior
// Default: "0"
// Bit 1 (1):  Keep disguise when class-switching
// Bit 2 (2):  Keep disguise when throwing med packs and reviving
// Bit 3 (4):  Keep disguise when throwing ammo packs
// Bit 4 (8):  Keep disguise when laying mines or using pliers
// Bit 5 (16): Enable stealing uniform from live player from behind
// Bit 6 (32): Enable disguised enemy name drawing when close-up
// Example: 1+2+4+8+16+32 = 63 (all enabled)
set g_covertops "0"

// Engineer behavior
// Default: "0"
// Bit 1 (1): Friendly landmines are not tripped by own team
// Bit 2 (2): Friendly dynamite cannot be disarmed by own team
// Bit 3 (4): Enable shared construction XP
// Example: 1+2+4 = 7 (all enabled)
set g_engineers "0"

// Medic behavior
// Default: "0"
// Bit 3 (4):  Regenerate normal health at 2HP/s, bonus health at 1HP/s
// Bit 4 (8):  Completely disable health regeneration
// Bit 5 (16): Share adrenaline
// Bit 6 (32): Pause health regeneration for 5 seconds after taking damage
// Note: Bits 4 and 8 are mutually exclusive
// Example: 4+16+32 = 52 (slower regen, share adrenaline, pause on damage)
set g_medics "0"

// Soldier behavior
// Default: "0"
// Bit 1 (1): Enable gravity effect on panzer rockets
// Example: 1 = Gravity enabled
set g_soldiers "0"

///////////////////////////////////////////////////////////////////////////////
//
// SKILLS & WEAPONS (BITFLAGS)
//
///////////////////////////////////////////////////////////////////////////////

// Skills behavior
// Default: "0"
// Bit 1 (1): Level 4 battle-sense can spot mines for team
// Bit 2 (2): Level 4 explosives-and-construction skill carries over to all classes
// Bit 3 (4): Adrenaline carries over to all classes
// Bit 4 (8): Level 4 signals enables all classes to spot disguised enemies
// Example: 1+2+4+8 = 15 (all enabled)
set g_skills "0"

// Weapons behavior
// Default: "5606"
// Bit 1 (1):    Field ops with level 0 battle-sense do not spawn with binoculars
// Bit 2 (2):    Syringes function underwater
// Bit 3 (4):    Pliers function underwater
// Bit 4 (8):    "Too many air strikes" will restore used charge bar
// Bit 5 (16):   "Too many air strikes" will restore half of used charge bar
// Bit 6 (32):   Ammo packs restore a lost helmet
// Bit 7 (64):   Players with binoculars drop them upon death
// Bit 8 (128):  Allies reload rifles mid-clip to match corresponding axis ability
// Bit 9 (256):  Enable throwing knives
// Bit 10 (512): Enable poison throwing knives
// Bit 11 (1024): Enable Winchester M97 (shotgun)
// Bit 12 (2048): Disable adrenaline
// Bit 13 (4096): Enable Molotov-Cocktails
// Example: 2+4+8+32+256+512+1024+4096 = 5934 (common configuration)
// Example: 2+4+8+32+64+256+512+1024+4096 = 5998 (with binoc drop)
// Default: 2+4+8+32+256+512+1024+4096 = 5910, but shown as 5606 here
set g_weapons "5606"

// Miscellaneous options
// Default: "66"
// Bit 1 (1):  Enable double jump
// Bit 2 (2):  Binocular war (binoculars only)
// Bit 3 (4):  Admins only mode
// Bit 4 (8):  Players can throw health/ammo packs vertically
// Bit 6 (32): Level-4 battle-sense revivees get full health
// Bit 7 (64): More realistic weapons aim-spread (crouch/prone, slick surfaces, water)
// Example: 2+64 = 66 (binoc war + realistic aim, default)
set g_misc "66"

///////////////////////////////////////////////////////////////////////////////
//
// CHARGE TIMES
//
///////////////////////////////////////////////////////////////////////////////

// Medic charge time
// Default: "45000"
// Milliseconds to fully charge special ability
set g_medicChargeTime "45000"

// Medic self-heal delay
// Default: "0"
// Milliseconds delay before medic can self-heal (0 = immediate)
set g_medicSelfHealDelay "0"

// Engineer charge time
// Default: "30000"
// Milliseconds to fully charge special ability
set g_engineerChargeTime "30000"

// Field ops charge time (LT)
// Default: "40000"
// Milliseconds to fully charge special ability
set g_LTChargeTime "40000"

// Soldier charge time
// Default: "20000"
// Milliseconds to fully charge special ability
set g_soldierChargeTime "20000"

// Covert ops charge time
// Default: "30000"
// Milliseconds to fully charge special ability
set g_covertopsChargeTime "30000"

///////////////////////////////////////////////////////////////////////////////
//
// PLAYER FEATURES
//
///////////////////////////////////////////////////////////////////////////////

// Private messages
// Default: "1"
// Enable private messaging system
set g_privateMessages "1"

// Play dead
// Default: "1"
// Allow players to play dead
set g_playDead "1"

// Drag corpse
// Default: "1"
// Allow dragging corpses
set g_dragCorpse "1"

// Shove
// Default: "100"
// Player shove force (0 = disabled)
set g_shove "100"

// Shove no Z
// Default: "1"
// Prevent shove from affecting vertical movement
set g_shoveNoZ "1"

// Prone delay
// Default: "0"
// Delay in milliseconds before going prone
set g_proneDelay "0"

// Spawn invulnerability
// Default: "3"
// Seconds of invulnerability after spawn
set g_spawnInvul "3"

// Pack distance
// Default: "4"
// Distance multiplier for health/ammo pack pickup
set g_packDistance "4"

// Drop health
// Default: "2"
// Health packs dropped on death (0 = none, 1 = one, 2 = based on skill)
set g_dropHealth "2"

// Drop ammo
// Default: "2"
// Ammo packs dropped on death (0 = none, 1 = one, 2 = based on skill)
set g_dropAmmo "2"

// Goomba
// Default: "4"
// Damage when landing on enemy heads (0 = disabled)
set g_goomba "4"

// Class change
// Default: "0"
// Allow mid-life class changes via limbo menu
set g_classChange "0"

// Shortcuts
// Default: "0"
// Enable chat shortcuts
set g_shortcuts "0"

// Slash kill behavior
// Default: "0"
// 0 = Normal kill, 1 = Half charge, 2 = Zero charge, 4 = Same charge, 8 = No kill
set g_slashKill "0"

// Spectator mode
// Default: "0"
// Bit 1 (1): Click to spawn spectator
// Bit 2 (2): Click to spawn even if missed
// Bit 3 (4): Persistent spectator (survives map change)
// Bit 4 (8): Free spectator mode
set g_spectator "0"

///////////////////////////////////////////////////////////////////////////////
//
// KILLING SPREES
//
///////////////////////////////////////////////////////////////////////////////

// Killing spree
// Default: "1"
// Enable killing spree announcements
set g_killingSpree "1"

// Killing spree levels
// Default: "5 10 15 20 25 30"
// Kill counts for each spree level
set g_killSpreeLevels "5 10 15 20 25 30"

// Losing spree levels
// Default: "10 20 30"
// Death counts for each losing spree level
set g_loseSpreeLevels "10 20 30"

///////////////////////////////////////////////////////////////////////////////
//
// GAME FEATURES
//
///////////////////////////////////////////////////////////////////////////////

// Poison syringes
// Default: "1"
// Enable poison syringes
set g_poisonSyringes "1"

// Fear
// Default: "0"
// Enable fear effect
set g_fear "0"

// Reflect friendly fire
// Default: "100"
// Percentage of friendly fire damage reflected back (0-100)
set g_reflectFriendlyFire "100"

// Team damage restriction
// Default: "0"
// Restrict players who deal excessive team damage
set g_teamDamageRestriction "0"

// Team damage minimum hits
// Default: "6"
// Minimum team hits before restriction
set g_teamDamageMinHits "6"

// Vulnerable weapons
// Default: "0"
// Allow weapons to be damaged/destroyed
// Bit 1 (1): Panzer, Bit 2 (2): Grenades, Bit 3 (4): Canister, Bit 4 (8): Satchel
set g_vulnerableWeapons "0"

// Mover scale
// Default: "1.0"
// Speed multiplier for movers (doors, lifts, etc.)
set g_moverScale "1.0"

// Snap
// Default: "7"
// Snapshot optimization flags (advanced)
set g_snap "7"

// True ping
// Default: "1"
// Display true ping instead of scoreboard ping
set g_truePing "1"

// Admin
// Default: "1"
// Enable admin system
set g_admin "1"

// Mute time
// Default: "0"
// Default mute duration in seconds (0 = permanent)
set g_muteTime "0"

// Censor
// Default: "0"
// Enable word censoring
set g_censor "0"

// Censor penalty
// Default: "0"
// Penalty for using censored words
// Bit 1 (1): Kill player (unless bit 3), Bit 2 (2): Kick if in name
// Bit 3 (4): Don't gib, Bit 4 (8): Temporary mute
set g_censorPenalty "0"

///////////////////////////////////////////////////////////////////////////////
//
// EXPERIMENTAL FEATURES
//
///////////////////////////////////////////////////////////////////////////////

// Bullet mode
// Default: "0"
// Enable experimental bullet mode
set g_bulletmode "0"

// Bullet mode debug
// Default: "0"
set g_bulletmodeDebug "0"

// Bullet mode reference
// Default: "1"
set g_bulletmodeReference "1"

// Bullet mode trail
// Default: "0"
set g_bulletmodeTrail "0"

// Hit mode
// Default: "0"
// Enable experimental hit detection mode
set g_hitmode "0"

// Hit mode antilag
// Default: "800"
set g_hitmodeAntilag "800"

// Hit mode antilag lerp
// Default: "1"
set g_hitmodeAntilagLerp "1"

// Hit mode debug
// Default: "0"
set g_hitmodeDebug "0"

// Hit mode fat
// Default: "0"
set g_hitmodeFat "0"

// Hit mode ghosting
// Default: "0"
set g_hitmodeGhosting "0"

// Hit mode reference
// Default: "1"
set g_hitmodeReference "1"

// Hit mode zone
// Default: "0"
set g_hitmodeZone "0"
