#ifndef BGAME_WEAPON_SCRIPT_H
#define BGAME_WEAPON_SCRIPT_H

// Presence is separate from the value: zero and false are valid overrides.
enum weaponScriptField_t {
    WSF_DAMAGE, WSF_SPLASH_DAMAGE, WSF_SPLASH_RADIUS, WSF_SPREAD,
    WSF_SPREAD_RATIO, WSF_MAXAMMO, WSF_MAXCLIP, WSF_STARTAMMO,
    WSF_STARTCLIP, WSF_RELOADTIME, WSF_FIREDELAYTIME, WSF_NEXTSHOTTIME,
    WSF_MAXHEAT, WSF_COOLRATE, WSF_HEADSHOT, WSF_REFLECTION,
    WSF_FALLOFF, WSF_GIBBING, WSF_MOVESPEED, WSF_USES, WSF_COUNT
};

struct weaponScriptDef_t {
    char name[64], statname[64];
    char selfKillMessage[128], killMessage[128], killMessage2[128];
    unsigned int present;
    float values[WSF_COUNT];
    qboolean hasScript;
};

extern weaponScriptDef_t bg_weaponScripts[WP_NUM_WEAPONS];
void BG_InitWeaponScriptState();
void BG_ApplyWeaponScriptAmmo(int weapon);
void BG_ApplyWeaponScriptStatNames();
qboolean BG_ParseWeaponScriptInfo(int weapon, const char* info);
float BG_WeaponScriptValue(int weapon, weaponScriptField_t field, float fallback);
bool BG_WeaponScriptHas(int weapon, weaponScriptField_t field);

#endif
