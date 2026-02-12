#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <game/jxac/jxac_server.h>
#include <game/jxac/jxac_tcp_server.h>
#include <vector>
#include <cstdlib>
#include <cstring>

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

// Expected values for protected CVARs (cvarName, expectedValue, exactMatch)
// Only CVARs that need specific value validation are listed here
// Other CVARs in batches are just logged for monitoring
static const jxacCvarCheck_t protectedCvars[] = {
    // Batch 1 - Wallhack related (critical)
    { "r_drawentities", "1", qtrue },
    { "r_drawworld", "1", qtrue },
    { "r_fullbright", "0", qtrue },
    { "r_lightmap", "0", qtrue },
    { "r_showimages", "0", qtrue },
    { "r_shownormals", "0", qtrue },
    { "r_showtris", "0", qtrue },
    { "r_showsky", "1", qtrue },
    { "r_fastsky", "0", qtrue },
    // Batch 2 - Visibility related
    { "r_znear", "4", qfalse },      // Allow values close to 4
    { "r_nocull", "0", qtrue },
    { "r_drawfoliage", "1", qtrue },
    { "r_noportals", "0", qtrue },
    { "r_mapoverbrightbits", "2", qfalse },  // Allow 2 or 3
    { "r_intensity", "1", qfalse },          // Tolerance ±0.5
    // Batch 3 - Client misc
    { "cg_thirdPerson", "0", qtrue },
    { "cg_shadows", "1", qfalse },   // Allow 0-1
    // Batch 4 - Textures
    { "r_picmip", "0", qfalse },     // Allow 0-2
    { "", "", qfalse }  // Terminator
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
    
    // Initialize CVAR batch indexes
    memset( currentCvarBatch, 0, sizeof( currentCvarBatch ) );
    memset( currentForcedCvarOffset, 0, sizeof( currentForcedCvarOffset ) );
    
    // Initialize rate limiting arrays (security: prevent disk fill attacks)
    memset( lastScreenshotTime, 0, sizeof( lastScreenshotTime ) );
    memset( screenshotsThisHour, 0, sizeof( screenshotsThisHour ) );
    memset( hourStartTime, 0, sizeof( hourStartTime ) );
    
    // Load CVAR config file (if exists)
    loadCvarConfig( cvar::objects::g_jxacCvarFile.svalue );
    
    // Load forced CVAR config
    loadForceCvarConfig( cvar::objects::g_jxacForceCvarFile.svalue );
    
    // Load cheat CVAR scanner config
    loadCheatCvarConfig( cvar::objects::g_jxacCheatCvarFile.svalue );
    
    // Load cheat signature database
    loadCheatDatabase( cvar::objects::g_jxacCheatDbFile.svalue );
    
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
    
    // Free any allocated screenshot buffers
    for ( int i = 0; i < MAX_CLIENTS; i++ ) {
        if ( playerData[i].ssBuffer ) {
            free( playerData[i].ssBuffer );
            playerData[i].ssBuffer = NULL;
        }
    }
    
    initialized = qfalse;
}

///////////////////////////////////////////////////////////////////////////////

void Server::frame() {
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
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    // Initialize player data
    jxacPlayerData_t* pd = &playerData[clientNum];
    memset( pd, 0, sizeof( jxacPlayerData_t ) );
    
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
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    // Free screenshot buffer if allocated
    if ( pd->ssBuffer ) {
        free( pd->ssBuffer );
        pd->ssBuffer = NULL;
    }
    
    // Clear player data
    memset( pd, 0, sizeof( jxacPlayerData_t ) );
}

///////////////////////////////////////////////////////////////////////////////

