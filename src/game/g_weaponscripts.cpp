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
#include <game/g_xmod.h>
#include <bgame/numeric_text.h>

///////////////////////////////////////////////////////////////////////////////

// Weapon script definitions for all weapons
// The same overrides are used by server gameplay and client prediction.
#define weaponScripts bg_weaponScripts

// Weapon filename mappings
static const char* weaponFilenames[WP_NUM_WEAPONS] = {
    "",                        // WP_NONE
    "knife",                   // WP_KNIFE
    "luger",                   // WP_LUGER
    "mp40",                    // WP_MP40
    "grenade",                 // WP_GRENADE_LAUNCHER
    "panzerfaust",             // WP_PANZERFAUST
    "flamethrower",            // WP_FLAMETHROWER
    "colt",                    // WP_COLT
    "thompson",                // WP_THOMPSON
    "pineapple",               // WP_GRENADE_PINEAPPLE (NQ: pineapple.weap)
    "sten",                    // WP_STEN
    "syringe",                 // WP_MEDIC_SYRINGE
    "ammopack",                // WP_AMMO (NQ: ammopack.weap)
    "arty",                    // WP_ARTY
    "silenced_luger",          // WP_SILENCER (NQ: silenced_luger.weap)
    "dynamite",                // WP_DYNAMITE
    "",                        // WP_SMOKETRAIL
    "mapmortar",                // WP_MAPMORTAR
    "",                        // VERYBIGEXPLOSION
    "medpack",                 // WP_MEDKIT (NQ: medpack.weap)
    "binocs",                  // WP_BINOCULARS (NQ: binocs.weap)
    "pliers",                  // WP_PLIERS
    "smokemarker",             // WP_SMOKE_MARKER (NQ: smokemarker.weap)
    "kar98",                   // WP_KAR98
    "m1_garand",               // WP_CARBINE (NQ: m1_garand.weap - Engineer rifle grenade)
    "m1_garand_s",             // WP_GARAND (NQ: m1_garand_s.weap - Sniper base, uses both)
    "landmine",                // WP_LANDMINE
    "satchel",                 // WP_SATCHEL
    "satchel_det",             // WP_SATCHEL_DET
    "tripmine",                // WP_TRIPMINE
    "smokegrenade",            // WP_SMOKE_BOMB (NQ: smokegrenade.weap)
    "mg42",                    // WP_MOBILE_MG42 (NQ: mg42.weap)
    "k43",                     // WP_K43 (NQ: k43.weap - CovertOps base, uses both)
    "fg42",                    // WP_FG42 (NQ: fg42.weap - base, uses both)
    "",                        // WP_DUMMY_MG42
    "mortar",                  // WP_MORTAR
    "",                        // WP_LOCKPICK
    "akimbo_colt",             // WP_AKIMBO_COLT
    "akimbo_luger",            // WP_AKIMBO_LUGER
    "gpg40",                   // WP_GPG40
    "m7",                      // WP_M7
    "silenced_colt",           // WP_SILENCED_COLT
    "m1_garand_s",             // WP_GARAND_SCOPE (shares m1_garand_s.weap, uses both_altweap)
    "k43",                     // WP_K43_SCOPE (shares k43.weap, uses both_altweap)
    "fg42",                    // WP_FG42SCOPE (shares fg42.weap, uses both_altweap)
    "mortar_set",              // WP_MORTAR_SET
    "adrenaline",              // WP_MEDIC_ADRENALINE
    "akimbo_silenced_colt",    // WP_AKIMBO_SILENCEDCOLT (NQ: akimbo_silenced_colt.weap)
    "akimbo_silenced_luger",   // WP_AKIMBO_SILENCEDLUGER (NQ: akimbo_silenced_luger.weap)
    "mg42",                    // WP_MOBILE_MG42_SET (shares mg42.weap, uses both_altweap)
    "poison",                  // WP_POISON_SYRINGE
    "adrenaline_share",        // WP_ADRENALINE_SHARE
    "m97",                     // WP_M97
    "poison_gas",              // WP_POISON_GAS
    "landmine_bbetty",         // WP_LANDMINE_BBETTY
    "landmine_pgas",           // WP_LANDMINE_PGAS
    "molotov",                 // WP_MOLOTOV
    "bombax",                  // WP_BOMB
    "ppsh",                    // WP_PPSH
    "bomb",                    // WP_BOMB_ALLIES
};


