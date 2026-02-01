#ifndef GAME_G_WEAPONSCRIPTS_H
#define GAME_G_WEAPONSCRIPTS_H

///////////////////////////////////////////////////////////////////////////////
//
// g_weaponscripts.h - Weapon Script Loading System
//
// Based on NoQuarter's weaponscripts system
// Allows server admins to customize weapon properties via .weap files
//
///////////////////////////////////////////////////////////////////////////////

#include <bgame/q_shared.h>

// Maximum path length for weapon scripts directory
#define MAX_WEAPONSCRIPT_PATH 256

// Maximum length of weapon script file
#define MAX_WEAPONSCRIPT_SIZE 8192

// Cvar for weapon scripts directory
extern vmCvar_t g_weaponScriptsDir;

///////////////////////////////////////////////////////////////////////////////
// Weapon Script Properties
//
// These can be set in .weap files in the "both" section
// to modify server-side weapon behavior
///////////////////////////////////////////////////////////////////////////////

typedef struct weaponScriptDef_s {
    // Basic weapon info
    char    name[64];           // Display name
    char    statname[64];       // Stats display name
    
    // Damage properties
    int     damage;             // Base damage per hit
    int     splashDamage;       // Splash damage amount
    int     splashRadius;       // Splash damage radius
    
    // Spread properties  
    int     spread;             // Base spread value
    float   spreadRatio;        // Spread ratio multiplier
    
    // Ammo properties
    int     maxAmmo;            // Maximum ammo capacity
    int     maxClip;            // Maximum clip size
    int     startAmmo;          // Starting ammo
    int     startClip;          // Starting clip
    
    // Timing properties
    int     reloadTime;         // Reload time in ms
    int     fireDelayTime;      // Fire delay in ms
    int     nextShotTime;       // Time between shots in ms
    
    // Heat properties
    int     maxHeat;            // Maximum heat before overheat
    int     coolRate;           // Cooling rate
    
    // Flags
    qboolean headshotWeapon;    // Can this weapon headshot?
    qboolean bulletReflection;  // Does this weapon reflect bullets?
    qboolean distanceFalloff;   // Does damage fall off with distance?
    
    // Kill messages
    char    selfKillMessage[128];   // Message when player kills self
    char    killMessage[128];       // Kill message part 1
    char    killMessage2[128];      // Kill message part 2
    
    // Set to qtrue if this weapon has a custom script loaded
    qboolean hasScript;
} weaponScriptDef_t;

///////////////////////////////////////////////////////////////////////////////
// Function Declarations
///////////////////////////////////////////////////////////////////////////////

// Initialize weapon scripts system
void G_InitWeaponScripts( void );

// Load weapon scripts from directory
void G_LoadWeaponScripts( void );

// Get weapon script definition for a weapon
weaponScriptDef_t* G_GetWeaponScript( int weapon );

// Check if weapon has a custom script loaded
qboolean G_WeaponHasScript( int weapon );

// Parse a single weapon script file
qboolean G_ParseWeaponScript( const char* filename, int weapon );

// Apply weapon script to ammo table
void G_ApplyWeaponScript( int weapon );

// Reset all weapon scripts to defaults
void G_ResetWeaponScripts( void );

// Broadcast weapon scripts to clients via configstrings
void G_BroadcastWeaponScripts( void );

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_G_WEAPONSCRIPTS_H
