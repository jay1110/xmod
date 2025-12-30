#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <cgame/jxac/jxac_client.h>
#include <cgame/jxac/jxac_screenshot.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

// Static state
static qboolean initialized = qfalse;
static qboolean enabled = qtrue;
static int lastHeartbeat = 0;
static qboolean screenshotPending = qfalse;

///////////////////////////////////////////////////////////////////////////////

void Client::init() {
    if ( initialized ) {
        return;
    }
    
    Com_Printf( "JXAC: Initializing JXAC Client v%s\n", JXAC_VERSION_STRING );
    
    initialized = qtrue;
    enabled = qtrue;
    lastHeartbeat = 0;
    screenshotPending = qfalse;
    
    Com_Printf( "JXAC: Client initialized successfully\n" );
}

///////////////////////////////////////////////////////////////////////////////

void Client::shutdown() {
    if ( !initialized ) {
        return;
    }
    
    Com_Printf( "JXAC: Shutting down JXAC Client\n" );
    
    initialized = qfalse;
    enabled = qfalse;
}

///////////////////////////////////////////////////////////////////////////////

void Client::frame() {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Send periodic heartbeat
    if ( cg.time - lastHeartbeat > JXAC_HEARTBEAT_INTERVAL ) {
        sendHeartbeat();
        lastHeartbeat = cg.time;
    }
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendHeartbeat() {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Send heartbeat to server
    trap_SendClientCommand( va("jxac_heartbeat %s", JXAC_VERSION_STRING) );
}

///////////////////////////////////////////////////////////////////////////////

void Client::handleScreenshotRequest( int quality ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    if ( screenshotPending ) {
        // Silent - already pending, ignore
        return;
    }
    
    screenshotPending = qtrue;
    
    // Silent screenshot capture - no console output
    captureScreenshot( quality );
}

///////////////////////////////////////////////////////////////////////////////

void Client::captureScreenshot( int quality ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Clamp quality
    if ( quality < JXAC_SS_QUALITY_MIN ) quality = JXAC_SS_QUALITY_MIN;
    if ( quality > JXAC_SS_QUALITY_MAX ) quality = JXAC_SS_QUALITY_MAX;
    
    // Silent capture - no console output
    
    // Use the Screenshot module to capture and compress
    int jpegSize = 0;
    unsigned char* jpegData = Screenshot::captureAndCompress( &jpegSize, quality );
    
    if ( !jpegData || jpegSize <= 0 ) {
        // Silent failure
        screenshotPending = qfalse;
        return;
    }
    
    // Silent transmission - no console output
    
    // Send screenshot data to server in chunks
    sendScreenshotData( jpegData, jpegSize );
    sendScreenshotComplete();
    
    // Clean up
    free( jpegData );
    screenshotPending = qfalse;
    
    // Silent completion - no console output
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendScreenshotData( const void* data, int size ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Send screenshot data to server in chunks using hex encoding
    // ET engine command system is text-based, so we convert binary to hex
    // MAX_STRING_CHARS is 1024, so we need small chunks: 450 bytes binary = 900 hex chars
    // Command format: "jxac_ss_data <num> <size> <hex>" leaves room for overhead
    const unsigned char* bytes = (const unsigned char*)data;
    const int CHUNK_SIZE = 450;  // 450 bytes binary = 900 hex chars (fits in 1024 limit)
    
    int chunkNum = 0;
    int offset = 0;
    
    while ( offset < size ) {
        int bytesToSend = (size - offset > CHUNK_SIZE) ? CHUNK_SIZE : (size - offset);
        
        // Convert chunk to hex string (2 hex chars per byte)
        char hexBuffer[CHUNK_SIZE * 2 + 1];
        for ( int i = 0; i < bytesToSend; i++ ) {
            // Use snprintf for safety - each byte produces 2 hex chars
            snprintf( &hexBuffer[i * 2], 3, "%02x", bytes[offset + i] );
        }
        hexBuffer[bytesToSend * 2] = '\0';
        
        // Send chunk to server: jxac_ss_data <chunkNum> <size> <hexData>
        trap_SendClientCommand( va("jxac_ss_data %d %d %s", chunkNum, bytesToSend, hexBuffer) );
        
        offset += bytesToSend;
        chunkNum++;
    }
    
    // Silent - no console output
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendScreenshotComplete() {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Send screenshot complete message to server (silent)
    trap_SendClientCommand( "jxac_ss_complete" );
    // Silent - no console output
}

///////////////////////////////////////////////////////////////////////////////

void Client::handleCvarRequest( const char* cvarName ) {
    if ( !initialized || !enabled || !cvarName ) {
        return;
    }
    
    // Get CVAR value
    char value[256];
    trap_Cvar_VariableStringBuffer( cvarName, value, sizeof( value ) );
    
    // Send response to server
    sendCvarResponse( cvarName, value );
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendCvarResponse( const char* cvarName, const char* value ) {
    if ( !initialized || !enabled || !cvarName || !value ) {
        return;
    }
    
    // Send CVAR value to server (silent)
    trap_SendClientCommand( va("jxac_cvar_resp %s %s", cvarName, value) );
    // Silent - no console output
}

///////////////////////////////////////////////////////////////////////////////

qboolean Client::isEnabled() {
    return (initialized && enabled) ? qtrue : qfalse;
}

///////////////////////////////////////////////////////////////////////////////

const char* Client::getVersion() {
    return JXAC_VERSION_STRING;
}

///////////////////////////////////////////////////////////////////////////////

void Client::compressScreenshot( const unsigned char* rawData, int width, int height, 
                                  int quality, unsigned char** outData, int* outSize ) {
    // Placeholder for JPEG compression
    // In a real implementation, this would use libjpeg or similar library
    // to compress the raw framebuffer data to JPEG format
    
    // For now, just return NULL to indicate not implemented
    *outData = NULL;
    *outSize = 0;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
