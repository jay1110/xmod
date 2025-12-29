#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <game/jxac/jxac_server.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

// Static storage for player data
static jxacPlayerData_t playerData[MAX_CLIENTS];
static qboolean initialized = qfalse;

// Configuration (will be controlled by CVARs)
static int g_jxacEnable = 1;
static int g_jxacScreenshotQuality = JXAC_SS_QUALITY_DEFAULT;
static char g_jxacScreenshotPath[MAX_QPATH] = "jxac/screenshots/";
static int g_jxacCheckCvars = 1;
static int g_jxacCheckWallhack = 1;
static int g_jxacCheckSpeedhack = 1;
static int g_jxacAutoBan = 0;
static int g_jxacAutoKick = 1;
static char g_jxacLogFile[MAX_QPATH] = "jxac.log";

///////////////////////////////////////////////////////////////////////////////

void Server::init() {
    if ( initialized ) {
        return;
    }
    
    Com_Printf( "JXAC: Initializing JXAC Server v%s\n", JXAC_VERSION_STRING );
    
    // Clear player data
    memset( playerData, 0, sizeof( playerData ) );
    
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
    if ( !initialized || !g_jxacEnable ) {
        return;
    }
    
    // Check for heartbeat timeouts
    checkHeartbeats();
    
    // Check for pending screenshot timeouts
    checkTimeouts();
}

///////////////////////////////////////////////////////////////////////////////

void Server::clientConnect( int clientNum ) {
    if ( !initialized || !g_jxacEnable ) {
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
    if ( !initialized || !g_jxacEnable ) {
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
    if ( !initialized || !g_jxacEnable ) {
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
    if ( !initialized || !g_jxacEnable ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    gentity_t* ent = &g_entities[clientNum];
    if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
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
    
    pd->screenshotPending = qtrue;
    pd->screenshotRequestTime = level.time;
    pd->ssDataReceived = 0;
    pd->ssDataExpected = 0;
    
    Com_Printf( "JXAC: Requesting screenshot from client %d (quality: %d)\n", clientNum, quality );
    
    // Send screenshot request to client (placeholder)
    // In a real implementation, this would send a network message:
    // trap_SendServerCommand( clientNum, va("jxac ss_req %d", quality) );
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestScreenshotAll( int quality ) {
    if ( !initialized || !g_jxacEnable ) {
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
    if ( !initialized || !g_jxacEnable ) {
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
    if ( !initialized || !g_jxacEnable ) {
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
    if ( !initialized || !g_jxacEnable ) {
        return;
    }
    
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }
    
    jxacPlayerData_t* pd = &playerData[clientNum];
    pd->lastHeartbeat = level.time;
    pd->status |= JXAC_STATUS_HEARTBEAT;
}

///////////////////////////////////////////////////////////////////////////////

void Server::requestCvarCheck( int clientNum ) {
    if ( !initialized || !g_jxacEnable || !g_jxacCheckCvars ) {
        return;
    }
    
    // Placeholder for CVAR checking
    // Would send request to client to report specific CVARs
}

///////////////////////////////////////////////////////////////////////////////

void Server::handleCvarResponse( int clientNum, const char* cvarName, const char* value ) {
    if ( !initialized || !g_jxacEnable ) {
        return;
    }
    
    // Placeholder for CVAR validation
    // Would check if the reported value matches expected values
}

///////////////////////////////////////////////////////////////////////////////

void Server::reportViolation( int clientNum, jxacViolationType_t type, const char* details ) {
    if ( !initialized || !g_jxacEnable ) {
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
    if ( g_jxacAutoBan ) {
        banPlayer( clientNum, va( "JXAC Violation: %s", details ? details : "Cheating detected" ) );
    } else if ( g_jxacAutoKick ) {
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
    if ( !initialized || !g_jxacEnable ) {
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
                JXAC_VERSION_STRING, g_jxacEnable ? "YES" : "NO" );
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
                 g_jxacScreenshotPath, cleanname, timestamp );
    
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
    if ( !violation || g_jxacLogFile[0] == '\0' ) {
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
    trap_FS_FOpenFile( g_jxacLogFile, &f, FS_APPEND );
    
    if ( !f ) {
        Com_Printf( "JXAC: Failed to open log file: %s\n", g_jxacLogFile );
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
    for ( int i = 0; i < level.maxclients; i++ ) {
        gentity_t* ent = &g_entities[i];
        if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
            continue;
        }
        
        jxacPlayerData_t* pd = &playerData[i];
        
        // Check heartbeat timeout
        if ( level.time - pd->lastHeartbeat > JXAC_HEARTBEAT_TIMEOUT ) {
            reportViolation( i, JXAC_VIOLATION_NO_RESPONSE, "Heartbeat timeout" );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Server::checkTimeouts() {
    for ( int i = 0; i < level.maxclients; i++ ) {
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

} // namespace jxac
