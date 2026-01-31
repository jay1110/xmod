///////////////////////////////////////////////////////////////////////////////
//
// g_weaponscripts.cpp - Weapon Script Loading System
//
// Based on NoQuarter's weaponscripts system
// Allows server admins to customize weapon properties via .weap files
//
///////////////////////////////////////////////////////////////////////////////

#include <bgame/impl.h>
#include <game/g_weaponscripts.h>

///////////////////////////////////////////////////////////////////////////////

// Weapon script definitions for all weapons
static weaponScriptDef_t weaponScripts[WP_NUM_WEAPONS];

// Weapon filename mappings
static const char* weaponFilenames[WP_NUM_WEAPONS] = {
    "",                     // WP_NONE
    "knife",               // WP_KNIFE
    "luger",               // WP_LUGER
    "mp40",                // WP_MP40
    "grenade",             // WP_GRENADE_LAUNCHER
    "panzerfaust",         // WP_PANZERFAUST
    "flamethrower",        // WP_FLAMETHROWER
    "colt",                // WP_COLT
    "thompson",            // WP_THOMPSON
    "grenade_pineapple",   // WP_GRENADE_PINEAPPLE
    "sten",                // WP_STEN
    "syringe",             // WP_MEDIC_SYRINGE
    "ammo",                // WP_AMMO
    "arty",                // WP_ARTY
    "silencer",            // WP_SILENCER
    "dynamite",            // WP_DYNAMITE
    "",                    // WP_SMOKETRAIL
    "",                    // WP_MAPMORTAR
    "",                    // VERYBIGEXPLOSION
    "medkit",              // WP_MEDKIT
    "binoculars",          // WP_BINOCULARS
    "pliers",              // WP_PLIERS
    "smoke_marker",        // WP_SMOKE_MARKER
    "kar98",               // WP_KAR98
    "carbine",             // WP_CARBINE
    "garand",              // WP_GARAND
    "landmine",            // WP_LANDMINE
    "satchel",             // WP_SATCHEL
    "satchel_det",         // WP_SATCHEL_DET
    "tripmine",            // WP_TRIPMINE
    "smoke_bomb",          // WP_SMOKE_BOMB
    "mobile_mg42",         // WP_MOBILE_MG42
    "k43",                 // WP_K43
    "fg42",                // WP_FG42
    "",                    // WP_DUMMY_MG42
    "mortar",              // WP_MORTAR
    "",                    // WP_LOCKPICK
    "akimbo_colt",         // WP_AKIMBO_COLT
    "akimbo_luger",        // WP_AKIMBO_LUGER
    "gpg40",               // WP_GPG40
    "m7",                  // WP_M7
    "silenced_colt",       // WP_SILENCED_COLT
    "garand_scope",        // WP_GARAND_SCOPE
    "k43_scope",           // WP_K43_SCOPE
    "fg42_scope",          // WP_FG42SCOPE
    "mortar_set",          // WP_MORTAR_SET
    "adrenaline",          // WP_MEDIC_ADRENALINE
    "akimbo_silencedcolt", // WP_AKIMBO_SILENCEDCOLT
    "akimbo_silencedluger",// WP_AKIMBO_SILENCEDLUGER
    "mobile_mg42_set",     // WP_MOBILE_MG42_SET
    "poison",              // WP_POISON_SYRINGE (was poison_syringe)
    "adrenaline_share",    // WP_ADRENALINE_SHARE
    "m97",                 // WP_M97
    "poison_gas",          // WP_POISON_GAS
    "landmine_bbetty",     // WP_LANDMINE_BBETTY
    "landmine_pgas",       // WP_LANDMINE_PGAS
    "molotov",             // WP_MOLOTOV
};

///////////////////////////////////////////////////////////////////////////////

/*
==============
G_InitWeaponScripts

Initialize the weapon scripts system
==============
*/
void G_InitWeaponScripts( void )
{
    // Initialize all weapon scripts to defaults
    memset( weaponScripts, 0, sizeof(weaponScripts) );
    
    // Load weapon scripts if directory is specified
    if ( g_weaponScriptsDir.string[0] ) {
        G_LoadWeaponScripts();
    }
}

/*
==============
G_ResetWeaponScripts

Reset all weapon scripts to defaults
==============
*/
void G_ResetWeaponScripts( void )
{
    memset( weaponScripts, 0, sizeof(weaponScripts) );
}

