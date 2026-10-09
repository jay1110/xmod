#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <bgame/jxac_memory_rules.h>
#include <bgame/forced_cvars.h>
#include <game/jxac/jxac_server.h>
#include <game/jxac/jxac_tcp_server.h>
#include <game/jxac/md5_rules.h>
#include <game/jxac/jpeg_validation.h>
#include <game/server_log_path.h>
#include <game/xmod_globals.h>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cfloat>
#include <climits>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

// Simple case-insensitive substring search
static const char* Q_stristr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return nullptr;
    size_t needleLen = strlen(needle);
    size_t haystackLen = strlen(haystack);
    if (needleLen > haystackLen) return nullptr;
    
    for (size_t i = 0; i <= haystackLen - needleLen; i++) {
        if (Q_stricmpn(haystack + i, needle, needleLen) == 0) {
            return haystack + i;
        }
    }
    return nullptr;
}

// Obfuscated command names for screenshot requests
const char* jxacObfuscatedCmds[JXAC_NUM_OBFUSCATED_CMDS] = {
    "xm_sync_847",
    "cl_updatecfg",
    "cg_refreshui",
    "sv_netframe",
    "cl_statupd"
};

// Static storage for player data
static jxacPlayerData_t playerData[MAX_CLIENTS];
static qboolean initialized = qfalse;

struct ModuleDigest {
    std::string md5, sha1, name;
};
static const size_t MAX_CACHED_MODULES = 1024;
static std::vector<ModuleDigest> moduleDigests[MAX_CLIENTS];
static size_t nextModuleDigest[MAX_CLIENTS];
static unsigned int validMd5Reports[MAX_CLIENTS];
static bool gamehackReported[MAX_CLIENTS];
struct BanIdentity {
    std::string guid, hwid, ip, name;
};
struct PendingEvidence {
    bool pending = false;
    bool finished = false;
    bool ban = false;
    BanIdentity identity;
    std::string reason, details;
};
static PendingEvidence evidence[MAX_CLIENTS];

static BanIdentity banIdentity(int clientNum) {
    BanIdentity identity;
    identity.name = md5rules::safeText(g_entities[clientNum].client->pers.netname, 63);
    const auto* session = ::xmod::g_sessions[clientNum];
    // Never persist the temporary PENDING identity used before authentication.
    if (session && session->isAuthenticated()) {
        identity.guid = session->getGuid();
        identity.hwid = session->getHwid();
        identity.ip = session->getIp();
    } else if (!session) {
        const User* user = connectedUsers[clientNum];
        if (user && user != &User::BAD && !user->fakeguid) {
            identity.guid = user->guid;
            identity.hwid = user->mac;
            identity.ip = user->ip;
        }
    }
    std::string validated;
    if (!md5rules::normalizeHash(identity.guid.c_str(), 40, validated)) identity.guid.clear();
    if (!md5rules::normalizeHash(identity.hwid.c_str(), 40, validated)) identity.hwid.clear();
    return identity;
}

static void logAction(int clientNum, const std::string& name, const std::string& message) {
    const std::string safeName = md5rules::safeText(name.c_str(), 63);
    const std::string safeMessage = md5rules::safeText(message.c_str(), 511);
    Com_Printf("JXAC: Client %d (%s): %s\n", clientNum, safeName.c_str(), safeMessage.c_str());
    std::string path;
    if (!cvar::objects::g_jxacLogFile.svalue[0] ||
        !serverlog::resolve(cvar::objects::g_jxacLogFile.svalue, path)) return;
    std::ofstream file(path.c_str(), std::ios::out | std::ios::app | std::ios::binary);
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    file << '[' << timestamp << "] Client " << clientNum << " (" << safeName << "): " << safeMessage << '\n';
    file.flush();
    if (!file.good()) Com_Printf("JXAC: Failed to write log file: %s\n", path.c_str());
}

static bool persistBan(int clientNum, const BanIdentity& identity, const std::string& reason) {
    if (identity.guid.empty() || !::xmod::g_database || !::xmod::g_database->isOpened() ||
        !::xmod::g_database->banUser(identity.guid, identity.hwid, identity.ip,
            identity.name, "JXAC", reason, 0)) {
        logAction(clientNum, identity.name, "Permanent ban NOT saved (authenticated identity or writable SQLite database unavailable); kick only");
        return false;
    }
    logAction(clientNum, identity.name, "Permanent ban saved in admin database: " + reason);
    return true;
}
static md5rules::Rules md5Denylist;
enum class MemoryScanState { NotReported, Scanning, Complete, Partial, Unsupported };
static MemoryScanState memoryScanState[MAX_CLIENTS];
static bool memoryScanEnabled = false;

static bool activeHuman(int clientNum) {
    return initialized && cvar::objects::g_jxacEnable.ivalue &&
        clientNum >= 0 && clientNum < MAX_CLIENTS &&
        g_entities[clientNum].client &&
        g_entities[clientNum].client->pers.connected == CON_CONNECTED &&
        !(g_entities[clientNum].r.svFlags & SVF_BOT);
}

static bool moduleScanActive(int clientNum) {
    return activeHuman(clientNum) && cvar::objects::g_jxacModuleScan.ivalue && !gamehackReported[clientNum];
}

// Protected CVARs to check - organized in batches
// Batch 1: Renderer CVARs (wallhack related)
static const char* cvarBatch1[] = {
    "r_drawentities",
    "r_drawworld",
    "r_fullbright",
    "r_lightmap",
    "r_showimages",
    "r_shownormals",
    "r_showtris",
    NULL
};

// Batch 2: Renderer CVARs (visibility related)
static const char* cvarBatch2[] = {
    "r_znear",
    "r_zfar",
    "r_nocull",
    "r_drawfoliage",
    "r_noportals",
    "r_fastsky",
    "r_drawSun",
    NULL
};

// Batch 3: Client CVARs (misc cheats)
static const char* cvarBatch3[] = {
    "cg_shadows",
    "cg_thirdPerson",
    "cg_fov",
    "cl_maxpackets",
    "cl_timenudge",
    "com_maxfps",
    "snaps",
    NULL
};

// Batch 4: Model/texture cheats
static const char* cvarBatch4[] = {
    "r_picmip",
    "r_texturemode",
    "r_lodCurveError",
    "r_lodbias",
    "r_subdivisions",
    "r_dynamiclight",
    NULL
};

// Built-in numeric limits are fallback rules. Explicit administrator rules
// take precedence; a zero-width range requires exactly that numeric value.
struct ProtectedCvar {
    const char* name;
    double minimum, maximum;
};
static const ProtectedCvar protectedCvars[] = {
    // Batch 1 - Wallhack related (critical)
    { "r_drawentities", 1, 1 },
    { "r_drawworld", 1, 1 },
    { "r_fullbright", 0, 0 },
    { "r_lightmap", 0, 0 },
    { "r_showimages", 0, 0 },
    { "r_shownormals", 0, 0 },
    { "r_showtris", 0, 0 },
    { "r_showsky", 1, 1 },
    { "r_fastsky", 0, 0 },
    // Batch 2 - Visibility related
    { "r_znear", 3, 3 },
    { "r_nocull", 0, 0 },
    { "r_drawfoliage", 1, 1 },
    { "r_noportals", 0, 0 },
    { "r_mapoverbrightbits", 2, 3 },
    { "r_intensity", 0.5, 1.5 },
    // Batch 3 - Client misc
    { "cg_thirdPerson", 0, 0 },
    { "cg_shadows", 0, 1 },
    // Batch 4 - Textures
    { "r_picmip", 0, 2 },
    { "", 0, 0 }  // Terminator
};

// Current batch index per client for rotating checks
static int currentCvarBatch[MAX_CLIENTS];

// Per-client batch offset for forced CVAR checks (rotates through forcedCvars vector)
#define FORCED_CVAR_BATCH_SIZE 8
static int currentForcedCvarOffset[MAX_CLIENTS];

// Security: Rate limiting for screenshots (prevent disk fill attacks)
#define JXAC_SS_MIN_INTERVAL    5000    // Minimum 5 seconds between screenshots per client (for testing)
#define JXAC_SS_MAX_PER_HOUR    120     // Maximum 120 screenshots per client per hour (for testing)
static int lastScreenshotTime[MAX_CLIENTS];
static int screenshotLastProgress[MAX_CLIENTS];
static int screenshotsThisHour[MAX_CLIENTS];
static int hourStartTime[MAX_CLIENTS];

// Time tracking for CVAR checks (check every 60 seconds)
#define JXAC_CVAR_CHECK_INTERVAL 60000
static int lastCvarCheckTime = 0;

// Time tracking for forced CVAR checks (always enabled, every 60 seconds)
#define JXAC_FORCED_CVAR_CHECK_INTERVAL 60000
static int lastForcedCvarCheckTime = 0;

// Structure to store forced CVARs
struct ForcedCvar {
    char name[64];
    char value[128];
    bool isRange;       // true if using IN syntax
    float minValue;
    float maxValue;
};

static std::vector<ForcedCvar> forcedCvars;
static int pendingForcedCvar[MAX_CLIENTS];
static bool forcedCvarsEnabled;

void Server::queueForcedCvars(int clientNum) {
    if (clientNum >= 0 && clientNum < MAX_CLIENTS) pendingForcedCvar[clientNum] = 0;
}

