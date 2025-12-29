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
    // In a real implementation, this would send a network message:
    // trap_SendClientCommand( va("jxac heartbeat %s", JXAC_VERSION_STRING) );
    
    // For now, this is a placeholder
}

///////////////////////////////////////////////////////////////////////////////

void Client::handleScreenshotRequest( int quality ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    if ( screenshotPending ) {
        Com_Printf( "JXAC: Screenshot request ignored - already pending\n" );
        return;
    }
    
    screenshotPending = qtrue;
    
    Com_Printf( "JXAC: Screenshot requested (quality: %d)\n", quality );
    
    // Capture screenshot
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
    
    Com_Printf( "JXAC: Capturing screenshot (quality: %d)...\n", quality );
    
    // Use the Screenshot module to capture and compress
    int jpegSize = 0;
    unsigned char* jpegData = Screenshot::captureAndCompress( &jpegSize, quality );
    
    if ( !jpegData || jpegSize <= 0 ) {
        Com_Printf( "JXAC: Failed to capture/compress screenshot\n" );
        screenshotPending = qfalse;
        return;
    }
    
    Com_Printf( "JXAC: Screenshot compressed to %d bytes\n", jpegSize );
    
    // Send screenshot data to server in chunks
    sendScreenshotData( jpegData, jpegSize );
    sendScreenshotComplete();
    
    // Clean up
    free( jpegData );
    screenshotPending = qfalse;
    
    Com_Printf( "JXAC: Screenshot sent successfully\n" );
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendScreenshotData( const void* data, int size ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Send screenshot data to server in chunks
    const unsigned char* ptr = (const unsigned char*)data;
    int remaining = size;
    
    while ( remaining > 0 ) {
        int chunkSize = ( remaining > JXAC_SS_CHUNK_SIZE ) ? JXAC_SS_CHUNK_SIZE : remaining;
        
        // In a real implementation, send network message:
        // trap_SendClientCommand( va("jxac ss_data %d ...", chunkSize) );
        
        ptr += chunkSize;
        remaining -= chunkSize;
    }
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendScreenshotComplete() {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Send screenshot complete message to server
    // In a real implementation:
    // trap_SendClientCommand( "jxac ss_complete" );
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
    
    // Send CVAR value to server
    // In a real implementation:
    // trap_SendClientCommand( va("jxac cvar %s %s", cvarName, value) );
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
