#ifndef GAME_JXAC_SERVER_H
#define GAME_JXAC_SERVER_H

///////////////////////////////////////////////////////////////////////////////
// JXAC Server-Side Module
// Handles player tracking, violation detection, and admin actions
///////////////////////////////////////////////////////////////////////////////

#include <bgame/jxac_common.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

class Server {
public:
    // Initialize JXAC server
    static void init();
    
    // Shutdown JXAC server
    static void shutdown();
    
    // Frame update (called every server frame)
    static void frame();
    
    // Client connection events
    static void clientConnect( int clientNum );
    static void clientDisconnect( int clientNum );
    static void clientBegin( int clientNum );
    
    // Screenshot functions
    static void requestScreenshot( int clientNum, int quality = JXAC_SS_QUALITY_DEFAULT, const char* reason = "Requested by admin" );
    static void requestScreenshotAll( int quality = JXAC_SS_QUALITY_DEFAULT, const char* reason = "Requested by admin" );
    static void handleScreenshotData( int clientNum, const void* data, int size );
    static void handleScreenshotComplete( int clientNum );
    
    // Heartbeat handling
    static void handleHeartbeat( int clientNum );
    
    // CVAR checking
    static void requestCvarCheck( int clientNum );
    static void requestForcedCvarCheck( int clientNum );
    static void queueForcedCvars(int clientNum);
    static int sendPendingForcedCvar(int clientNum);
    static void handleCvarResponse( int clientNum, const char* cvarName, const char* value );
    static void loadCvarConfig( const char* filename );
    static void reloadConfig();
    
    // Violation handling
    static void reportViolation( int clientNum, jxacViolationType_t type, const char* details );
    static void handleViolation( int clientNum, jxacViolationType_t type, const char* details );
    
    // Status and information
    static jxacPlayerData_t* getPlayerData( int clientNum );
    static const char* getStatusString( int clientNum );
    static void printStatus( int clientNum );
    static void printStatusAll();
    
    // Admin actions
    static void kickPlayer( int clientNum, const char* reason );
    static void banPlayer( int clientNum, const char* reason );

    // Config file loading
    static void loadForceCvarConfig( const char* filename );
    static void loadCheatCvarConfig( const char* filename );
    static void loadCheatDatabase( const char* filename );
    static void checkForcedCvar( int clientNum, const char* cvarName, const char* value );
    static void requestCheatCvarScan( int clientNum );
    static void handleCheatCvarResponse( int clientNum, const char* cvarName, const char* value );
    static void checkModuleSignature( int clientNum, const char* moduleName, const char* checksum );

    // Binary message handling (uses trap_SendMessage channel)
    static void handleBinaryMessage( int clientNum, const char* buf, int buflen );
    static void sendBinaryMessage( int clientNum, jxacMessageType_t type, const void* data, int dataLen );

private:
    static void saveScreenshot( int clientNum, const unsigned char* data, int size );
    static void logViolation( const jxacViolation_t* violation );
    static void checkHeartbeats();
    static void checkTimeouts();
};

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac

#endif // GAME_JXAC_SERVER_H
