#ifndef BGAME_CVAR_CVAR_H
#define BGAME_CVAR_CVAR_H

///////////////////////////////////////////////////////////////////////////////

class Cvar
{
private:
    static list<Cvar*> REGISTRY;
    static list<Cvar*> UPDATE;

public:
    static void init   ( );
    static void update ( );

    static const list<Cvar*>& getUpdateList();

private:
    void trapRegister ( );
    void trapUpdate   ( );

    void(* _callback )(Cvar&);

    vmCvar_t _data;
    int      _callbackModificationCount;

public:
    Cvar( const string&, const string& = "", int = 0, void(*)(Cvar&) = 0, bool = true );
    ~Cvar();

    void set ( float );
    void set ( int );
    void set ( const string& );

    const string name;
    const string defaultValue;
    const int    flags;

    cvarHandle_t& handle;
    const int&    modificationCount;

    const float&      fvalue;
    const int&        ivalue;
    const char* const svalue;

    int lastModificationCount;
};

///////////////////////////////////////////////////////////////////////////////

namespace objects {
    extern Cvar bg_cpu;

    extern Cvar bg_dynamiteTime;
    extern Cvar bg_glow;
    extern Cvar bg_misc;
    extern Cvar bg_panzerWar;
    extern Cvar bg_poisonSyringes;
    extern Cvar bg_skills;
    extern Cvar bg_sniperWar;
    extern Cvar bg_weapons;
    extern Cvar bg_wolfrof;
    extern Cvar bg_spawnInvulNoClip;

    extern Cvar bg_maxEngineers;
    extern Cvar bg_maxMedics;
    extern Cvar bg_maxFieldOps;
    extern Cvar bg_maxCovertOps;

    extern Cvar bg_maxFlamers;
    extern Cvar bg_maxPanzers;
    extern Cvar bg_maxMG42s;
    extern Cvar bg_maxMortars;
    extern Cvar bg_maxGrenLaunchers;
    extern Cvar bg_maxM97s;

    extern Cvar bg_sk5_battle;
    extern Cvar bg_sk5_cvops;
    extern Cvar bg_sk5_eng;
    extern Cvar bg_sk5_fdops;
    extern Cvar bg_sk5_lightweap;
    extern Cvar bg_sk5_medic;
    extern Cvar bg_sk5_soldier;

    extern Cvar bg_bulletmode;
    extern Cvar bg_hitmode;

    extern Cvar bg_ammoUnlimited;
    extern Cvar bg_ammoFireDelayNudge;
    extern Cvar bg_ammoNextDelayNudge;

    extern Cvar bg_covertops;

    extern Cvar bg_fixedphysics;
    extern Cvar bg_fixedphysicsfps;

    extern Cvar bg_proneDelay;

    extern Cvar bg_doubleJump;
    extern Cvar bg_djHeight;
    extern Cvar bg_weaponsenable;

    extern Cvar gameState;

    // g_noReload and g_noCharge are server-synced via SERVERINFO
    extern Cvar g_noReload;
    extern Cvar g_noCharge;

    // JXAC enabled status - synced from server
    extern Cvar bg_jxacEnabled;
    extern Cvar bg_jxacModuleScan;
    extern Cvar bg_jxacAntiTamper;
    extern Cvar bg_jxacSpeedhack;
}

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_CVAR_CVAR_H
