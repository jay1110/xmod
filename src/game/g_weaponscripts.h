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
#define MAX_WEAPONSCRIPT_SIZE 65536

// Cvar for weapon scripts directory
extern vmCvar_t g_weaponScriptsDir;

///////////////////////////////////////////////////////////////////////////////
// Weapon Script Properties
//
// These can be set in .weap files in the "both" section
// to modify server-side weapon behavior
///////////////////////////////////////////////////////////////////////////////

// Shared definition is included by bg_public.h.
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

void G_BuildWeaponScriptInfo(int weapon, char* info, int size);
void G_ApplyWeaponProjectileOverrides(struct gentity_s* projectile, int weapon);

// Broadcast weapon scripts to all clients via server commands
void G_BroadcastWeaponScripts( void );

// Send weapon scripts to a specific client via server commands
void G_SendWeaponScripts( int clientNum );

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_G_WEAPONSCRIPTS_H