/*
==============
G_GetWeaponScript

Get the weapon script definition for a weapon
==============
*/
weaponScriptDef_t* G_GetWeaponScript( int weapon )
{
    if ( weapon < 0 || weapon >= WP_NUM_WEAPONS ) {
        return NULL;
    }
    return &weaponScripts[weapon];
}

/*
==============
G_WeaponHasScript

Check if a weapon has a custom script loaded
==============
*/
qboolean G_WeaponHasScript( int weapon )
{
    if ( weapon < 0 || weapon >= WP_NUM_WEAPONS ) {
        return qfalse;
    }
    return weaponScripts[weapon].hasScript;
}

/*
==============
SkipWhitespace

Skip whitespace characters in a buffer
==============
*/
static const char* SkipWhitespace( const char* data )
{
    while ( *data && (*data == ' ' || *data == '\t' || *data == '\r' || *data == '\n') ) {
        data++;
    }
    return data;
}

/*
==============
ParseToken

Parse a single token from the buffer
==============
*/
static const char* ParseToken( const char* data, char* token, int tokenSize )
{
    int len = 0;
    
    data = SkipWhitespace( data );
    
    if ( !*data ) {
        token[0] = '\0';
        return NULL;
    }
    
    // Handle quoted strings
    if ( *data == '"' ) {
        data++;
        while ( *data && *data != '"' && len < tokenSize - 1 ) {
            token[len++] = *data++;
        }
        if ( *data == '"' ) {
            data++;
        }
    }
    else {
        // Parse until whitespace or special character
        while ( *data && *data != ' ' && *data != '\t' && *data != '\r' && 
                *data != '\n' && *data != '{' && *data != '}' && len < tokenSize - 1 ) {
            token[len++] = *data++;
        }
    }
    
    token[len] = '\0';
    return data;
}