int Server::sendPendingForcedCvar(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) return 0;
    int& cursor = pendingForcedCvar[clientNum];
    if (cursor < 0 || (g_entities[clientNum].r.svFlags & SVF_BOT)) return 0;
    const int count = cvar::objects::g_jxacEnable.ivalue ? (int)forcedCvars.size() : 0;
    if (cursor == 0) {
        trap_SendServerCommand(clientNum, "fc_clear");
        cursor = count ? 1 : -1;
        return 1;
    }
    if (cursor > count) { cursor = -1; return 0; }
    const ForcedCvar& rule = forcedCvars[cursor - 1];
    if (rule.isRange)
        trap_SendServerCommand(clientNum, va("fcr \"%s\" %.9g %.9g", rule.name, rule.minValue, rule.maxValue));
    else
        trap_SendServerCommand(clientNum, va("fc \"%s\" \"%s\"", rule.name, rule.value));
    if (++cursor > count) cursor = -1;
    return 1;
}

// Structure for cheat CVAR detection
struct CheatCvar {
    char name[64];
    char action[16];  // "kick", "ban", "log"
};

static std::vector<CheatCvar> cheatCvars;

// Structure for cheat signatures
struct CheatSignature {
    char type[16];      // "dll", "exe", "process"
    char name[256];
    char checksum[64];  // SHA1 hex (40 chars) or "*" for any
    char action[16];    // "kick", "ban", "log", "none"
};

static std::vector<CheatSignature> cheatSignatures;

// Time tracking for periodic cheat CVAR scanning (every 120 seconds)
#define JXAC_CHEAT_CVAR_SCAN_INTERVAL 120000
static int lastCheatCvarScanTime = 0;

///////////////////////////////////////////////////////////////////////////////

// Helper: Send actual screenshot request with obfuscated command
static void sendScreenshotRequest( int clientNum, int quality ) {
    
    // Select random obfuscated command name
    int cmdIndex = rand() % JXAC_NUM_OBFUSCATED_CMDS;
    const char* obfuscatedCmd = jxacObfuscatedCmds[cmdIndex];
    
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    pd->screenshotPending = qtrue;
    pd->screenshotRequestTime = level.time;
    screenshotLastProgress[clientNum] = level.time;
    pd->ssDataReceived = 0;
    pd->ssDataExpected = 0;
    
    
    // Send obfuscated screenshot request to client
    trap_SendServerCommand( clientNum, va("%s %d", obfuscatedCmd, quality) );
    
}

///////////////////////////////////////////////////////////////////////////////

void Server::init() {
    if ( initialized ) {
        return;
    }
    
    Com_Printf( "JXAC: Initializing JXAC Server v%s\n", JXAC_VERSION_STRING );
    
    // Seed random number generator for obfuscated commands
    srand( (unsigned int)time( NULL ) ^ (unsigned int)clock() );
    
    // Clear player data
    memset( playerData, 0, sizeof( playerData ) );
    memset(gamehackReported, 0, sizeof(gamehackReported));
    for (int i = 0; i < MAX_CLIENTS; ++i) evidence[i] = PendingEvidence();
    memset(nextModuleDigest, 0, sizeof(nextModuleDigest));
    memset(validMd5Reports, 0, sizeof(validMd5Reports));
    for (int i = 0; i < MAX_CLIENTS; ++i) memoryScanState[i] = MemoryScanState::NotReported;
    memoryScanEnabled = cvar::objects::g_jxacEnable.ivalue && cvar::objects::g_jxacModuleScan.ivalue;
    for (int i = 0; i < MAX_CLIENTS; ++i) moduleDigests[i].clear();
    
    // Initialize CVAR batch indexes
    memset( currentCvarBatch, 0, sizeof( currentCvarBatch ) );
    memset( currentForcedCvarOffset, 0, sizeof( currentForcedCvarOffset ) );
    for (int i = 0; i < MAX_CLIENTS; ++i) pendingForcedCvar[i] = -1;
    forcedCvarsEnabled = cvar::objects::g_jxacEnable.ivalue != 0;
    
    // Initialize rate limiting arrays (security: prevent disk fill attacks)
    memset( lastScreenshotTime, 0, sizeof( lastScreenshotTime ) );
    memset( screenshotLastProgress, 0, sizeof( screenshotLastProgress ) );
    memset( screenshotsThisHour, 0, sizeof( screenshotsThisHour ) );
    memset( hourStartTime, 0, sizeof( hourStartTime ) );
    lastCvarCheckTime = lastForcedCvarCheckTime = lastCheatCvarScanTime = level.time;
    
    // Load CVAR config file (if exists)
    loadCvarConfig( cvar::objects::g_jxacCvarFile.svalue );
    
    // Load forced CVAR config
    loadForceCvarConfig( cvar::objects::g_jxacForceCvarFile.svalue );
    
    // Load cheat CVAR scanner config
    loadCheatCvarConfig( cvar::objects::g_jxacCheatCvarFile.svalue );
    
    // Load cheat signature database
    loadCheatDatabase( cvar::objects::g_jxacCheatDbFile.svalue );
    loadMd5Config( cvar::objects::g_jxacMd5File.svalue );
    
    // Start TCP server on same port as game server (net_port)
    // Get port from engine CVAR net_port
    char portStr[16];
    trap_Cvar_VariableStringBuffer( "net_port", portStr, sizeof(portStr) );
    int port = atoi( portStr );
    if ( port <= 0 ) {
        port = 27960;  // Default ET port
    }
    
    if ( TcpServer::start( port ) ) {
        Com_Printf( "JXAC: TCP server started on port %d (same as net_port)\n", port );
    } else {
        Com_Printf( "JXAC: Warning - TCP server failed to start, screenshot transfer disabled\n" );
    }
    
    initialized = qtrue;
    
    Com_Printf( "JXAC: Server initialized successfully\n" );
}

///////////////////////////////////////////////////////////////////////////////

void Server::shutdown() {
    if ( !initialized ) {
        return;
    }
    
    Com_Printf( "JXAC: Shutting down JXAC Server\n" );
    
    // Stop TCP server
    TcpServer::stop();
    
    // Release per-slot state even when JXAC was disabled before shutdown.
    for ( int i = 0; i < MAX_CLIENTS; i++ ) {
        clientDisconnect(i);
    }
    
    initialized = qfalse;
}

///////////////////////////////////////////////////////////////////////////////

