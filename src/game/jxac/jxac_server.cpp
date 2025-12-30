#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <game/jxac/jxac_server.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

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

// Time tracking for CVAR checks (check every 60 seconds)
#define JXAC_CVAR_CHECK_INTERVAL 60000
static int lastCvarCheckTime = 0;

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
    
    Com_Printf( "JXAC: Sending screenshot request to client %d (cmd: %s, quality: %d)\n", 
                clientNum, obfuscatedCmd, quality );
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
    
    // Load CVAR config file (if exists)
    loadCvarConfig( cvar::objects::g_jxacCvarFile.svalue );
    
    initialized = qtrue;
    
    Com_Printf( "JXAC: Server initialized successfully\n" );
}

///////////////////////////////////////////////////////////////////////////////

void Server::shutdown() {
    if ( !initialized ) {
        return;
    }
    
    Com_Printf( "JXAC: Shutting down JXAC Server\n" );
    
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
    
    // Speedhack and aimbot detection (every 100ms)
    static int lastAntiCheatCheck = 0;
    if ( level.time - lastAntiCheatCheck > 100 ) {
        lastAntiCheatCheck = level.time;
        
        for ( int i = 0; i < level.maxclients; i++ ) {
            gentity_t* ent = &g_entities[i];
            if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
                continue;
            }
            
            // Skip bots
            if ( ent->r.svFlags & SVF_BOT ) {
                continue;
            }
            
            checkSpeedhack( i );
            checkAimbot( i );
        }
    }
    
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
        Com_Printf( "JXAC: Received unexpected screenshot data from client %d\n", clientNum );
        return;
    }
    
    // First chunk - allocate buffer
    if ( pd->ssBuffer == NULL ) {
        pd->ssBuffer = (unsigned char*)malloc( JXAC_SS_MAX_SIZE );
        if ( !pd->ssBuffer ) {
            Com_Printf( "JXAC: Failed to allocate screenshot buffer for client %d\n", clientNum );
            pd->screenshotPending = qfalse;
            return;
        }
    }
    
    // Check bounds
    if ( pd->ssDataReceived + size > JXAC_SS_MAX_SIZE ) {
        Com_Printf( "JXAC: Screenshot data exceeds maximum size for client %d\n", clientNum );
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
        Com_Printf( "JXAC: Received screenshot complete without pending request for client %d\n", clientNum );
        return;
    }
    
    Com_Printf( "JXAC: Screenshot complete for client %d (%d bytes)\n", clientNum, pd->ssDataReceived );
    
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

void Server::handleCvarResponse( int clientNum, const char* cvarName, const char* value ) {
    if ( !initialized || !cvar::objects::g_jxacEnable.ivalue ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS || !cvarName || !value ) {
        return;
    }
    
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
        Com_Printf( "JXAC: Failed to open screenshot file: %s\n", filename );
        return;
    }
    
    trap_FS_Write( data, size, f );
    trap_FS_FCloseFile( f );
    
    Com_Printf( "JXAC: Screenshot saved: %s (%d bytes)\n", filename, size );
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
            if ( level.time - pd->screenshotRequestTime > 30000 ) {
                Com_Printf( "JXAC: Screenshot timeout for client %d\n", i );
                
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
    
    // TODO: Reload cheat signature database
    // loadCheatDatabase( cvar::objects::g_jxacCheatFile.svalue );
    
    Com_Printf( "JXAC: Configuration reload complete\n" );
}

///////////////////////////////////////////////////////////////////////////////

void Server::checkSpeedhack( int clientNum ) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    // Get player entity and previous position
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client ) {
        return;
    }
    
    gclient_t* client = ent->client;
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    // Initialize on first check
    if ( pd->lastCheckTime == 0 ) {
        VectorCopy( client->ps.origin, pd->lastOrigin );
        pd->lastCheckTime = level.time;
        return;
    }
    
    // Calculate position delta
    vec3_t delta;
    VectorSubtract( client->ps.origin, pd->lastOrigin, delta );
    float distance = VectorLength( delta );
    
    // Calculate time delta (ms)
    int timeDelta = level.time - pd->lastCheckTime;
    if ( timeDelta <= 0 ) {
        return;
    }
    
    // Calculate speed (units per second)
    float speed = (distance / (float)timeDelta) * 1000.0f;
    
    // Get maximum allowed speed (base + sprint + modifiers)
    // Default player speed is around 320, sprint multiplier is ~1.3x
    float maxSpeed = client->ps.speed * 1.5f; // 1.5x for sprint and tolerance
    
    // Check for speedhack (allow 10% tolerance for network jitter)
    if ( speed > maxSpeed * 1.1f ) {
        char details[256];
        Com_sprintf( details, sizeof(details), 
                    "Speedhack detected: %.1f units/s (max: %.1f)", 
                    speed, maxSpeed );
        reportViolation( clientNum, JXAC_VIOLATION_SPEEDHACK, details );
    }
    
    // Update tracking data
    VectorCopy( client->ps.origin, pd->lastOrigin );
    pd->lastCheckTime = level.time;
}

///////////////////////////////////////////////////////////////////////////////

void Server::checkAimbot( int clientNum ) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client ) {
        return;
    }
    
    gclient_t* client = ent->client;
    jxacPlayerData_t* pd = &playerData[clientNum];
    
    // Initialize on first check
    if ( pd->lastCheckTime == 0 ) {
        VectorCopy( client->ps.viewangles, pd->lastViewAngles );
        pd->lastScore = client->ps.persistant[PERS_SCORE];
        return;
    }
    
    // Calculate angle delta
    vec3_t angleDelta;
    for ( int i = 0; i < 3; i++ ) {
        angleDelta[i] = AngleSubtract( client->ps.viewangles[i], 
                                       pd->lastViewAngles[i] );
    }
    
    float angleChange = VectorLength( angleDelta );
    
    // Detect impossible snap (>170° in single frame = ~17ms at 60fps, ~100ms check interval)
    if ( angleChange > 170.0f ) {
        pd->aimbotSnapCount++;
        
        if ( pd->aimbotSnapCount > 3 ) {
            char details[256];
            Com_sprintf( details, sizeof(details), 
                        "Aimbot snap detected: %.1f degree change", 
                        angleChange );
            reportViolation( clientNum, JXAC_VIOLATION_AIMBOT, details );
            pd->aimbotSnapCount = 0; // Reset after reporting
        }
    } else if ( angleChange < 10.0f ) {
        // Decay snap count if no suspicious behavior
        if ( pd->aimbotSnapCount > 0 ) {
            pd->aimbotSnapCount--;
        }
    }
    
    // Track headshot ratio (check on kills)
    if ( client->ps.persistant[PERS_SCORE] != pd->lastScore ) {
        pd->totalKills++;
        // Headshot detection would need hit zone tracking
        // This is a placeholder for future enhancement
    }
    
    // Update tracking
    VectorCopy( client->ps.viewangles, pd->lastViewAngles );
    pd->lastScore = client->ps.persistant[PERS_SCORE];
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