void Server::clientBegin( int clientNum ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    // Send JXAC status check to client
    // This would normally send a network message to the client
    // For now, just mark as verified (placeholder)
    playerData[clientNum].status |= JXAC_STATUS_VERIFIED | JXAC_STATUS_CLEAN;
    
    // Send all forced CVARs to this client so they can enforce them locally
    // This follows the NitMod pattern: server sends "fc" commands on client begin
    gentity_t* ent = &g_entities[clientNum];
    if ( ent->r.svFlags & SVF_BOT ) {
        return;
    }
    for ( int i = 0; i < (int)forcedCvars.size(); i++ ) {
        if ( !forcedCvars[i].isRange ) {
            trap_SendServerCommand( clientNum, va("fc \"%s\" \"%s\"", forcedCvars[i].name, forcedCvars[i].value) );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestScreenshot( int clientNum, int quality ) {
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
    
    if ( pd->screenshotPending ) {
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
    
    Com_Printf( "JXAC: Scheduled screenshot for client %d in %d ms (quality: %d)\n", 
                clientNum, randomDelay, quality );
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestScreenshotAll( int quality ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    Com_Printf( "JXAC: Requesting screenshots from all players\n" );
    
    for ( int i = 0; i < level.maxclients; i++ ) {
        gentity_t* ent = &g_entities[i];
        if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
            continue;
        }
        
        requestScreenshot( i, quality );
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
            return;
        }
    }
    
    // Check bounds
    if ( pd->ssDataReceived + size > JXAC_SS_MAX_SIZE ) {
        free( pd->ssBuffer );
        pd->ssBuffer = NULL;
        pd->screenshotPending = qfalse;
        reportViolation( clientNum, JXAC_VIOLATION_SS_BLOCKED, "Screenshot too large" );
        return;
    }
    
    // Copy data to buffer
    memcpy( pd->ssBuffer + pd->ssDataReceived, data, size );
    pd->ssDataReceived += size;
    
}

///////////////////////////////////////////////////////////////////////////////

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
    
    // Security: Validate JPEG header (SOI marker: 0xFF 0xD8)
    if ( pd->ssDataReceived < 3 || 
         pd->ssBuffer[0] != 0xFF || 
         pd->ssBuffer[1] != 0xD8 ) {
        free( pd->ssBuffer );
        pd->ssBuffer = NULL;
        pd->screenshotPending = qfalse;
        reportViolation( clientNum, JXAC_VIOLATION_SS_BLOCKED, "Invalid screenshot data (not JPEG)" );
        return;
    }
    
    // Security: Validate JPEG footer (EOI marker: 0xFF 0xD9) - optional but recommended
    if ( pd->ssDataReceived >= 2 ) {
        if ( pd->ssBuffer[pd->ssDataReceived - 2] != 0xFF || 
             pd->ssBuffer[pd->ssDataReceived - 1] != 0xD9 ) {
            // Just warn, don't reject - some JPEGs might have trailing data
        }
    }
    
    
    // Update rate limiting counters
    lastScreenshotTime[clientNum] = level.time;
    screenshotsThisHour[clientNum]++;
    
    // Save screenshot to disk
    saveScreenshot( clientNum, pd->ssBuffer, pd->ssDataReceived );
    
    // Clean up
    free( pd->ssBuffer );
    pd->ssBuffer = NULL;
    pd->screenshotPending = qfalse;
    
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
    
    // Check against protected CVARs list
    for ( int i = 0; protectedCvars[i].name[0] != '\0'; i++ ) {
        if ( Q_stricmp( protectedCvars[i].name, cvarName ) == 0 ) {
            qboolean violation = qfalse;
            
            if ( protectedCvars[i].exactMatch ) {
                // Exact match required
                if ( Q_stricmp( protectedCvars[i].expectedValue, value ) != 0 ) {
                    violation = qtrue;
                }
            } else {
                // Check if value is within acceptable range (for numeric values)
                // First verify both values are actually numeric
                char* endptr1 = NULL;
                char* endptr2 = NULL;
                float expected = strtof( protectedCvars[i].expectedValue, &endptr1 );
                float actual = strtof( value, &endptr2 );
                
                // Only apply tolerance if both values parsed as valid numbers (check endptr points past input)
                if ( endptr1 != NULL && endptr1 != protectedCvars[i].expectedValue && *endptr1 == '\0' &&
                     endptr2 != NULL && endptr2 != value && *endptr2 == '\0' ) {
                    // Both are numeric - allow some tolerance for non-exact matches
                    if ( fabs( expected - actual ) > 0.5f ) {
                        violation = qtrue;
                    }
                } else {
                    // At least one is non-numeric - do string comparison
                    if ( Q_stricmp( protectedCvars[i].expectedValue, value ) != 0 ) {
                        violation = qtrue;
                    }
                }
            }
            
            if ( violation ) {
                char details[256];
                Com_sprintf( details, sizeof(details), "Illegal CVAR: %s=%s (expected %s)", 
                            cvarName, value, protectedCvars[i].expectedValue );
                reportViolation( clientNum, JXAC_VIOLATION_CVAR, details );
            }
            
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::reportViolation( int clientNum, jxacViolationType_t type, const char* details ) {
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
    Q_strncpyz( violation.playerName, ent->client->pers.netname, sizeof( violation.playerName ) );
    Q_strncpyz( violation.details, details ? details : "", sizeof( violation.details ) );
    
    logViolation( &violation );
    
    Com_Printf( "^1JXAC VIOLATION: Client %d (%s) - Type %d: %s\n", 
                clientNum, ent->client->pers.netname, type, details ? details : "N/A" );
    
    // Auto-actions
    if ( cvar::objects::g_jxacAutoBan.ivalue ) {
        banPlayer( clientNum, va( "JXAC Violation: %s", details ? details : "Cheating detected" ) );
    } else if ( cvar::objects::g_jxacAutoKick.ivalue ) {
        kickPlayer( clientNum, va( "JXAC Violation: %s", details ? details : "Cheating detected" ) );
    }
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
    
    if ( pd->status & JXAC_STATUS_CLEAN ) {
        return "^2CLEAN";
    }
    
    if ( pd->status & JXAC_STATUS_VERIFIED ) {
        return "^5VERIFIED";
    }
    
    if ( pd->status & JXAC_STATUS_CONNECTED ) {
        return "^6CONNECTED";
    }
    
    return "^7UNKNOWN";
}

///////////////////////////////////////////////////////////////////////////////

void Server::printStatus( int clientNum ) {
    if ( !initialized ) {
        Com_Printf( "JXAC: Not initialized\n" );
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        Com_Printf( "JXAC: Invalid client number\n" );
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
        Com_Printf( "JXAC: Client not connected\n" );
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    Com_Printf( "JXAC Status for client %d (%s):\n", clientNum, ent->client->pers.netname );
    Com_Printf( "  Status: %s\n", getStatusString( clientNum ) );
    Com_Printf( "  Violations: %d\n", pd->violations );
    Com_Printf( "  Last Heartbeat: %d ms ago\n", level.time - pd->lastHeartbeat );
    
    if ( pd->lastViolation != JXAC_VIOLATION_NONE ) {
        Com_Printf( "  Last Violation: Type %d at %d\n", pd->lastViolation, pd->lastViolationTime );
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::printStatusAll() {
    if ( !initialized ) {
        Com_Printf( "JXAC: Not initialized\n" );
        return;
    }
    
    Com_Printf( "JXAC Status (v%s) - Enabled: %s\n", 
                JXAC_VERSION_STRING, cvar::objects::g_jxacEnable.ivalue ? "YES" : "NO" );
    Com_Printf( "%-4s %-32s %-12s %-10s\n", "Slot", "Name", "Status", "Violations" );
    Com_Printf( "------------------------------------------------------------\n" );
    
    for ( int i = 0; i < level.maxclients; i++ ) {
        gentity_t* ent = &g_entities[i];
        if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
            continue;
        }
        
        jxacPlayerData_t* pd = &playerData[i];
        Com_Printf( "%-4d %-32s %-12s %-10d\n", 
                    i, 
                    ent->client->pers.netname, 
                    getStatusString( i ), 
                    pd->violations );
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
    
    Com_Printf( "JXAC: Banning client %d (%s): %s\n", 
                clientNum, ent->client->pers.netname, reason ? reason : "JXAC Violation" );
    
    // Mark as banned
    playerData[clientNum].status |= JXAC_STATUS_BANNED;
    
    // Drop client
    trap_DropClient( clientNum, reason ? reason : "JXAC Ban", 0 );
}

///////////////////////////////////////////////////////////////////////////////

void Server::saveScreenshot( int clientNum, const unsigned char* data, int size ) {
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client ) {
        return;
    }
    
    // Create filename: <playername>_<guid>_<timestamp>.jpg
    char filename[MAX_QPATH];
    char cleanname[64];
    
    // Clean player name (remove color codes and special chars)
    Q_strncpyz( cleanname, ent->client->pers.netname, sizeof( cleanname ) );
    Q_CleanStr( cleanname );
    
    // Get timestamp
    time_t rawtime;
    struct tm* timeinfo;
    char timestamp[32];
    
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    strftime( timestamp, sizeof( timestamp ), "%Y%m%d_%H%M%S", timeinfo );
    
    // Build full path
    Com_sprintf( filename, sizeof( filename ), "%s%s_%s.jpg", 
                 cvar::objects::g_jxacScreenshotPath.svalue, cleanname, timestamp );
    
    
    // Write file
    fileHandle_t f;
    trap_FS_FOpenFile( filename, &f, FS_WRITE );
    
    if ( !f ) {
        return;
    }
    
    trap_FS_Write( data, size, f );
    trap_FS_FCloseFile( f );
    
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
    
    // Open log file for append
    fileHandle_t f;
    trap_FS_FOpenFile( cvar::objects::g_jxacLogFile.svalue, &f, FS_APPEND );
    
    if ( !f ) {
        Com_Printf( "JXAC: Failed to open log file: %s\n", cvar::objects::g_jxacLogFile.svalue );
        return;
    }
    
    // Write log entry
    char logEntry[512];
    Com_sprintf( logEntry, sizeof( logEntry ), 
                 "[%s] Client %d (%s) - Violation Type %d: %s\n",
                 timestamp, violation->clientNum, violation->playerName, 
                 violation->type, violation->details );
    
    trap_FS_Write( logEntry, strlen( logEntry ), f );
    trap_FS_FCloseFile( f );
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
        
        // Check screenshot request timeout (30 seconds)
        if ( pd->screenshotPending ) {
            int elapsed = level.time - pd->screenshotRequestTime;
            if ( elapsed > 30000 ) {
                
                if ( pd->ssBuffer ) {
                    free( pd->ssBuffer );
                    pd->ssBuffer = NULL;
                }
                
                pd->screenshotPending = qfalse;
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

void Server::reloadConfig() {
    Com_Printf( "JXAC: Reloading configuration files\n" );
    
    // Reload CVAR config
    loadCvarConfig( cvar::objects::g_jxacCvarFile.svalue );
    
    // Reload forced CVAR config
    loadForceCvarConfig( cvar::objects::g_jxacForceCvarFile.svalue );
    
    // Reload cheat CVAR scanner config
    loadCheatCvarConfig( cvar::objects::g_jxacCheatCvarFile.svalue );
    
    // Reload cheat signature database
    loadCheatDatabase( cvar::objects::g_jxacCheatDbFile.svalue );
    
    Com_Printf( "JXAC: Configuration reload complete\n" );
}

///////////////////////////////////////////////////////////////////////////////

void Server::loadForceCvarConfig( const char* filename ) {
    if ( !filename || filename[0] == '\0' ) {
        Com_Printf( "JXAC: No forced CVAR config file specified\n" );
        return;
    }
    
    fileHandle_t f;
    int len = trap_FS_FOpenFile( filename, &f, FS_READ );
    
    if ( !f || len <= 0 ) {
        Com_Printf( "JXAC: Failed to load forced CVAR config: %s\n", filename );
        return;
    }
    
    forcedCvars.clear();
    
    char* buffer = (char*)malloc( len + 1 );
    if ( !buffer ) {
        Com_Printf( "JXAC: Failed to allocate memory for forced CVAR config\n" );
        trap_FS_FCloseFile( f );
        return;
    }
    
    trap_FS_Read( buffer, len, f );
    buffer[len] = '\0';
    trap_FS_FCloseFile( f );
    
    // Parse line by line
    char* line = strtok( buffer, "\n" );
    while ( line ) {
        // Skip comments and empty lines
        while ( *line == ' ' || *line == '\t' ) line++;
        if ( *line == '/' || *line == '\0' ) {
            line = strtok( NULL, "\n" );
            continue;
        }
        
        ForcedCvar fcvar;
        memset( &fcvar, 0, sizeof( fcvar ) );
        
        // Parse "forcecvar <cvar> <value>"
        if ( sscanf( line, "forcecvar %63s %127s", fcvar.name, fcvar.value ) == 2 ) {
            fcvar.isRange = false;
            forcedCvars.push_back( fcvar );
        }
        // Parse "sv_cvar <cvar> IN <min> <max>"
        else if ( sscanf( line, "sv_cvar %63s IN %f %f", fcvar.name, &fcvar.minValue, &fcvar.maxValue ) == 3 ) {
            fcvar.isRange = true;
            forcedCvars.push_back( fcvar );
        }
        // Parse "sv_cvar <cvar> EQ <value>"
        else if ( sscanf( line, "sv_cvar %63s EQ %127s", fcvar.name, fcvar.value ) == 2 ) {
            fcvar.isRange = false;
            forcedCvars.push_back( fcvar );
        }
        
        line = strtok( NULL, "\n" );
    }
    
    free( buffer );
    Com_Printf( "JXAC: Loaded %d forced CVARs from %s\n", (int)forcedCvars.size(), filename );
}

///////////////////////////////////////////////////////////////////////////////

void Server::checkForcedCvar( int clientNum, const char* cvarName, const char* value ) {
    for ( const auto& fcvar : forcedCvars ) {
        if ( Q_stricmp( fcvar.name, cvarName ) == 0 ) {
            if ( fcvar.isRange ) {
                float fval = atof( value );
                if ( fval < fcvar.minValue || fval > fcvar.maxValue ) {
                    char details[256];
                    Com_sprintf( details, sizeof( details ), 
                               "Forced CVAR '%s' out of range: %.2f (must be %.2f-%.2f)",
                               cvarName, fval, fcvar.minValue, fcvar.maxValue );
                    reportViolation( clientNum, JXAC_VIOLATION_CVAR, details );
                }
            } else {
                if ( Q_stricmp( value, fcvar.value ) != 0 ) {
                    char details[256];
                    Com_sprintf( details, sizeof( details ), 
                               "Forced CVAR '%s' mismatch: '%s' (must be '%s')",
                               cvarName, value, fcvar.value );
                    reportViolation( clientNum, JXAC_VIOLATION_CVAR, details );
                    // Re-send the forced value to the client
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
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS || !cvarName || !value ) {
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
                reportViolation( clientNum, JXAC_VIOLATION_TAMPER, details );
                banPlayer( clientNum, "Cheat CVAR detected" );
            } else if ( Q_stricmp( ccvar.action, "kick" ) == 0 ) {
                reportViolation( clientNum, JXAC_VIOLATION_TAMPER, details );
                kickPlayer( clientNum, "Cheat CVAR detected" );
            } else {
                reportViolation( clientNum, JXAC_VIOLATION_TAMPER, details );
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

void Server::checkModuleSignature( int clientNum, const char* moduleName, const char* checksum ) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS || !moduleName || !checksum ) {
        return;
    }
    
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
        if ( Q_stristr( moduleName, sig.name ) == NULL ) continue;
        
        // Check checksum (if not wildcard)
        if ( sig.checksum[0] == '*' ) {
            // Wildcard - any DLL with this name is banned
        } else if ( Q_stricmp( checksum, sig.checksum ) != 0 ) {
            continue; // Checksum mismatch
        }
        
        // Found match - take action
        char details[512];
        Com_sprintf( details, sizeof( details ), 
                   "Cheat module detected: %s (SHA1: %s)", moduleName, checksum );
        
        if ( Q_stricmp( sig.action, "ban" ) == 0 ) {
            reportViolation( clientNum, JXAC_VIOLATION_CHECKSUM, details );
            banPlayer( clientNum, "Cheat module detected" );
        } else if ( Q_stricmp( sig.action, "kick" ) == 0 ) {
            reportViolation( clientNum, JXAC_VIOLATION_CHECKSUM, details );
            kickPlayer( clientNum, "Cheat module detected" );
        } else if ( Q_stricmp( sig.action, "none" ) != 0 ) {
            reportViolation( clientNum, JXAC_VIOLATION_CHECKSUM, details );
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