namespace {
class ScriptLexer {
public:
    const char* cursor;
    bool failed;
    explicit ScriptLexer(const char* text) : cursor(text), failed(false) {}
    bool next(char* out, unsigned int size) {
        for (;;) {
            while (*cursor && (unsigned char)*cursor <= ' ') ++cursor;
            if (cursor[0] == '/' && cursor[1] == '/') {
                while (*cursor && *cursor != '\n') ++cursor;
                continue;
            }
            if (cursor[0] == '/' && cursor[1] == '*') {
                cursor += 2;
                while (*cursor && !(cursor[0] == '*' && cursor[1] == '/')) ++cursor;
                if (!*cursor) { failed = true; return false; }
                cursor += 2; continue;
            }
            break;
        }
        out[0] = 0;
        if (!*cursor) return false;
        unsigned int length = 0;
        if (*cursor == '{' || *cursor == '}') {
            out[0] = *cursor++; out[1] = 0; return true;
        }
        bool quoted = *cursor == '"';
        if (quoted) ++cursor;
        while (*cursor && (quoted ? *cursor != '"' :
                ((unsigned char)*cursor > ' ' && *cursor != '{' && *cursor != '}' &&
                 !(cursor[0] == '/' && (cursor[1] == '/' || cursor[1] == '*'))))) {
            if (length + 1 >= size) { failed = true; return false; }
            out[length++] = *cursor++;
        }
        out[length] = 0;
        if (quoted) {
            if (*cursor != '"') { failed = true; return false; }
            ++cursor;
        }
        return true;
    }
    bool expect(const char* expected) {
        char token[256];
        return next(token, sizeof(token)) && !Q_stricmp(token, expected);
    }
    bool skipSection() {
        if (!expect("{")) return false;
        int depth = 1;
        char token[256];
        while (depth && next(token, sizeof(token))) {
            if (!strcmp(token, "{")) ++depth;
            if (!strcmp(token, "}")) --depth;
        }
        return !depth && !failed;
    }
};

struct ScriptProperty { const char* name; weaponScriptField_t field; };
const ScriptProperty properties[] = {
    {"damage", WSF_DAMAGE}, {"splashdamage", WSF_SPLASH_DAMAGE},
    {"splashdamage_radius", WSF_SPLASH_RADIUS}, {"splashRadius", WSF_SPLASH_RADIUS},
    {"spread", WSF_SPREAD}, {"spreadRatio", WSF_SPREAD_RATIO},
    {"maxammo", WSF_MAXAMMO}, {"maxclip", WSF_MAXCLIP},
    {"startammo", WSF_STARTAMMO}, {"startclip", WSF_STARTCLIP},
    {"reloadTime", WSF_RELOADTIME}, {"fireDelayTime", WSF_FIREDELAYTIME},
    {"nextShotTime", WSF_NEXTSHOTTIME}, {"maxHeat", WSF_MAXHEAT},
    {"coolRate", WSF_COOLRATE}, {"headshotWeapon", WSF_HEADSHOT},
    {"bulletReflection", WSF_REFLECTION}, {"DistanceFalloff", WSF_FALLOFF},
    {"GibbingWeapon", WSF_GIBBING}, {"movementSpeedScale", WSF_MOVESPEED},
    {"uses", WSF_USES}
};

bool parseBoth(ScriptLexer& lexer, weaponScriptDef_t& script, const char* filename) {
    if (!lexer.expect("{")) return false;
    char key[256], token[256];
    while (lexer.next(key, sizeof(key))) {
        if (!strcmp(key, "}")) { script.hasScript = qtrue; return true; }
        char* text = NULL;
        unsigned int size = 0;
        if (!Q_stricmp(key, "name")) { text = script.name; size = sizeof(script.name); }
        else if (!Q_stricmp(key, "statname")) { text = script.statname; size = sizeof(script.statname); }
        else if (!Q_stricmp(key, "KillMessage")) { text = script.killMessage; size = sizeof(script.killMessage); }
        else if (!Q_stricmp(key, "KillMessage2")) { text = script.killMessage2; size = sizeof(script.killMessage2); }
        else if (!Q_stricmp(key, "selfKillMessage")) { text = script.selfKillMessage; size = sizeof(script.selfKillMessage); }
        if (text) {
            if (!lexer.next(token, sizeof(token)) || !strcmp(token, "{") || !strcmp(token, "}")) return false;
            if (strlen(token) >= size) return false;
            // These characters cannot be represented safely in an info/server-command string.
            for (char* p = token; *p; ++p)
                if (*p == '"' || *p == '\\' || *p == ';' || (unsigned char)*p < 32) *p = ' ';
            Q_strncpyz(text, token, size);
            continue;
        }
        int field = -1;
        for (unsigned int i = 0; i < sizeof(properties) / sizeof(properties[0]); ++i)
            if (!Q_stricmp(key, properties[i].name)) { field = properties[i].field; break; }
        if (field < 0) {
            G_Printf("^3WARNING: Unsupported weapon property '%s' in %s\n", key, filename);
            // Skip one value (or a nested block) without accidentally interpreting it as a property.
            const char* saved = lexer.cursor;
            if (!lexer.next(token, sizeof(token))) return false;
            if (!strcmp(token, "}")) { lexer.cursor = saved; continue; }
            if (!strcmp(token, "{")) { lexer.cursor = saved; if (!lexer.skipSection()) return false; }
            continue;
        }
        const char* saved = lexer.cursor;
        if (!lexer.next(token, sizeof(token))) return false;
        double value;
        if (field >= WSF_HEADSHOT && field <= WSF_GIBBING) {
            if (!Q_stricmp(token,"yes") || !Q_stricmp(token,"true") || !strcmp(token,"1")) value = 1;
            else if (!Q_stricmp(token,"no") || !Q_stricmp(token,"false") || !strcmp(token,"0")) value = 0;
            else {
                // Old files use presence-only flags. Keep the next property for the next iteration.
                bool property = !strcmp(token,"}") || !Q_stricmp(token,"name") || !Q_stricmp(token,"statname") ||
                    !Q_stricmp(token,"KillMessage") || !Q_stricmp(token,"KillMessage2") || !Q_stricmp(token,"selfKillMessage");
                for (unsigned int i = 0; i < sizeof(properties) / sizeof(properties[0]); ++i)
                    if (!Q_stricmp(token, properties[i].name)) property = true;
                if (!property) return false;
                lexer.cursor = saved; value = 1;
            }
        } else {
            if (!XmodParseFiniteDecimal(token, value) || !(value >= 0 && value <= 65535)) return false;
            if (field != WSF_SPREAD_RATIO && field != WSF_MOVESPEED && value != (int)value) return false;
            if ((field == WSF_SPREAD_RATIO || field == WSF_MOVESPEED) && value > 100) return false;
        }
        script.present |= 1u << field;
        script.values[field] = (float)value;
    }
    return false;
}

int altWeaponFor(int weapon) {
    switch (weapon) {
        case WP_GARAND: return WP_GARAND_SCOPE;
        case WP_K43: return WP_K43_SCOPE;
        case WP_FG42: return WP_FG42SCOPE;
        case WP_MOBILE_MG42: return WP_MOBILE_MG42_SET;
        case WP_MORTAR: return WP_MORTAR_SET;
        default: return -1;
    }
}
bool isAltWeapon(int weapon) {
    return weapon == WP_GARAND_SCOPE || weapon == WP_K43_SCOPE || weapon == WP_FG42SCOPE ||
        weapon == WP_MOBILE_MG42_SET || weapon == WP_MORTAR_SET;
}
} // namespace

