#ifndef GAME_CVAR_CVAR_H
#define GAME_CVAR_CVAR_H

///////////////////////////////////////////////////////////////////////////////

namespace objects {
    extern Cvar g_admin;
    extern Cvar g_adminLog;

    extern Cvar g_bulletmodeDebug;
    extern Cvar g_bulletmodeReference;
    extern Cvar g_bulletmodeTrail;

    extern Cvar g_hitmodeAntilag;
    extern Cvar g_hitmodeAntilagLerp;
    extern Cvar g_hitmodeDebug;
    extern Cvar g_hitmodeFat;
    extern Cvar g_hitmodeGhosting;
    extern Cvar g_hitmodeReference;
    extern Cvar g_hitmodeZone;

    extern Cvar g_jxacEnable;
    extern Cvar g_jxacScreenshotQuality;
    extern Cvar g_jxacScreenshotPath;
    extern Cvar g_jxacCheckCvars;
    extern Cvar g_jxacCheckWallhack;
    extern Cvar g_jxacAutoBan;
    extern Cvar g_jxacAutoKick;
    extern Cvar g_jxacLogFile;
    extern Cvar g_jxacHeartbeatTimeout;
    extern Cvar g_jxacCvarFile;
    extern Cvar g_jxacCheatFile;
    
    extern Cvar g_jxacModuleScan;
    extern Cvar g_jxacAntiTamper;
    extern Cvar g_jxacCheckSpeedhack;
    
    extern Cvar g_jxacCvarScan;
    extern Cvar g_jxacCvarScanWait;
    extern Cvar g_jxacCvarScanDelay;
    extern Cvar g_jxacCvarScanInterval;
    extern Cvar g_jxacCvarScanMaxWarnings;
    
    extern Cvar g_jxacForceCvarFile;
    extern Cvar g_jxacCheatCvarFile;
    extern Cvar g_jxacCheatDbFile;

    extern Cvar g_kickMessage;
    extern Cvar g_kickTime;
    extern Cvar g_protestMessage;

    extern Cvar g_maxLandmines;
    extern Cvar g_maxTripmines;
    extern Cvar g_snap;
    extern Cvar g_shutdownExit;
    extern Cvar g_warmup;

    extern Cvar sv_tempBanMessage;

    extern Cvar g_test; // TODO: nuke when done with scale testing
} // namespace objects

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CVAR_CVAR_H
