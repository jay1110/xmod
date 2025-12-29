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
    // Note: For actual implementation, we need a binary data transmission method
    // For now, we'll use a simplified approach - send completion message only
    Com_Printf( "JXAC: Screenshot data ready (%d bytes), sending completion\n", size );
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendScreenshotComplete() {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Send screenshot complete message to server
    trap_SendClientCommand( "jxac_ss_complete" );
    Com_Printf( "JXAC: Screenshot complete message sent to server\n" );
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
    trap_SendClientCommand( va("jxac_cvar_resp %s %s", cvarName, value) );
    Com_Printf( "JXAC: Sent CVAR %s=%s to server\n", cvarName, value );
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