void G_ResetWeaponScripts() { BG_InitWeaponScriptState(); }

void G_InitWeaponScripts() {
    G_ResetWeaponScripts();
    if (g_weaponScriptsDir.string[0]) G_LoadWeaponScripts();
}

weaponScriptDef_t* G_GetWeaponScript(int weapon) {
    return weapon > WP_NONE && weapon < WP_NUM_WEAPONS ? &weaponScripts[weapon] : NULL;
}
qboolean G_WeaponHasScript(int weapon) {
    weaponScriptDef_t* script = G_GetWeaponScript(weapon);
    return script ? script->hasScript : qfalse;
}

qboolean G_ParseWeaponScript(const char* filename, int weapon) {
    if (weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS) return qfalse;
    fileHandle_t file = 0;
    int len = trap_FS_FOpenFile(filename, &file, FS_READ);
    if (len < 0 || !file) return qfalse;
    if (len == 0 || len >= MAX_WEAPONSCRIPT_SIZE) {
        trap_FS_FCloseFile(file);
        G_Printf("^3WARNING: Empty or oversized weapon script %s\n", filename);
        return qfalse;
    }
    char buffer[MAX_WEAPONSCRIPT_SIZE];
    trap_FS_Read(buffer, len, file);
    trap_FS_FCloseFile(file);
    buffer[len] = 0;
    const char* start = buffer;
    if (len >= 3 && (unsigned char)start[0] == 0xef && (unsigned char)start[1] == 0xbb && (unsigned char)start[2] == 0xbf) start += 3;
    ScriptLexer lexer(start);
    weaponScriptDef_t parsed = {}, alternate = {};
    bool valid = lexer.expect("weaponDef") && lexer.expect("{");
    bool closed = false;
    char token[256];
    while (valid && lexer.next(token, sizeof(token))) {
        if (!strcmp(token,"}")) { closed = true; break; }
        if (!Q_stricmp(token,"both")) valid = parseBoth(lexer, parsed, filename);
        else if (!Q_stricmp(token,"both_altweap")) valid = parseBoth(lexer, alternate, filename);
        else if (!Q_stricmp(token,"client") || !Q_stricmp(token,"client_altweap")) valid = lexer.skipSection();
        else { G_Printf("^3WARNING: Unknown weapon section '%s' in %s\n", token, filename); valid = false; }
    }
    if (!valid || !closed || lexer.failed || lexer.next(token,sizeof(token)) || lexer.failed) {
        G_Printf("^3WARNING: Invalid weapon script %s; keeping defaults\n", filename);
        return qfalse;
    }
    // Commit only a complete, valid file; failed custom files cannot contaminate fallback data.
    weaponScripts[weapon] = parsed;
    int alternateWeapon = altWeaponFor(weapon);
    if (alternateWeapon >= 0) weaponScripts[alternateWeapon] = alternate;
    return qtrue;
}

