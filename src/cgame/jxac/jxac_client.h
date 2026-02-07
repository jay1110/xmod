#ifndef CGAME_JXAC_CLIENT_H
#define CGAME_JXAC_CLIENT_H

///////////////////////////////////////////////////////////////////////////////
// JXAC Client-Side Module
// Handles screenshot capture, heartbeat, and anti-tamper
///////////////////////////////////////////////////////////////////////////////

#include <bgame/jxac_common.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

class Client {
public:
    // Initialize JXAC client
    static void init();
    
    // Shutdown JXAC client
    static void shutdown();
    
    // Frame update (called every client frame)
    static void frame();
    
    // Heartbeat
    static void sendHeartbeat();
    
    // Screenshot handling
    static void handleScreenshotRequest( int quality );
    static void handleScreenshotEngineRequest( int quality );
    static void captureScreenshot( int quality );
    static void captureScreenshotEngine( int quality );
    static void sendScreenshotData( const void* data, int size );
    static void sendScreenshotComplete();
    
    // CVAR handling
    static void handleCvarRequest( const char* cvarName );
    static void sendCvarResponse( const char* cvarName, const char* value );
    
    // Binary message handling (uses trap_SendMessage channel)
    static void handleBinaryMessage( const char* buf, int buflen );
    static void sendBinaryMessage( jxacMessageType_t type, const void* data, int dataLen );
    
    // Status
    static qboolean isEnabled();
    static const char* getVersion();
    
private:
    static void compressScreenshot( const unsigned char* rawData, int width, int height, 
                                     int quality, unsigned char** outData, int* outSize );
};

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac

#endif // CGAME_JXAC_CLIENT_H