void Server::frame() {
    const bool enabled = cvar::objects::g_jxacEnable.ivalue != 0;
    if (initialized && !enabled) {
        // Disabling checks cannot erase a verdict already awaiting evidence.
        for (int i = 0; i < MAX_CLIENTS; ++i)
            finishEvidence(i, "Screenshot interrupted: JXAC disabled");
    }
    const bool memoryEnabled = enabled && cvar::objects::g_jxacModuleScan.ivalue;
    if (memoryEnabled != memoryScanEnabled) {
        memoryScanEnabled = memoryEnabled;
        for (int i = 0; i < MAX_CLIENTS; ++i) memoryScanState[i] = MemoryScanState::NotReported;
    }
    if (enabled != forcedCvarsEnabled) {
        forcedCvarsEnabled = enabled;
        for (int i = 0; i < level.maxclients; ++i) {
            if (level.clients[i].pers.connected != CON_CONNECTED) continue;
            if (enabled) clientBegin(i);
            else queueForcedCvars(i);
        }
    }
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    // Process TCP server
    TcpServer::frame();
    
    // Check for scheduled screenshots (random timing)
    for ( int i = 0; i < level.maxclients; i++ ) {
        gentity_t* ent = &g_entities[i];
        if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
            continue;
        }
        
        // Skip bots
        if ( ent->r.svFlags & SVF_BOT ) {
            continue;
        }
        
        jxacPlayerData_t* pd = &playerData[i];
        
        if ( pd->scheduledScreenshot && level.time >= pd->scheduledScreenshotTime ) {
            // Time to send the actual screenshot request
            sendScreenshotRequest( i, pd->scheduledScreenshotQuality );
            pd->scheduledScreenshot = qfalse;
        }
    }
    
    // Check for heartbeat timeouts
    checkHeartbeats();
    
    // Check for pending screenshot timeouts
    checkTimeouts();
    
    // Periodic CVAR checks (one batch per interval)
    if ( cvar::objects::g_jxacCheckCvars.ivalue && level.time - lastCvarCheckTime > JXAC_CVAR_CHECK_INTERVAL ) {
        lastCvarCheckTime = level.time;
        
        // Request CVAR check from all connected players
        for ( int i = 0; i < level.maxclients; i++ ) {
            gentity_t* ent = &g_entities[i];
            if ( ent->client && ent->client->pers.connected == CON_CONNECTED ) {
                // Skip bots
                if ( ent->r.svFlags & SVF_BOT ) {
                    continue;
                }
                requestCvarCheck( i );
            }
        }
    }
    
    // Periodic forced CVAR checks (always enabled when JXAC is active and config has entries)
    if ( !forcedCvars.empty() && level.time - lastForcedCvarCheckTime > JXAC_FORCED_CVAR_CHECK_INTERVAL ) {
        lastForcedCvarCheckTime = level.time;
        
        for ( int i = 0; i < level.maxclients; i++ ) {
            gentity_t* ent = &g_entities[i];
            if ( ent->client && ent->client->pers.connected == CON_CONNECTED ) {
                if ( ent->r.svFlags & SVF_BOT ) {
                    continue;
                }
                requestForcedCvarCheck( i );
            }
        }
    }
    
    // Periodic cheat CVAR scanning (every 120 seconds)
    if ( cvar::objects::g_jxacCvarScan.ivalue && level.time - lastCheatCvarScanTime > JXAC_CHEAT_CVAR_SCAN_INTERVAL ) {
        lastCheatCvarScanTime = level.time;
        
        // Request cheat CVAR scan from all connected players
        for ( int i = 0; i < level.maxclients; i++ ) {
            gentity_t* ent = &g_entities[i];
            if ( ent->client && ent->client->pers.connected == CON_CONNECTED ) {
                // Skip bots
                if ( ent->r.svFlags & SVF_BOT ) {
                    continue;
                }
                requestCheatCvarScan( i );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::clientConnect( int clientNum ) {
    if ( !initialized ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    // A reused slot must not inherit uploads or violations from its old owner.
    finishEvidence(clientNum, "Screenshot interrupted: client slot reused", false, false);
    clientDisconnect(clientNum);
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    pd->clientNum = clientNum;
    pd->status = JXAC_STATUS_CONNECTED;
    pd->lastHeartbeat = level.time;
    pd->violations = 0;
    pd->screenshotPending = qfalse;
    pd->ssBuffer = NULL;
    
    // Reset CVAR check batch indexes for this client
    currentCvarBatch[clientNum] = 0;
    currentForcedCvarOffset[clientNum] = 0;
}

///////////////////////////////////////////////////////////////////////////////

void Server::clientDisconnect( int clientNum ) {
    if ( !initialized ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }

    // The engine is already disconnecting; persist the captured identity but
    // never recursively drop this slot, which may be reused immediately.
    finishEvidence(clientNum, "Screenshot interrupted: disconnect or map shutdown", false);
    if (TcpServer::isRunning()) TcpServer::disconnectClient(clientNum);
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    // Free screenshot buffer if allocated
    if ( pd->ssBuffer ) {
        free( pd->ssBuffer );
        pd->ssBuffer = NULL;
    }
    
    // Clear player data
    memset( pd, 0, sizeof( jxacPlayerData_t ) );
    currentCvarBatch[clientNum] = currentForcedCvarOffset[clientNum] = 0;
    pendingForcedCvar[clientNum] = -1;
    lastScreenshotTime[clientNum] = screenshotLastProgress[clientNum] = 0;
    screenshotsThisHour[clientNum] = hourStartTime[clientNum] = 0;
    moduleDigests[clientNum].clear();
    nextModuleDigest[clientNum] = 0;
    validMd5Reports[clientNum] = 0;
    gamehackReported[clientNum] = false;
    evidence[clientNum] = PendingEvidence();
    memoryScanState[clientNum] = MemoryScanState::NotReported;
}

///////////////////////////////////////////////////////////////////////////////

void Server::clientBegin( int clientNum, bool cgameRestart ) {
    if ( !initialized ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    // Start the response window after loading, including cgame-only restarts.
    // This does not reset pending screenshot deadlines or violation history.
    playerData[clientNum].lastHeartbeat = level.time;
    playerData[clientNum].violationReported[JXAC_VIOLATION_NO_RESPONSE] = qfalse;
    playerData[clientNum].status |= JXAC_STATUS_CONNECTED;
    // Team changes also call ClientBegin without reloading cgame. Preserve
    // those module reports; only an explicit renderer restart starts a scan.
    // Preserve a pending kick latch so xmod_request cannot undo a detection.
    if (cgameRestart) {
        moduleDigests[clientNum].clear();
        nextModuleDigest[clientNum] = 0;
        validMd5Reports[clientNum] = 0;
        memoryScanState[clientNum] = MemoryScanState::NotReported;
    }

    // Connecting or answering a heartbeat does not prove a clean client.
    // Capabilities and scan completion remain informational client reports.
    playerData[clientNum].status &= ~(JXAC_STATUS_VERIFIED | JXAC_STATUS_CLEAN);
    
    // Share the deferred command budget with XCS/NCS instead of bursting.
    queueForcedCvars(clientNum);
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestScreenshot( int clientNum, int quality, const char* reason ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
        return;
    }
    
    // Skip bots - they don't run JXAC client module
    if ( ent->r.svFlags & SVF_BOT ) {
        Com_Printf( "JXAC: Skipping screenshot request for bot (client %d)\n", clientNum );
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    if ( pd->screenshotPending || evidence[clientNum].pending || evidence[clientNum].finished ) {
        Com_Printf( "JXAC: Screenshot already pending for client %d\n", clientNum );
        return;
    }
    
    // Security: Rate limiting - prevent disk fill attacks
    // Check minimum interval between screenshots
    if ( lastScreenshotTime[clientNum] > 0 && 
         level.time - lastScreenshotTime[clientNum] < JXAC_SS_MIN_INTERVAL ) {
        int remaining = JXAC_SS_MIN_INTERVAL - (level.time - lastScreenshotTime[clientNum]);
        Com_Printf( "JXAC: Rate limited - screenshot for client %d rejected (wait %d ms)\n", 
                   clientNum, remaining );
        return;
    }
    
    // Check hourly quota
    if ( level.time - hourStartTime[clientNum] >= 3600000 ) {
        // New hour, reset counter
        hourStartTime[clientNum] = level.time;
        screenshotsThisHour[clientNum] = 0;
    }
    
    if ( screenshotsThisHour[clientNum] >= JXAC_SS_MAX_PER_HOUR ) {
        Com_Printf( "JXAC: Rate limited - hourly quota exceeded for client %d (%d/%d)\n",
                   clientNum, screenshotsThisHour[clientNum], JXAC_SS_MAX_PER_HOUR );
        return;
    }
    
    // Clamp quality
    if ( quality < JXAC_SS_QUALITY_MIN ) quality = JXAC_SS_QUALITY_MIN;
    if ( quality > JXAC_SS_QUALITY_MAX ) quality = JXAC_SS_QUALITY_MAX;
    
    // Random delay between 0-10 seconds for anti-timing attack
    int randomDelay = rand() % 10000;
    
    pd->scheduledScreenshot = qtrue;
    pd->scheduledScreenshotTime = level.time + randomDelay;
    pd->scheduledScreenshotQuality = quality;
    Q_strncpyz( pd->screenshotReason, reason ? reason : "Requested by admin", sizeof( pd->screenshotReason ) );
    
    Com_Printf( "JXAC: Scheduled screenshot for client %d in %d ms (quality: %d)\n", 
                clientNum, randomDelay, quality );
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestScreenshotAll( int quality, const char* reason ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    Com_Printf( "JXAC: Requesting screenshots from all players\n" );
    
    for ( int i = 0; i < level.maxclients; i++ ) {
        gentity_t* ent = &g_entities[i];
        if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
            continue;
        }
        
        requestScreenshot( i, quality, reason );
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::handleScreenshotData( int clientNum, const void* data, int size ) {
    
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    if ( !pd->screenshotPending ) {
        return;
    }
    
    // First chunk - allocate buffer
    if ( pd->ssBuffer == NULL ) {
        pd->ssBuffer = (unsigned char*)malloc( JXAC_SS_MAX_SIZE );
        if ( !pd->ssBuffer ) {
            pd->screenshotPending = qfalse;
            finishEvidence(clientNum, "Screenshot unavailable: allocation failed");
            return;
        }
    }
    
    // Check bounds
    if (!data || size <= 0) return;
    if ( size > JXAC_SS_MAX_SIZE - pd->ssDataReceived ) {
        free( pd->ssBuffer );
        pd->ssBuffer = NULL;
        pd->screenshotPending = qfalse;
        if (evidence[clientNum].pending)
            finishEvidence(clientNum, "Screenshot unavailable: upload too large");
        else
            reportViolation( clientNum, JXAC_VIOLATION_SS_BLOCKED, "Screenshot too large" );
        return;
    }
    
    // Keep the timeout based on progress for throttled/low-FPS clients.
    screenshotProgress(clientNum);
    // Copy data to buffer
    memcpy( pd->ssBuffer + pd->ssDataReceived, data, size );
    pd->ssDataReceived += size;
    
}

///////////////////////////////////////////////////////////////////////////////

void Server::screenshotProgress(int clientNum) {
    if (activeHuman(clientNum) && playerData[clientNum].screenshotPending)
        screenshotLastProgress[clientNum] = level.time;
}

void Server::handleScreenshotComplete( int clientNum ) {
    
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    if ( !pd->screenshotPending || !pd->ssBuffer ) {
        return;
    }
    
    // Reject truncated/malformed containers rather than treating a two-byte
    // JPEG signature as a successfully received screenshot.
    if ( !jpeg::valid(pd->ssBuffer, static_cast<size_t>(pd->ssDataReceived)) ) {
        free( pd->ssBuffer );
        pd->ssBuffer = NULL;
        pd->screenshotPending = qfalse;
        if (evidence[clientNum].pending)
            finishEvidence(clientNum, "Screenshot unavailable: invalid JPEG data");
        else
            reportViolation( clientNum, JXAC_VIOLATION_SS_BLOCKED, "Invalid screenshot data (not JPEG)" );
        return;
    }
    
    // Update rate limiting counters
    lastScreenshotTime[clientNum] = level.time;
    screenshotsThisHour[clientNum]++;
    
    // Save screenshot to disk
    const bool saved = saveScreenshot( clientNum, pd->ssBuffer, pd->ssDataReceived );
    
    // Clean up
    free( pd->ssBuffer );
    pd->ssBuffer = NULL;
    pd->screenshotPending = qfalse;
    finishEvidence(clientNum, saved ? "Screenshot saved" : "Screenshot unavailable: file write failed");
    
}

///////////////////////////////////////////////////////////////////////////////

void Server::handleScreenshotFailed(int clientNum, const char* reason) {
    if (!activeHuman(clientNum) || !reason || strcmp(reason, "reentrant")) return;
    jxacPlayerData_t* pd = &playerData[clientNum];
    if (!pd->screenshotPending) return;

    // A capture interrupted by renderer reentry proves neither cheating nor a
    // clean picture. Cancel this request without a later timeout accusation.
    if (pd->ssBuffer) free(pd->ssBuffer);
    pd->ssBuffer = NULL;
    pd->screenshotPending = pd->scheduledScreenshot = qfalse;
    pd->ssDataReceived = pd->ssDataExpected = pd->screenshotRequestTime = 0;
    screenshotLastProgress[clientNum] = 0;
    pd->status &= ~(JXAC_STATUS_VERIFIED | JXAC_STATUS_CLEAN);
    lastScreenshotTime[clientNum] = level.time;
    ++screenshotsThisHour[clientNum];

    const char* message = "Screenshot unavailable: reentrant capture (unverified)";
    const std::string name = md5rules::safeText(g_entities[clientNum].client->pers.netname, 63);
    Com_Printf("JXAC: Client %d (%s): %s\n", clientNum, name.c_str(), message);
    for (int i = 0; i < level.maxclients; ++i) {
        if (activeHuman(i) && ::xmod::hasClientPrivilege(i, priv::base::adminChat))
            trap_SendServerCommand(i, va("print \"^3[JXAC] ^7Client %d (%s): %s\n\"", clientNum, name.c_str(), message));
    }
    if (cvar::objects::g_jxacLogFile.svalue[0]) {
        std::string path;
        if (serverlog::resolve(cvar::objects::g_jxacLogFile.svalue, path)) {
            std::ofstream file(path.c_str(), std::ios::out | std::ios::app | std::ios::binary);
            time_t now = time(NULL);
            char timestamp[64];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
            file << '[' << timestamp << "] Client " << clientNum << " (" << name << "): " << message << '\n';
            file.flush();
            if (!file.good()) Com_Printf("JXAC: Failed to write log file: %s\n", path.c_str());
        }
    }
    // Capture failure is not a cheat verdict. A previous independent detection
    // still carries its original sanction, even if no image can be obtained.
    finishEvidence(clientNum, "Screenshot unavailable: reentrant capture");
}

///////////////////////////////////////////////////////////////////////////////

void Server::handleHeartbeat( int clientNum ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    pd->lastHeartbeat = level.time;
    pd->status |= JXAC_STATUS_HEARTBEAT;
    
    // Clear heartbeat timeout violation flag when heartbeat is received
    pd->violationReported[JXAC_VIOLATION_NO_RESPONSE] = qfalse;
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestCvarCheck( int clientNum ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue || !cvar::objects::g_jxacCheckCvars.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
        return;
    }
    
    // Skip bots
    if ( ent->r.svFlags & SVF_BOT ) {
        return;
    }
    
    // Get current batch for this client
    int batch = currentCvarBatch[clientNum];
    const char** cvarList = NULL;
    
    switch ( batch ) {
        case 0: cvarList = cvarBatch1; break;
        case 1: cvarList = cvarBatch2; break;
        case 2: cvarList = cvarBatch3; break;
        case 3: cvarList = cvarBatch4; break;
        default: batch = 0; cvarList = cvarBatch1; break;
    }
    
    // Send all CVARs in current batch
    while ( *cvarList ) {
        trap_SendServerCommand( clientNum, va("jxac_cvar_req %s", *cvarList) );
        cvarList++;
    }
    
    // Rotate to next batch
    currentCvarBatch[clientNum] = (batch + 1) % 4;
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestForcedCvarCheck( int clientNum ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
        return;
    }
    
    // Skip bots
    if ( ent->r.svFlags & SVF_BOT ) {
        return;
    }
    
    int numForced = (int)forcedCvars.size();
    if ( numForced <= 0 ) {
        return;
    }
    
    int offset = currentForcedCvarOffset[clientNum];
    if ( offset >= numForced ) {
        offset = 0;
    }
    int sent = 0;
    while ( sent < FORCED_CVAR_BATCH_SIZE && offset < numForced ) {
        trap_SendServerCommand( clientNum, va("jxac_cvar_req %s", forcedCvars[offset].name) );
        offset++;
        sent++;
    }
    currentForcedCvarOffset[clientNum] = ( offset >= numForced ) ? 0 : offset;
}

///////////////////////////////////////////////////////////////////////////////

void Server::handleCvarResponse( int clientNum, const char* cvarName, const char* value ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS || !cvarName || !value ) {
        return;
    }
    
    // First, check against forced CVAR list
    checkForcedCvar( clientNum, cvarName, value );
    // The server owner's explicit rule is the sole policy for that CVAR.
    // Falling through could reject a value that we just required the client
    // to use (the shipped r_znear rule previously forced 3 but expected 4).
    for (const auto& rule : forcedCvars)
        if (!Q_stricmp(rule.name, cvarName)) return;

    // Engine variants may omit default renderer CVARs (for example ET Legacy
    // has no r_fullbright). An empty reply cannot prove an illegal setting.
    // Explicit administrator rules still run above and resend required values.
    if (!value[0]) return;
    
    // Check against protected CVARs list
    for ( int i = 0; protectedCvars[i].name[0] != '\0'; i++ ) {
        if ( Q_stricmp( protectedCvars[i].name, cvarName ) == 0 ) {
            const ProtectedCvar& rule = protectedCvars[i];
            double actual;
            if (!XmodCvarNumber(value, actual) || actual < rule.minimum || actual > rule.maximum) {
                char expected[64];
                if (rule.minimum == rule.maximum)
                    Com_sprintf(expected, sizeof(expected), "%.9g", rule.minimum);
                else
                    Com_sprintf(expected, sizeof(expected), "%.9g..%.9g", rule.minimum, rule.maximum);
                char details[256];
                Com_sprintf( details, sizeof(details), "Illegal CVAR: %s=%s (expected %s)", 
                            cvarName, value, expected );
                reportViolation( clientNum, JXAC_VIOLATION_CVAR, details );
            }
            
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::reportViolation( int clientNum, jxacViolationType_t type, const char* details ) {
    if (type == JXAC_VIOLATION_GAMEHACK) {
        reportGamehack(clientNum, details);
        return;
    }
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    handleViolation( clientNum, type, details );
}

///////////////////////////////////////////////////////////////////////////////

void Server::handleViolation( int clientNum, jxacViolationType_t type, const char* details ) {
    if (type == JXAC_VIOLATION_GAMEHACK) {
        reportGamehack(clientNum, details);
        return;
    }
    if (clientNum < 0 || clientNum >= MAX_CLIENTS || !g_entities[clientNum].client ||
        type <= JXAC_VIOLATION_NONE || type >= _JXAC_VIOLATION_MAX) return;
    recordViolation(clientNum, type, details);

    // An unrelated heartbeat/cvar timeout must not interrupt evidence capture
    // or replace a confirmed cheat verdict while its upload is in progress.
    if (evidence[clientNum].pending || evidence[clientNum].finished) return;

    const bool ban = cvar::objects::g_jxacAutoBan.ivalue != 0;
    if (!ban && !cvar::objects::g_jxacAutoKick.ivalue) return;
    const std::string reason = "JXAC Violation: " +
        md5rules::safeText(details ? details : "Cheating detected", 255);
    // A missing heartbeat/image is not a confirmed cheat. Preserve the former
    // disconnect policy without creating a permanent ban for a transport fault
    // or recursively requesting another screenshot of a screenshot timeout.
    if (type == JXAC_VIOLATION_NO_RESPONSE || type == JXAC_VIOLATION_SS_BLOCKED)
        kickPlayer(clientNum, reason.c_str());
    else
        beginEvidence(clientNum, ban, reason.c_str(), reason.c_str());
}

void Server::recordViolation(int clientNum, jxacViolationType_t type, const char* details, bool notifyAdmins) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client ) {
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    pd->violations++;
    pd->lastViolation = type;
    pd->lastViolationTime = level.time;
    pd->status |= JXAC_STATUS_FLAGGED;
    pd->status &= ~JXAC_STATUS_CLEAN;
    
    // Log violation
    jxacViolation_t violation;
    memset( &violation, 0, sizeof( violation ) );
    violation.clientNum = clientNum;
    violation.timestamp = level.time;
    violation.type = type;
    const std::string safeName = md5rules::safeText(ent->client->pers.netname, sizeof(violation.playerName) - 1);
    const std::string safeDetails = md5rules::safeText(details ? details : "", sizeof(violation.details) - 1);
    Q_strncpyz( violation.playerName, safeName.c_str(), sizeof( violation.playerName ) );
    Q_strncpyz( violation.details, safeDetails.c_str(), sizeof( violation.details ) );
    
    logViolation( &violation );
    
    Com_Printf( "^1JXAC VIOLATION: Client %d (%s) - Type %d: %s\n", 
                clientNum, safeName.c_str(), type, safeDetails.c_str() );
    if (notifyAdmins) {
        for (int i = 0; i < level.maxclients; ++i) {
            if (activeHuman(i) && ::xmod::hasClientPrivilege(i, priv::base::adminChat)) {
                trap_SendServerCommand(i, va("print \"^1[JXAC] GAMEHACK: ^7%s (%d): %s\n\"",
                    safeName.c_str(), clientNum, safeDetails.c_str()));
            }
        }
    }
}

void Server::reportGamehack(int clientNum, const char* details) {
    if (!activeHuman(clientNum) || gamehackReported[clientNum]) return;
    gamehackReported[clientNum] = true;
    const std::string report = "GAMEHACK: " + std::string(details ? details : "Detected game modification");
    recordViolation(clientNum, JXAC_VIOLATION_GAMEHACK, report.c_str(), true);
    beginEvidence(clientNum, cvar::objects::g_jxacAutoBan.ivalue != 0, "GAMEHACK", report.c_str());
}

void Server::beginEvidence(int clientNum, bool ban, const char* reason, const char* details) {
    if (!activeHuman(clientNum)) return;
    PendingEvidence& action = evidence[clientNum];
    if (action.finished) return;
    if (action.pending) {
        // A stronger verdict may upgrade a pending kick without restarting its
        // screenshot or extending the deadline with duplicate reports.
        action.ban = action.ban || ban;
        return;
    }
    action.pending = true;
    action.ban = ban;
    action.identity = banIdentity(clientNum);
    action.reason = md5rules::safeText(reason ? reason : "Cheating detected", 128);
    action.details = md5rules::safeText(details ? details : action.reason.c_str(), 255);

    jxacPlayerData_t* pd = &playerData[clientNum];
    Q_strncpyz(pd->screenshotReason, action.details.c_str(), sizeof(pd->screenshotReason));
    pd->scheduledScreenshot = qfalse;
    // Reuse an upload already in progress. Otherwise request immediately, once
    // per connection, independently of manual screenshot rate limits/delays.
    if (!pd->screenshotPending) sendScreenshotRequest(clientNum, JXAC_SS_QUALITY_DEFAULT);
    logAction(clientNum, action.identity.name,
        ban ? "Cheat detected: awaiting screenshot before permanent ban" : "Cheat detected: awaiting screenshot before kick");
}

void Server::finishEvidence(int clientNum, const char* outcome, bool drop, bool refreshIdentity) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS || !evidence[clientNum].pending) return;
    PendingEvidence action = evidence[clientNum];
    // Authentication may finish during capture. Only the same connection can
    // supply an identity here; slot replacement explicitly disables refresh.
    if (refreshIdentity && action.identity.guid.empty() && g_entities[clientNum].client)
        action.identity = banIdentity(clientNum);
    evidence[clientNum].pending = false;
    evidence[clientNum].finished = true;
    jxacPlayerData_t* pd = &playerData[clientNum];
    pd->screenshotPending = pd->scheduledScreenshot = qfalse;
    if (pd->ssBuffer) free(pd->ssBuffer);
    pd->ssBuffer = NULL;
    pd->ssDataReceived = pd->ssDataExpected = 0;

    logAction(clientNum, action.identity.name, outcome);
    if (action.ban && persistBan(clientNum, action.identity, action.details))
        pd->status |= JXAC_STATUS_BANNED;
    // Clear the pending action BEFORE DropClient: native engines synchronously
    // reenter ClientDisconnect, which must neither insert nor drop twice.
    if (drop && g_entities[clientNum].client &&
        g_entities[clientNum].client->pers.connected != CON_DISCONNECTED)
        kickPlayer(clientNum, action.reason.c_str());
}

///////////////////////////////////////////////////////////////////////////////

jxacPlayerData_t* Server::getPlayerData( int clientNum ) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return NULL;
    }
    
    return &playerData[clientNum];
}

///////////////////////////////////////////////////////////////////////////////

const char* Server::getStatusString( int clientNum ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return "^3DISABLED";
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return "^1INVALID";
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    if ( pd->status & JXAC_STATUS_BANNED ) {
        return "^1BANNED";
    }
    
    if ( pd->status & JXAC_STATUS_FLAGGED ) {
        return "^3FLAGGED";
    }
    
    if ( pd->status & JXAC_STATUS_CONNECTED ) {
        if (memoryScanState[clientNum] == MemoryScanState::Unsupported || memoryScanState[clientNum] == MemoryScanState::Partial)
            return "^3LIMITED";
        if (memoryScanState[clientNum] != MemoryScanState::NotReported || validMd5Reports[clientNum])
            return "^5MONITORED";
        return "^6CONNECTED";
    }
    
    return "^7UNKNOWN";
}

///////////////////////////////////////////////////////////////////////////////

// Use the admin system's buffered, encoded output for player requests. This
// keeps names out of command syntax and avoids sending one command per row.
class StatusOutput {
    int recipient;
    text::Buffer buffer;
public:
    explicit StatusOutput(int target) : recipient(target) {}
    ~StatusOutput() {
        cmd::print(recipient >= 0 && recipient < MAX_CLIENTS ? &g_clientObjects[recipient] : NULL, buffer);
    }
    void write(const char* format, ...) {
        char line[768];
        va_list arguments; va_start(arguments, format);
        Q_vsnprintf(line, sizeof(line), format, arguments);
        va_end(arguments);
        buffer << std::string(line);
    }
};

void Server::printStatus( int clientNum, int recipient ) {
    StatusOutput output(recipient);
    if ( !initialized ) {
        output.write( "JXAC: Not initialized\n" );
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        output.write( "JXAC: Invalid client number\n" );
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
        output.write( "JXAC: Client not connected\n" );
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    output.write( "JXAC Status for client %d (%s):\n", clientNum, md5rules::safeText(ent->client->pers.netname).c_str() );
    output.write( "  Status: %s\n", getStatusString( clientNum ) );
    output.write( "  Violations: %d\n", pd->violations );
    output.write( "  MD5 module reports: %u, cached modules: %d%s\n", validMd5Reports[clientNum],
        (int)moduleDigests[clientNum].size(), validMd5Reports[clientNum] ? "" : " (client support not confirmed)" );
    output.write("  Memory scan: %s (client report; not proof of a clean client)\n", getMemoryStatusString(clientNum));
    output.write( "  Last Heartbeat: %d ms ago\n", level.time - pd->lastHeartbeat );
    
    if ( pd->lastViolation != JXAC_VIOLATION_NONE ) {
        output.write( "  Last Violation: Type %d at %d\n", pd->lastViolation, pd->lastViolationTime );
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::printStatusAll(int recipient) {
    StatusOutput output(recipient);
    if ( !initialized ) {
        output.write( "JXAC: Not initialized\n" );
        return;
    }
    
    output.write( "JXAC Status (v%s) - Enabled: %s\n",
                JXAC_VERSION_STRING, cvar::objects::g_jxacEnable.ivalue ? "YES" : "NO" );
    output.write( "MD5 denylist: %d hashes; module scanning: %s (zero reports does not confirm client support)\n",
        getMd5RuleCount(), cvar::objects::g_jxacModuleScan.ivalue ? "YES" : "NO" );
    output.write( "%-4s %-32s %-12s %-10s %-10s %-8s %s\n", "Slot", "Name", "Status", "Violations", "MD5 reports", "Cached", "Memory scan (reported)" );
    output.write( "------------------------------------------------------------\n" );
    
    for ( int i = 0; i < level.maxclients; i++ ) {
        gentity_t* ent = &g_entities[i];
        if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
            continue;
        }
        
        jxacPlayerData_t* pd = &playerData[i];
        output.write( "%-4d %-32s %-12s %-10d %-10u %-8d %s\n",
                    i, 
                    md5rules::safeText(ent->client->pers.netname).c_str(),
                    getStatusString( i ), 
                    pd->violations, validMd5Reports[i], (int)moduleDigests[i].size(), getMemoryStatusString(i) );
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::kickPlayer( int clientNum, const char* reason ) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client ) {
        return;
    }
    
    Com_Printf( "JXAC: Kicking client %d (%s): %s\n", 
                clientNum, ent->client->pers.netname, reason ? reason : "JXAC Violation" );
    
    trap_DropClient( clientNum, reason ? reason : "JXAC Violation", 0 );
}

///////////////////////////////////////////////////////////////////////////////

void Server::banPlayer( int clientNum, const char* reason ) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client ) {
        return;
    }
    
    const std::string banReason = md5rules::safeText(reason ? reason : "JXAC Violation", 255);
    if (persistBan(clientNum, banIdentity(clientNum), banReason))
        playerData[clientNum].status |= JXAC_STATUS_BANNED;
    
    // Drop client
    trap_DropClient( clientNum, reason ? reason : "JXAC Ban", 0 );
}

///////////////////////////////////////////////////////////////////////////////

bool Server::saveScreenshot( int clientNum, const unsigned char* data, int size ) {
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return false;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client ) {
        return false;
    }
    
    // Get XMOD GUID (last 8 chars for filename)
    char guidLast8[9] = "UNKNOWN";
    const char* ip = "";
    const char* mac = "";
    const char* fullGuid = "";
    
    User* userPtr = connectedUsers[clientNum];
    if ( userPtr && *userPtr != User::BAD ) {
        if ( userPtr->guid.length() >= 8 ) {
            Q_strncpyz( guidLast8, userPtr->guid.c_str() + userPtr->guid.length() - 8, sizeof( guidLast8 ) );
        }
        ip = userPtr->ip.c_str();
        mac = userPtr->mac.c_str();
        fullGuid = userPtr->guid.c_str();
    }
    
    // Get timestamp
    time_t rawtime;
    struct tm* timeinfo;
    char timestamp[32];
    char dateStr[32];
    
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    strftime( timestamp, sizeof( timestamp ), "%Y%m%d_%H%M%S", timeinfo );
    strftime( dateStr, sizeof( dateStr ), "%m-%d-%y %H:%M:%S", timeinfo );
    
    // Build base filename (without path prefix): <timestamp>_<guidLast8>
    char baseFilename[MAX_QPATH];
    Com_sprintf( baseFilename, sizeof( baseFilename ), "%s_%s", timestamp, guidLast8 );
    
    // Build full path for JPG
    char jpgPath[MAX_QPATH];
    Com_sprintf( jpgPath, sizeof( jpgPath ), "%s%s.jpg", 
                 cvar::objects::g_jxacScreenshotPath.svalue, baseFilename );
    
    // Write JPG file
    fileHandle_t f;
    trap_FS_FOpenFile( jpgPath, &f, FS_WRITE );
    
    if ( !f ) {
        return false;
    }
    
    const int written = trap_FS_Write( data, size, f );
    trap_FS_FCloseFile( f );

    // Both stock ET and Legacy return FS_Write's byte count. Check it after
    // closing; reopening a loose JPG for reading can be blocked by sv_pure.
    if (written != size) return false;
    logAction(clientNum, ent->client->pers.netname, std::string("Screenshot file saved: ") + jpgPath);
    
    // Gather player information for the text file
    char cleanname[64];
    Q_strncpyz( cleanname, ent->client->pers.netname, sizeof( cleanname ) );
    Q_CleanStr( cleanname );
    
    const char* coloredName = ent->client->pers.netname;
    
    // Get client version from userinfo
    char userinfo[MAX_INFO_STRING];
    char clientVersion[128] = "";
    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
    const char* clVersion = Info_ValueForKey( userinfo, "cg_etVersion" );
    if ( clVersion && clVersion[0] ) {
        Q_strncpyz( clientVersion, clVersion, sizeof( clientVersion ) );
    }
    
    // Strip port from IP address (e.g. "123.123.123.123:27960" -> "123.123.123.123")
    char ipNoPort[64] = "";
    if ( ip && ip[0] ) {
        Q_strncpyz( ipNoPort, ip, sizeof( ipNoPort ) );
        char* colon = strchr( ipNoPort, ':' );
        if ( colon ) {
            *colon = '\0';
        }
    }
    
    // Get screenshot reason
    jxacPlayerData_t* pd = &playerData[clientNum];
    const char* reason = pd->screenshotReason[0] ? pd->screenshotReason : "Requested by admin";
    
    // Build text file content
    char txtContent[1024];
    Com_sprintf( txtContent, sizeof( txtContent ),
                 "Date: %s\n"
                 "File: %s.jpg\n"
                 "Player: %s (%s)\n"
                 "IP: %s\n"
                 "XMODGUID: %s\n"
                 "MAC: %s\n"
                 "Client: %s\n"
                 "Reason: %s\n",
                 dateStr,
                 baseFilename,
                 cleanname, coloredName,
                 ipNoPort,
                 fullGuid,
                 mac,
                 clientVersion,
                 reason );
    
    // Write TXT file
    char txtPath[MAX_QPATH];
    Com_sprintf( txtPath, sizeof( txtPath ), "%s%s.txt", 
                 cvar::objects::g_jxacScreenshotPath.svalue, baseFilename );
    
    trap_FS_FOpenFile( txtPath, &f, FS_WRITE );
    if ( f ) {
        trap_FS_Write( txtContent, strlen( txtContent ), f );
        trap_FS_FCloseFile( f );
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void Server::logViolation( const jxacViolation_t* violation ) {
    if ( !violation || cvar::objects::g_jxacLogFile.svalue[0] == '\0' ) {
        return;
    }
    
    // Get timestamp
    time_t rawtime;
    struct tm* timeinfo;
    char timestamp[64];
    
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    strftime( timestamp, sizeof( timestamp ), "%Y-%m-%d %H:%M:%S", timeinfo );
    
    // Use the same mod-home path rules as the admin log, including creation of
    // the default jxac/ directory and explicit absolute custom filenames.
    std::string path;
    if (!serverlog::resolve(cvar::objects::g_jxacLogFile.svalue, path)) {
        Com_Printf( "JXAC: Failed to open log file: %s\n", cvar::objects::g_jxacLogFile.svalue );
        return;
    }
    std::ofstream file(path.c_str(), std::ios::out | std::ios::app | std::ios::binary);
    if (!file.is_open()) {
        Com_Printf("JXAC: Failed to open log file: %s\n", path.c_str());
        return;
    }
    
    // Write log entry
    char logEntry[512];
    Com_sprintf( logEntry, sizeof( logEntry ), 
                 "[%s] Client %d (%s) - Violation Type %d: %s\n",
                 timestamp, violation->clientNum, violation->playerName, 
                 violation->type, violation->details );
    
    file.write(logEntry, strlen(logEntry));
    file.flush();
    if (!file.good()) Com_Printf("JXAC: Failed to write log file: %s\n", path.c_str());
}

///////////////////////////////////////////////////////////////////////////////

void Server::checkHeartbeats() {
    // Get timeout from CVAR (default 60000ms)
    int timeout = cvar::objects::g_jxacHeartbeatTimeout.ivalue;
    if ( timeout <= 0 ) {
        timeout = 60000;  // Fallback to 60 seconds
    }
    
    for ( int i = 0; i < level.maxclients; i++ ) {
        gentity_t* ent = &g_entities[i];
        if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
            continue;
        }
        
        // Skip bots - they don't run JXAC client module
        if ( ent->r.svFlags & SVF_BOT ) {
            continue;
        }
        
        jxacPlayerData_t* pd = &playerData[i];

        // A backwards server clock must not retain a future heartbeat deadline.
        if (level.time < pd->lastHeartbeat) {
            pd->lastHeartbeat = level.time;
            pd->violationReported[JXAC_VIOLATION_NO_RESPONSE] = qfalse;
        }

        // Check heartbeat timeout
        if ( level.time - pd->lastHeartbeat > timeout ) {
            // Only report violation once (prevents spam)
            if ( !pd->violationReported[JXAC_VIOLATION_NO_RESPONSE] ) {
                reportViolation( i, JXAC_VIOLATION_NO_RESPONSE, "Heartbeat timeout" );
                pd->violationReported[JXAC_VIOLATION_NO_RESPONSE] = qtrue;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::checkTimeouts() {
    for ( int i = 0; i < level.maxclients; i++ ) {
        gentity_t* ent = &g_entities[i];
        
        // Skip bots - they don't run JXAC client module
        if ( ent->r.svFlags & SVF_BOT ) {
            continue;
        }
        
        jxacPlayerData_t* pd = &playerData[i];
        
        // Allow paced uploads while still bounding stalled and endless uploads.
        if ( pd->screenshotPending ) {
            int elapsed = level.time - pd->screenshotRequestTime;
            int inactive = level.time - screenshotLastProgress[i];
            if (inactive < 0 || elapsed < 0 || inactive > 30000 || elapsed > 180000) {
                
                if ( pd->ssBuffer ) {
                    free( pd->ssBuffer );
                    pd->ssBuffer = NULL;
                }
                
                pd->screenshotPending = qfalse;
                if (evidence[i].pending)
                    finishEvidence(i, "Screenshot unavailable: upload timeout");
                else
                    reportViolation( i, JXAC_VIOLATION_SS_BLOCKED, "Screenshot request timeout" );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::loadCvarConfig( const char* filename ) {
    if ( !filename || filename[0] == '\0' ) {
        Com_Printf( "JXAC: No CVAR config file specified\n" );
        return;
    }
    
    Com_Printf( "JXAC: Loading CVAR config from %s\n", filename );
    
    // TODO: Implement config file parsing
    // File format: cvar_name,expected_value,tolerance
    // Example: r_fullbright,0,0
    // For now, using hardcoded CVARs in protectedCvars array
    
    Com_Printf( "JXAC: CVAR config loading not yet implemented - using defaults\n" );
}

///////////////////////////////////////////////////////////////////////////////

bool Server::reloadConfig() {
    if (!initialized) return false;
    Com_Printf( "JXAC: Reloading configuration files\n" );
    
    // Reload CVAR config
    loadCvarConfig( cvar::objects::g_jxacCvarFile.svalue );
    
    // Reload forced CVAR config
    loadForceCvarConfig( cvar::objects::g_jxacForceCvarFile.svalue );
    
    // Reload cheat CVAR scanner config
    loadCheatCvarConfig( cvar::objects::g_jxacCheatCvarFile.svalue );
    
    // Reload cheat signature database
    loadCheatDatabase( cvar::objects::g_jxacCheatDbFile.svalue );
    const bool md5Loaded = loadMd5Config( cvar::objects::g_jxacMd5File.svalue );
    
    Com_Printf( "JXAC: Configuration reload complete\n" );
    return md5Loaded;
}

int Server::getMd5RuleCount() {
    return (int)md5Denylist.size();
}

///////////////////////////////////////////////////////////////////////////////

void Server::loadForceCvarConfig(const char* filename) {
    std::vector<ForcedCvar> parsed;
    fileHandle_t file = 0;
    int length = 0;
    if (filename && *filename) {
        length = trap_FS_FOpenFile(filename, &file, FS_READ);
        if (!file || length < 0 || length > 65536) {
            if (file) trap_FS_FCloseFile(file);
            Com_Printf("JXAC: Cannot load forced CVAR config %s; previous rules retained.\n", filename);
            return;
        }
    }
    std::vector<char> buffer(length + 1, 0);
    if (file) {
        if (length) trap_FS_Read(buffer.data(), length, file);
        trap_FS_FCloseFile(file);
    }
    char* line = strtok(buffer.data(), "\n");
    while (line) {
        char* cursor = line;
        const std::string directive = COM_ParseExt(&cursor, qfalse);
        const std::string name = COM_ParseExt(&cursor, qfalse);
        const std::string operation = COM_ParseExt(&cursor, qfalse);
        std::string value, maximum;
        bool range = false, valid = false;
        if (directive == "forcecvar") { value = operation; valid = true; }
        else if (directive == "sv_cvar" && (operation == "EQ" || operation == "IN")) {
            value = COM_ParseExt(&cursor, qfalse);
            range = operation == "IN";
            if (range) maximum = COM_ParseExt(&cursor, qfalse);
            valid = true;
        }
        double lo = 0, hi = 0;
        valid = valid && XmodCvarNameValid(name) && value.size() < 128 &&
                value.find_first_of("\"\r\n") == std::string::npos;
        if (range) valid = valid && XmodCvarNumber(value, lo) && XmodCvarNumber(maximum, hi) &&
                           lo <= hi && lo >= -FLT_MAX && hi <= FLT_MAX;
        if (valid) {
            ForcedCvar rule = {};
            Q_strncpyz(rule.name, name.c_str(), sizeof(rule.name));
            Q_strncpyz(rule.value, value.c_str(), sizeof(rule.value));
            rule.isRange = range;
            rule.minValue = (float)lo;
            rule.maxValue = (float)hi;
            bool replaced = false;
            for (auto& old : parsed) {
                if (!Q_stricmp(old.name, rule.name)) { old = rule; replaced = true; break; }
            }
            if (!replaced && parsed.size() < 256) parsed.push_back(rule);
        } else if (!directive.empty()) {
            Com_Printf("JXAC: Ignoring invalid forced CVAR rule: %s\n", line);
        }
        line = strtok(NULL, "\n");
    }
    forcedCvars.swap(parsed);
    for (int i = 0; i < level.maxclients; ++i)
        if (level.clients[i].pers.connected == CON_CONNECTED) queueForcedCvars(i);
    Com_Printf("JXAC: Loaded %d forced CVARs.\n", (int)forcedCvars.size());
}

///////////////////////////////////////////////////////////////////////////////

void Server::checkForcedCvar( int clientNum, const char* cvarName, const char* value ) {
    for ( const auto& fcvar : forcedCvars ) {
        if ( Q_stricmp( fcvar.name, cvarName ) == 0 ) {
            if ( fcvar.isRange ) {
                double fval;
                if (!XmodCvarNumber(value, fval) || fval < fcvar.minValue || fval > fcvar.maxValue) {
                    // Keep the range rule: forcing one clamped value would prevent
                    // players from changing to another permitted value later.
                    trap_SendServerCommand(clientNum, va("fcr \"%s\" %.9g %.9g", fcvar.name, fcvar.minValue, fcvar.maxValue));
                }
            } else {
                if ( Q_stricmp( value, fcvar.value ) != 0 ) {
                    // Mismatch - re-send the forced value to the client
                    trap_SendServerCommand( clientNum, va("fc \"%s\" \"%s\"", fcvar.name, fcvar.value) );
                }
            }
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::loadCheatCvarConfig( const char* filename ) {
    if ( !filename || filename[0] == '\0' ) {
        Com_Printf( "JXAC: No cheat CVAR config file specified\n" );
        return;
    }
    
    fileHandle_t f;
    int len = trap_FS_FOpenFile( filename, &f, FS_READ );
    
    if ( !f || len <= 0 ) {
        Com_Printf( "JXAC: Failed to load cheat CVAR config: %s\n", filename );
        return;
    }
    
    cheatCvars.clear();
    
    char* buffer = (char*)malloc( len + 1 );
    if ( !buffer ) {
        Com_Printf( "JXAC: Failed to allocate memory for cheat CVAR config\n" );
        trap_FS_FCloseFile( f );
        return;
    }
    
    trap_FS_Read( buffer, len, f );
    buffer[len] = '\0';
    trap_FS_FCloseFile( f );
    
    // Parse line by line (format: cvar_name,action)
    char* line = strtok( buffer, "\n" );
    while ( line ) {
        while ( *line == ' ' || *line == '\t' ) line++;
        if ( *line == '/' || *line == '\0' ) {
            line = strtok( NULL, "\n" );
            continue;
        }
        
        CheatCvar ccvar;
        memset( &ccvar, 0, sizeof( ccvar ) );
        
        if ( sscanf( line, "%63[^,],%15s", ccvar.name, ccvar.action ) == 2 ) {
            cheatCvars.push_back( ccvar );
        }
        
        line = strtok( NULL, "\n" );
    }
    
    free( buffer );
    Com_Printf( "JXAC: Loaded %d cheat CVARs from %s\n", (int)cheatCvars.size(), filename );
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestCheatCvarScan( int clientNum ) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
        return;
    }
    
    // Skip bots
    if ( ent->r.svFlags & SVF_BOT ) {
        return;
    }
    
    // Send all cheat CVAR names to client for scanning
    for ( const auto& ccvar : cheatCvars ) {
        trap_SendServerCommand( clientNum, va( "jxac_cvar_req %s", ccvar.name ) );
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::handleCheatCvarResponse( int clientNum, const char* cvarName, const char* value ) {
    if ( !activeHuman(clientNum) || !cvarName || !value || evidence[clientNum].finished ) {
        return;
    }
    
    // If CVAR exists and is not empty/null, it's a cheat CVAR
    if ( !value || value[0] == '\0' ) {
        return; // CVAR doesn't exist - OK
    }
    
    for ( const auto& ccvar : cheatCvars ) {
        if ( Q_stricmp( ccvar.name, cvarName ) == 0 ) {
            char details[256];
            Com_sprintf( details, sizeof( details ), 
                       "Cheat CVAR detected: '%s' = '%s'", cvarName, value );
            
            if ( Q_stricmp( ccvar.action, "ban" ) == 0 ) {
                if (!evidence[clientNum].pending) recordViolation( clientNum, JXAC_VIOLATION_TAMPER, details );
                beginEvidence(clientNum, true, "Cheat CVAR detected", details);
            } else if ( Q_stricmp( ccvar.action, "kick" ) == 0 ) {
                if (!evidence[clientNum].pending) recordViolation( clientNum, JXAC_VIOLATION_TAMPER, details );
                beginEvidence(clientNum, false, "Cheat CVAR detected", details);
            } else if ( Q_stricmp( ccvar.action, "none" ) != 0 ) {
                recordViolation( clientNum, JXAC_VIOLATION_TAMPER, details );
            }
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::loadCheatDatabase( const char* filename ) {
    if ( !filename || filename[0] == '\0' ) {
        Com_Printf( "JXAC: No cheat database file specified\n" );
        return;
    }
    
    fileHandle_t f;
    int len = trap_FS_FOpenFile( filename, &f, FS_READ );
    
    if ( !f || len <= 0 ) {
        Com_Printf( "JXAC: Failed to load cheat database: %s\n", filename );
        return;
    }
    
    cheatSignatures.clear();
    
    char* buffer = (char*)malloc( len + 1 );
    if ( !buffer ) {
        Com_Printf( "JXAC: Failed to allocate memory for cheat database\n" );
        trap_FS_FCloseFile( f );
        return;
    }
    
    trap_FS_Read( buffer, len, f );
    buffer[len] = '\0';
    trap_FS_FCloseFile( f );
    
    // Parse line by line (format: type,name,checksum,action)
    char* line = strtok( buffer, "\n" );
    while ( line ) {
        while ( *line == ' ' || *line == '\t' ) line++;
        if ( *line == '/' || *line == '\0' ) {
            line = strtok( NULL, "\n" );
            continue;
        }
        
        CheatSignature sig;
        memset( &sig, 0, sizeof( sig ) );
        
        if ( sscanf( line, "%15[^,],%255[^,],%63[^,],%15s", 
                   sig.type, sig.name, sig.checksum, sig.action ) == 4 ) {
            cheatSignatures.push_back( sig );
        }
        
        line = strtok( NULL, "\n" );
    }
    
    free( buffer );
    Com_Printf( "JXAC: Loaded %d cheat signatures from %s\n", 
               (int)cheatSignatures.size(), filename );
}

///////////////////////////////////////////////////////////////////////////////

bool Server::loadMd5Config(const char* filename) {
    if (!filename || !*filename) {
        Com_Printf("JXAC: MD5 list path is empty; keeping %d active hashes\n", (int)md5Denylist.size());
        return false;
    }
    fileHandle_t file = 0;
    const int length = trap_FS_FOpenFile(filename, &file, FS_READ);
    if (!file || length < 0 || (size_t)length > md5rules::MAX_FILE_BYTES) {
        if (file) trap_FS_FCloseFile(file);
        Com_Printf("JXAC: Cannot read MD5 list %s (missing, unreadable or over 1 MiB); keeping %d active hashes\n",
            filename, (int)md5Denylist.size());
        return false;
    }
    // FS_Read has no return value. Unfilled bytes remain NUL and fail parsing.
    std::string contents((size_t)length, '\0');
    if (length) trap_FS_Read(&contents[0], length, file);
    trap_FS_FCloseFile(file);
    md5rules::Rules parsed;
    size_t line = 0;
    std::string error;
    if (!md5rules::parse(contents, parsed, line, error)) {
        Com_Printf("JXAC: Invalid MD5 list %s:%d: %s; keeping %d active hashes\n",
            filename, (int)line, error.c_str(), (int)md5Denylist.size());
        return false;
    }
    md5Denylist.swap(parsed);
    Com_Printf("JXAC: Loaded %d MD5 hashes from %s\n", (int)md5Denylist.size(), filename);
    for (int i = 0; i < MAX_CLIENTS; ++i) checkCachedModuleMd5(i);
    return true;
}

void Server::checkCachedModuleMd5(int clientNum) {
    if (!moduleScanActive(clientNum)) return;
    for (const auto& module : moduleDigests[clientNum]) {
        const auto match = md5Denylist.find(module.md5);
        if (match == md5Denylist.end()) continue;
        const std::string details = "MD5 " + module.md5 + " module " + md5rules::safeText(module.name) +
            (match->second.empty() ? "" : " [" + match->second + "]");
        reportGamehack(clientNum, details.c_str());
        return;
    }
}

const char* Server::getMemoryStatusString(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) return "invalid";
    switch (memoryScanState[clientNum]) {
        case MemoryScanState::Scanning: return "scanning";
        case MemoryScanState::Complete: return "complete";
        case MemoryScanState::Partial: return "partial";
        case MemoryScanState::Unsupported: return "unsupported";
        default: return "not reported";
    }
}

void Server::handleMemoryStatus(int clientNum, const char* status) {
    if (!moduleScanActive(clientNum) || !status) return;
    MemoryScanState state;
    if (!strcmp(status, "scanning")) state = MemoryScanState::Scanning;
    else if (!strcmp(status, "complete")) state = MemoryScanState::Complete;
    else if (!strcmp(status, "partial")) state = MemoryScanState::Partial;
    else if (!strcmp(status, "unsupported")) state = MemoryScanState::Unsupported;
    else return;
    // Reporting capabilities or finishing a scan is not an attestation. There
    // is no timeout penalty for unsupported or unreported memory scanning.
    memoryScanState[clientNum] = state;
}

void Server::handleMemoryHit(int clientNum, const char* ruleId) {
    if (!moduleScanActive(clientNum) || !ruleId || strlen(ruleId) > 31) return;
    const char* label = memoryrules::knownRuleLabel(ruleId);
    if (!label) return;
    // Only fixed rule identifiers cross the protocol. All diagnostic text is
    // supplied by the server; no memory bytes, paths or arbitrary text arrive.
    const std::string details = "Memory fingerprint " + std::string(ruleId) + " [" + label + "]";
    reportGamehack(clientNum, details.c_str());
}

void Server::handleModuleMd5(int clientNum, const char* md5, const char* sha1, const char* encodedBasename) {
    if (!moduleScanActive(clientNum) || !md5 || !sha1 || !encodedBasename) return;
    ModuleDigest module;
    if (!md5rules::normalizeHash(md5, 32, module.md5) ||
        !md5rules::normalizeHash(sha1, 40, module.sha1) ||
        !md5rules::decodeBasename(encodedBasename, module.name)) return;
    if (validMd5Reports[clientNum] < UINT_MAX) ++validMd5Reports[clientNum];

    // Check before caching so no flood of other reports can hide a match.
    const auto match = md5Denylist.find(module.md5);
    if (match != md5Denylist.end()) {
        const std::string details = "MD5 " + module.md5 + " module " + md5rules::safeText(module.name) +
            (match->second.empty() ? "" : " [" + match->second + "]");
        reportGamehack(clientNum, details.c_str());
        return;
    }
    auto& cached = moduleDigests[clientNum];
    for (const auto& previous : cached) {
        if (previous.md5 == module.md5 && previous.sha1 == module.sha1 && previous.name == module.name) return;
    }
    if (cached.size() < MAX_CACHED_MODULES) cached.push_back(module);
    else {
        cached[nextModuleDigest[clientNum]] = module;
        nextModuleDigest[clientNum] = (nextModuleDigest[clientNum] + 1) % MAX_CACHED_MODULES;
    }
    checkModuleSignature(clientNum, module.name.c_str(), module.sha1.c_str());
}

void Server::checkModuleSignature( int clientNum, const char* moduleName, const char* checksum ) {
    if (!moduleScanActive(clientNum) || !moduleName || !checksum || evidence[clientNum].finished) return;
    std::string digest;
    if (!md5rules::validBasename(moduleName) || !md5rules::normalizeHash(checksum, 40, digest)) return;
    
    for ( const auto& sig : cheatSignatures ) {
        // Check if type matches (dll/exe)
        bool typeMatch = false;
        if ( Q_stricmp( sig.type, "dll" ) == 0 && 
            ( Q_stristr( moduleName, ".dll" ) || Q_stristr( moduleName, ".so" ) ) ) {
            typeMatch = true;
        } else if ( Q_stricmp( sig.type, "exe" ) == 0 && 
                   Q_stristr( moduleName, ".exe" ) ) {
            typeMatch = true;
        } else if ( Q_stricmp( sig.type, "process" ) == 0 ) {
            typeMatch = true;
        }
        
        if ( !typeMatch ) continue;
        
        // Check name match (case-insensitive, partial match)
        if ( strcmp(sig.name, "*") != 0 && Q_stristr( moduleName, sig.name ) == NULL ) continue;
        
        // Check checksum (if not wildcard)
        if ( strcmp(sig.checksum, "*") == 0 ) {
            // Wildcard - any DLL with this name is banned
        } else if ( Q_stricmp( digest.c_str(), sig.checksum ) != 0 ) {
            continue; // Checksum mismatch
        }
        
        // Found match - take action
        char details[512];
        Com_sprintf( details, sizeof( details ), 
                   "Cheat module detected: %s (SHA1: %s)", md5rules::safeText(moduleName).c_str(), digest.c_str() );
        
        if ( Q_stricmp( sig.action, "ban" ) == 0 ) {
            if (!evidence[clientNum].pending) recordViolation( clientNum, JXAC_VIOLATION_CHECKSUM, details );
            beginEvidence(clientNum, true, "Cheat module detected", details);
        } else if ( Q_stricmp( sig.action, "kick" ) == 0 ) {
            if (!evidence[clientNum].pending) recordViolation( clientNum, JXAC_VIOLATION_CHECKSUM, details );
            beginEvidence(clientNum, false, "Cheat module detected", details);
        } else if ( Q_stricmp( sig.action, "none" ) != 0 ) {
            recordViolation( clientNum, JXAC_VIOLATION_CHECKSUM, details );
        }
        
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::sendBinaryMessage( int clientNum, jxacMessageType_t type, const void* data, int dataLen ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    // Build binary message with header
    char msgBuf[MAX_BINARY_MESSAGE];
    jxacBinaryHeader_t* header = (jxacBinaryHeader_t*)msgBuf;
    
    // Validate data length fits (use size_t for consistent comparison)
    if ( (size_t)dataLen + sizeof(jxacBinaryHeader_t) > MAX_BINARY_MESSAGE ) {
        return;
    }
    
    header->magic = JXAC_BINARY_MAGIC;
    header->type = (unsigned short)type;
    header->dataLen = (unsigned short)dataLen;
    
    if ( data && dataLen > 0 ) {
        memcpy( msgBuf + sizeof(jxacBinaryHeader_t), data, dataLen );
    }
    
    // Send via binary message channel
    trap_SendMessage( clientNum, msgBuf, sizeof(jxacBinaryHeader_t) + dataLen );
}

///////////////////////////////////////////////////////////////////////////////

// Helper function to find null terminator within bounds
static int findNullTerminator( const char* data, int maxLen ) {
    for ( int i = 0; i < maxLen; i++ ) {
        if ( data[i] == '\0' ) {
            return i;
        }
    }
    return -1;  // Not found
}

void Server::handleBinaryMessage( int clientNum, const char* buf, int buflen ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS || !buf || buflen < (int)sizeof(jxacBinaryHeader_t) ) {
        return;
    }
    
    const jxacBinaryHeader_t* header = (const jxacBinaryHeader_t*)buf;
    
    // Validate magic
    if ( header->magic != JXAC_BINARY_MAGIC ) {
        return;
    }
    
    // Validate data length
    if ( header->dataLen + sizeof(jxacBinaryHeader_t) > (unsigned int)buflen ) {
        return;
    }
    
    const char* data = buf + sizeof(jxacBinaryHeader_t);
    
    switch ( header->type ) {
        case JXAC_MSG_HEARTBEAT:
            handleHeartbeat( clientNum );
            break;
            
        case JXAC_MSG_SS_DATA:
            // Binary screenshot data
            handleScreenshotData( clientNum, data, header->dataLen );
            break;
            
        case JXAC_MSG_SS_COMPLETE:
            handleScreenshotComplete( clientNum );
            break;
            
        case JXAC_MSG_CVAR_RESPONSE:
            // CVAR response - data format: "name\0value\0"
            if ( header->dataLen > 0 ) {
                // Find first null terminator
                int nameEnd = findNullTerminator( data, header->dataLen );
                if ( nameEnd >= 0 && nameEnd < header->dataLen - 1 ) {
                    const char* cvarName = data;
                    const char* cvarValue = data + nameEnd + 1;
                    // Verify value is also null-terminated within bounds
                    int valueEnd = findNullTerminator( cvarValue, header->dataLen - nameEnd - 1 );
                    if ( valueEnd >= 0 ) {
                        handleCvarResponse( clientNum, cvarName, cvarValue );
                    }
                }
            }
            break;
            
        case JXAC_MSG_MODULE:
            // Module info - data format: "name\0checksum\0"
            if ( header->dataLen > 0 ) {
                // Find first null terminator
                int nameEnd = findNullTerminator( data, header->dataLen );
                if ( nameEnd >= 0 && nameEnd < header->dataLen - 1 ) {
                    const char* moduleName = data;
                    const char* checksum = data + nameEnd + 1;
                    // Verify checksum is also null-terminated within bounds
                    int checksumEnd = findNullTerminator( checksum, header->dataLen - nameEnd - 1 );
                    if ( checksumEnd >= 0 ) {
                        checkModuleSignature( clientNum, moduleName, checksum );
                    }
                }
            }
            break;
            
        case JXAC_MSG_MODULE_COMPLETE:
            // Module scan complete - nothing to do
            break;
            
        case JXAC_MSG_VIOLATION:
            // Client-reported violation - use memcpy for alignment safety
            if ( header->dataLen >= 4 ) {
                int violationTypeInt;
                memcpy( &violationTypeInt, data, sizeof(violationTypeInt) );
                jxacViolationType_t violationType = (jxacViolationType_t)violationTypeInt;
                if (violationType == JXAC_VIOLATION_GAMEHACK &&
                    (!cvar::objects::g_jxacCheckWallhack.ivalue || !cvar::objects::g_jxacAntiTamper.ivalue)) break;
                const char* details = "";
                if ( header->dataLen > 4 ) {
                    // Verify details string is null-terminated
                    int detailsEnd = findNullTerminator( data + 4, header->dataLen - 4 );
                    if ( detailsEnd >= 0 ) {
                        details = data + 4;
                    }
                }
                reportViolation( clientNum, violationType, details );
            }
            break;
            
        default:
            // Unknown message type - ignore
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