void G_ApplyWeaponScript(int weapon) {
    BG_ApplyWeaponScriptAmmo(weapon);
    BG_ApplyWeaponScriptStatNames();
    ammoTableNeedsUpdate = true;
}

void G_LoadWeaponScripts() {
    G_ResetWeaponScripts();
    const char* directory = g_weaponScriptsDir.string[0] ? g_weaponScriptsDir.string : "weapons";
    G_Printf("Loading weapon scripts from '%s'...\n", directory);
    for (int weapon = 1; weapon < WP_NUM_WEAPONS; ++weapon) {
        if (!weaponFilenames[weapon][0] || isAltWeapon(weapon)) continue;
        char filename[MAX_WEAPONSCRIPT_PATH];
        Com_sprintf(filename,sizeof(filename),"%s/%s.weap",directory,weaponFilenames[weapon]);
        qboolean loaded = G_ParseWeaponScript(filename,weapon);
        if (!loaded && Q_stricmp(directory,"weapons")) {
            Com_sprintf(filename,sizeof(filename),"weapons/%s.weap",weaponFilenames[weapon]);
            loaded = G_ParseWeaponScript(filename,weapon);
        }
        if (loaded) {
            G_ApplyWeaponScript(weapon);
            int alternate = altWeaponFor(weapon);
            if (alternate >= 0) G_ApplyWeaponScript(alternate);
        }
    }
    BG_updateAmmoTable();
}

void G_BuildWeaponScriptInfo(int weapon, char* info, int size) {
    info[0] = 0;
    weaponScriptDef_t* script = G_GetWeaponScript(weapon);
    if (!script || !script->hasScript || size < MAX_INFO_STRING) return;
    if (script->name[0]) Info_SetValueForKey(info,"n",script->name);
    if (script->statname[0]) Info_SetValueForKey(info,"t",script->statname);
    if (script->killMessage[0]) Info_SetValueForKey(info,"k",script->killMessage);
    if (script->killMessage2[0]) Info_SetValueForKey(info,"l",script->killMessage2);
    if (script->selfKillMessage[0]) Info_SetValueForKey(info,"s",script->selfKillMessage);
    char values[384];
    Com_sprintf(values,sizeof(values),"%u",script->present);
    for (int i = 0; i < WSF_COUNT; ++i) Q_strcat(values,sizeof(values),va(",%.9g",script->values[i]));
    Info_SetValueForKey(info,"v",values);
}

void G_SendWeaponScripts(int clientNum) {
    if (clientNum >= 0) G_SendXmodCS(clientNum);
    else for (int i = 0; i < level.maxclients; ++i)
        if (level.clients[i].pers.connected == CON_CONNECTED) G_SendXmodCS(i);
}
void G_BroadcastWeaponScripts() { G_SendWeaponScripts(-1); }

void G_ApplyWeaponProjectileOverrides(gentity_t* projectile, int weapon) {
    projectile->damage = (int)BG_WeaponScriptValue(weapon, WSF_DAMAGE, projectile->damage);
    projectile->splashDamage = (int)BG_WeaponScriptValue(weapon, WSF_SPLASH_DAMAGE, projectile->splashDamage);
    projectile->splashRadius = (int)BG_WeaponScriptValue(weapon, WSF_SPLASH_RADIUS, projectile->splashRadius);
}