/*
==============
G_ParseWeaponScriptBoth

Parse the "both" section of a weapon script
==============
*/
static const char* G_ParseWeaponScriptBoth( const char* data, weaponScriptDef_t* script )
{
    char token[256];
    int depth = 0;
    
    // Find opening brace
    data = SkipWhitespace( data );
    if ( *data != '{' ) {
        return data;
    }
    data++;
    depth = 1;
    
    while ( *data && depth > 0 ) {
        data = ParseToken( data, token, sizeof(token) );
        if ( !data ) {
            break;
        }
        
        // Handle braces
        data = SkipWhitespace( data );
        if ( *data == '{' ) {
            data++;
            depth++;
            continue;
        }
        if ( *data == '}' ) {
            data++;
            depth--;
            continue;
        }
        
        if ( !token[0] ) {
            continue;
        }
        
        // Parse properties
        if ( !Q_stricmp( token, "name" ) ) {
            data = ParseToken( data, script->name, sizeof(script->name) );
        }
        else if ( !Q_stricmp( token, "statname" ) ) {
            data = ParseToken( data, script->statname, sizeof(script->statname) );
        }
        else if ( !Q_stricmp( token, "damage" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->damage = atoi( token );
        }
        else if ( !Q_stricmp( token, "splashdamage" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->splashDamage = atoi( token );
        }
        else if ( !Q_stricmp( token, "splashdamage_radius" ) || !Q_stricmp( token, "splashRadius" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->splashRadius = atoi( token );
        }
        else if ( !Q_stricmp( token, "spread" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->spread = atoi( token );
        }
        else if ( !Q_stricmp( token, "spreadRatio" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->spreadRatio = (float)atof( token );
        }
        else if ( !Q_stricmp( token, "maxammo" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->maxAmmo = atoi( token );
        }
        else if ( !Q_stricmp( token, "maxclip" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->maxClip = atoi( token );
        }
        else if ( !Q_stricmp( token, "startammo" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->startAmmo = atoi( token );
        }
        else if ( !Q_stricmp( token, "startclip" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->startClip = atoi( token );
        }
        else if ( !Q_stricmp( token, "reloadTime" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->reloadTime = atoi( token );
        }
        else if ( !Q_stricmp( token, "fireDelayTime" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->fireDelayTime = atoi( token );
        }
        else if ( !Q_stricmp( token, "nextShotTime" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->nextShotTime = atoi( token );
        }
        else if ( !Q_stricmp( token, "maxHeat" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->maxHeat = atoi( token );
        }
        else if ( !Q_stricmp( token, "coolRate" ) ) {
            data = ParseToken( data, token, sizeof(token) );
            script->coolRate = atoi( token );
        }
        else if ( !Q_stricmp( token, "headshotWeapon" ) ) {
            script->headshotWeapon = qtrue;
        }
        else if ( !Q_stricmp( token, "bulletReflection" ) ) {
            script->bulletReflection = qtrue;
        }
        else if ( !Q_stricmp( token, "DistanceFalloff" ) ) {
            script->distanceFalloff = qtrue;
        }
        else if ( !Q_stricmp( token, "selfKillMessage" ) ) {
            data = ParseToken( data, script->selfKillMessage, sizeof(script->selfKillMessage) );
        }
        else if ( !Q_stricmp( token, "KillMessage" ) ) {
            data = ParseToken( data, script->killMessage, sizeof(script->killMessage) );
        }
        else if ( !Q_stricmp( token, "KillMessage2" ) ) {
            data = ParseToken( data, script->killMessage2, sizeof(script->killMessage2) );
        }
    }
    
    return data;
}

/*
==============
G_ParseWeaponScript

Parse a weapon script file
==============
*/
qboolean G_ParseWeaponScript( const char* filename, int weapon )
{
    fileHandle_t f;
    int len;
    char buf[MAX_WEAPONSCRIPT_SIZE];
    const char* data;
    char token[256];
    weaponScriptDef_t* script;
    
    if ( weapon < 0 || weapon >= WP_NUM_WEAPONS ) {
        return qfalse;
    }
    
    script = &weaponScripts[weapon];
    
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( len <= 0 ) {
        return qfalse;
    }
    
    if ( len >= MAX_WEAPONSCRIPT_SIZE ) {
        G_Printf( "^3WARNING: Weapon script %s is too large\n", filename );
        trap_FS_FCloseFile( f );
        return qfalse;
    }
    
    trap_FS_Read( buf, len, f );
    buf[len] = '\0';
    trap_FS_FCloseFile( f );
    
    // Parse the file
    data = buf;
    
    // Look for weaponDef
    data = ParseToken( data, token, sizeof(token) );
    if ( Q_stricmp( token, "weaponDef" ) != 0 ) {
        G_Printf( "^3WARNING: Expected 'weaponDef' in %s\n", filename );
        return qfalse;
    }
    
    // Skip opening brace
    data = SkipWhitespace( data );
    if ( *data != '{' ) {
        G_Printf( "^3WARNING: Expected '{' in %s\n", filename );
        return qfalse;
    }
    data++;
    
    // Parse sections
    while ( data && *data ) {
        data = ParseToken( data, token, sizeof(token) );
        if ( !data || !token[0] ) {
            break;
        }
        
        // Skip closing brace
        if ( token[0] == '}' ) {
            break;
        }
        
        if ( !Q_stricmp( token, "both" ) ) {
            data = G_ParseWeaponScriptBoth( data, script );
        }
        else if ( !Q_stricmp( token, "client" ) ) {
            // Skip client section - server doesn't need it
            int depth = 0;
            data = SkipWhitespace( data );
            if ( *data == '{' ) {
                data++;
                depth = 1;
                while ( *data && depth > 0 ) {
                    if ( *data == '{' ) depth++;
                    else if ( *data == '}' ) depth--;
                    data++;
                }
            }
        }
    }
    
    script->hasScript = qtrue;
    return qtrue;
}

/*
==============
G_ApplyWeaponScript

Apply weapon script settings to the ammo table
==============
*/
void G_ApplyWeaponScript( int weapon )
{
    weaponScriptDef_t* script;
    ammotable_t* ammo;
    
    if ( weapon < 0 || weapon >= WP_NUM_WEAPONS ) {
        return;
    }
    
    script = &weaponScripts[weapon];
    if ( !script->hasScript ) {
        return;
    }
    
    ammo = GetAmmoTableData( weapon );
    if ( !ammo ) {
        return;
    }
    
    // Apply properties if they were set (non-zero)
    if ( script->maxAmmo > 0 ) {
        ammo->maxammo = script->maxAmmo;
    }
    if ( script->maxClip > 0 ) {
        ammo->maxclip = script->maxClip;
    }
    if ( script->startAmmo > 0 ) {
        ammo->defaultStartingAmmo = script->startAmmo;
    }
    if ( script->startClip > 0 ) {
        ammo->defaultStartingClip = script->startClip;
    }
    if ( script->reloadTime > 0 ) {
        ammo->reloadTime = script->reloadTime;
    }
    if ( script->fireDelayTime > 0 ) {
        ammo->fireDelayTime = script->fireDelayTime;
    }
    if ( script->nextShotTime > 0 ) {
        ammo->nextShotTime = script->nextShotTime;
    }
    if ( script->maxHeat > 0 ) {
        ammo->maxHeat = script->maxHeat;
    }
    if ( script->coolRate > 0 ) {
        ammo->coolRate = script->coolRate;
    }
}

/*
==============
G_LoadWeaponScripts

Load all weapon scripts from the configured directory
If a weapon script is not found in the custom directory, fall back to "weapons/" folder
==============
*/
void G_LoadWeaponScripts( void )
{
    char filename[MAX_WEAPONSCRIPT_PATH];
    int i;
    int loadedCustom = 0;
    int loadedFallback = 0;
    qboolean useCustomDir = (qboolean)(g_weaponScriptsDir.string[0] != '\0');
    qboolean useFallback;
    const char* customDir;
    const char* fallbackDir = "weapons";
    
    // Determine directories to use
    if ( useCustomDir ) {
        customDir = g_weaponScriptsDir.string;
        // Fallback is enabled if custom dir is different from standard "weapons"
        useFallback = (qboolean)(Q_stricmp( customDir, fallbackDir ) != 0);
    } else {
        // No custom dir specified, use standard "weapons" folder as primary
        customDir = fallbackDir;
        useFallback = qfalse;
    }
    
    G_Printf( "Loading weapon scripts from '%s'...\n", customDir );
    if ( useFallback ) {
        G_Printf( "  (fallback to '%s/' if not found)\n", fallbackDir );
    }
    
    // Reset all scripts first
    G_ResetWeaponScripts();
    
    // Try to load a script for each weapon
    for ( i = 0; i < WP_NUM_WEAPONS; i++ ) {
        qboolean scriptLoaded = qfalse;
        
        // Skip weapons without a filename
        if ( !weaponFilenames[i][0] ) {
            continue;
        }
        
        // Try to load from primary directory
        Com_sprintf( filename, sizeof(filename), "%s/%s.weap", 
                     customDir, weaponFilenames[i] );
        
        if ( G_ParseWeaponScript( filename, i ) ) {
            G_ApplyWeaponScript( i );
            G_Printf( "  Loaded: %s\n", filename );
            loadedCustom++;
            scriptLoaded = qtrue;
        }
        
        // If not loaded and fallback is enabled, try the fallback directory
        if ( !scriptLoaded && useFallback ) {
            Com_sprintf( filename, sizeof(filename), "%s/%s.weap", 
                         fallbackDir, weaponFilenames[i] );
            
            if ( G_ParseWeaponScript( filename, i ) ) {
                G_ApplyWeaponScript( i );
                G_Printf( "  Loaded (fallback): %s\n", filename );
                loadedFallback++;
            }
        }
    }
    
    // Print summary
    if ( useFallback && loadedFallback > 0 ) {
        G_Printf( "Loaded %d weapon script(s) from '%s', %d from fallback '%s'\n", 
                  loadedCustom, customDir, loadedFallback, fallbackDir );
    } else {
        G_Printf( "Loaded %d weapon script(s)\n", loadedCustom + loadedFallback );
    }
    
    // Broadcast weapon script data to clients via configstrings
    G_BroadcastWeaponScripts();
}

/*
==============
G_BroadcastWeaponScripts

Send weapon script data (name, killMessage, killMessage2) to all clients via configstrings
==============
*/
void G_BroadcastWeaponScripts( void )
{
    char cs[MAX_INFO_STRING];
    int i;
    weaponScriptDef_t* script;
    
    for ( i = 0; i < WP_NUM_WEAPONS; i++ ) {
        script = &weaponScripts[i];
        
        // Clear the configstring
        cs[0] = '\0';
        
        if ( script->hasScript ) {
            // Build configstring with weapon script data
            // Format: n\<name>\k\<killMessage>\l\<killMessage2>\s\<selfKillMessage>
            if ( script->name[0] ) {
                Info_SetValueForKey( cs, "n", script->name );
            }
            if ( script->killMessage[0] ) {
                Info_SetValueForKey( cs, "k", script->killMessage );
            }
            if ( script->killMessage2[0] ) {
                Info_SetValueForKey( cs, "l", script->killMessage2 );
            }
            if ( script->selfKillMessage[0] ) {
                Info_SetValueForKey( cs, "s", script->selfKillMessage );
            }
        }
        
        // Set the configstring for this weapon
        trap_SetConfigstring( CS_WEAPONSCRIPTS + i, cs );
    }
}
