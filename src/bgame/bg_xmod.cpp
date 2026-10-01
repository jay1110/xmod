#include <bgame/impl.h>

#if defined( CGAMEDLL ) || defined( GAMEDLL ) 

///////////////////////////////////////////////////////////////////////////////

bool ammoTableNeedsUpdate = false;

void
BG_updateAmmoTable()
{
    if (!ammoTableNeedsUpdate)
        return;

    ammoTableNeedsUpdate = false;
    memcpy( ammoTableMP, ammoTableMP_BACKUP, sizeof(ammoTableMP) );
    for (int weapon = 1; weapon < WP_NUM_WEAPONS; ++weapon)
        BG_ApplyWeaponScriptAmmo(weapon);

    if (cvars::bg_weapons.ivalue & SBW_FAIRRIFLES ) {

        /* Make the rifles even.
         * Essentially copy all bits from src to dest, restore mod field.
         */

        struct Data {
            const weapon_t src;
            const weapon_t dst;
        };

        Data dataList[] = {
            { WP_K43,       WP_GARAND },
            { WP_K43_SCOPE, WP_GARAND_SCOPE },
            { WP_KAR98,     WP_CARBINE },
            { WP_GPG40,     WP_M7 },
            { WP_NONE,      WP_NONE }
        };

        for (Data* data = dataList; data->src != WP_NONE; data++) {
            ammotable_t* src = GetAmmoTableData( data->src );
            ammotable_t* dst = GetAmmoTableData( data->dst );

            int saved_mod = dst->mod;
            memcpy( dst, src, sizeof(ammotable_t) );
            dst->mod = saved_mod;
        }
    }

    if (cvars::bg_wolfrof.ivalue) {
        ammoTableMP[WP_THOMPSON].nextShotTime = 110; // RTCW: 120
        ammoTableMP[WP_MP40].nextShotTime     = 110; // RTCW: 100
        ammoTableMP[WP_STEN].nextShotTime     = 110; // RTCW: 110
    }

    // SBW_FASTSHOOTING - Enable fast shooting for MP40, Thompson and Sten (delay lowered from 150ms to 110ms)
    if (cvars::bg_weapons.ivalue & SBW_FASTSHOOTING) {
        ammoTableMP[WP_MP40].nextShotTime     = 110;
        ammoTableMP[WP_THOMPSON].nextShotTime = 110;
        ammoTableMP[WP_STEN].nextShotTime     = 110;
    }

    // SBW_NOSTEN_HEAT - Disable Sten overheating
    if (cvars::bg_weapons.ivalue & SBW_NOSTEN_HEAT) {
        ammoTableMP[WP_STEN].maxHeat = 0;
    }

    // SBW_NOMG42_HEAT - Disable Mobile MG42 overheating
    if (cvars::bg_weapons.ivalue & SBW_NOMG42_HEAT) {
        ammoTableMP[WP_MOBILE_MG42].maxHeat = 0;
        ammoTableMP[WP_MOBILE_MG42_SET].maxHeat = 0;
    }

    if (cvars::bg_ammoUnlimited.ivalue) {
        for (int i = 0; i < WP_NUM_WEAPONS; i++) {
            ammoTableMP[i].uses = 0;
            ammoTableMP[i].reloadTime = 0;
            ammoTableMP[i].maxHeat = 0;
            ammoTableMP[i].coolRate = 0;
        }
    }

    if (cvars::bg_ammoFireDelayNudge.ivalue) {
        for (int i = 0; i < WP_NUM_WEAPONS; i++)
            ammoTableMP[i].fireDelayTime += cvars::bg_ammoFireDelayNudge.ivalue;
    }

    if (cvars::bg_ammoNextDelayNudge.ivalue) {
        for (int i = 0; i < WP_NUM_WEAPONS; i++)
            ammoTableMP[i].nextShotTime += cvars::bg_ammoNextDelayNudge.ivalue;
    }

    Com_Printf( "^3Weapons Table Updated\n" );
}

///////////////////////////////////////////////////////////////////////////////

#endif // CGAMEDLL, GAMEDLL

///////////////////////////////////////////////////////////////////////////////

bool BG_IsPercent(const char* str)
{
    while (*str) {
        if (*str == '%')
            return true;
        if (*str < '0' || *str > '9')
            return false;
        str++;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
