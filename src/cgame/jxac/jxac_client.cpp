#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <cgame/jxac/jxac_client.h>
#include <cgame/jxac/jxac_screenshot.h>
#include <cgame/jxac/jxac_modules.h>
#include <cgame/jxac/jxac_antitamper.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

// Screenshot chunk queue structure
struct ScreenshotChunk {
    int chunkNum;
    int bytesToSend;
    char hexData[901];  // 450 bytes * 2 + null terminator
};

// Screenshot file state tracking
static char screenshotFilename[256] = {0};
static int screenshotRequestTime = 0;
static int screenshotQuality = 85;
static int screenshotCounter = 0;  // Counter for unique filenames

#define MAX_CHUNK_QUEUE 300  // Max chunks in queue (for ~135KB screenshot)
static ScreenshotChunk chunkQueue[MAX_CHUNK_QUEUE];
static int chunkQueueHead = 0;  // Next chunk to send
static int chunkQueueTail = 0;  // Next free slot
static int chunkQueueCount = 0;
static qboolean screenshotTransferActive = qfalse;

// Static state
static qboolean initialized = qfalse;
static qboolean enabled = qtrue;
static int lastHeartbeat = 0;
static qboolean screenshotPending = qfalse;
static int lastModuleScan = 0;

// Module scanning interval (180 seconds)
#define JXAC_MODULE_SCAN_INTERVAL 180000
#define JXAC_SCREENSHOT_TIMEOUT 5000  // 5 seconds to wait for screenshot file

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
    chunkQueueHead = 0;
    chunkQueueTail = 0;
    chunkQueueCount = 0;
    screenshotTransferActive = qfalse;
    lastModuleScan = 0;
    screenshotFilename[0] = '\0';
    screenshotRequestTime = 0;
    screenshotQuality = 85;
    
    // Initialize anti-tamper system
    AntiTamper::init();
    
    // Perform initial module scan on connect
    scanAndSendModules();
    lastModuleScan = cg.time;
    
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
    
    // Periodic module scan (every 180 seconds)
    if ( cg.time - lastModuleScan > JXAC_MODULE_SCAN_INTERVAL ) {
        scanAndSendModules();
        lastModuleScan = cg.time;
    }
    
    // Anti-tamper checks (handles its own timing)
    AntiTamper::check();
    
    // Check for pending screenshot file
    if ( screenshotPending && screenshotFilename[0] != '\0' ) {
        // Try to read the screenshot file
        fileHandle_t f;
        int len = trap_FS_FOpenFile( screenshotFilename, &f, FS_READ );
        
        if ( len > 0 ) {
            // File exists and has content - validate size
            // Limit screenshot size to prevent memory issues
            if ( len > JXAC_SS_MAX_SIZE ) {
                // Screenshot too large - abort
                trap_FS_FCloseFile( f );
                trap_FS_Delete( screenshotFilename );
                screenshotPending = qfalse;
                screenshotFilename[0] = '\0';
                return;
            }
            
            // Allocate buffer for file data
            unsigned char* fileData = (unsigned char*)malloc( len );
            if ( fileData ) {
                trap_FS_Read( fileData, len, f );
                trap_FS_FCloseFile( f );
                
                // Send the screenshot data
                sendScreenshotData( fileData, len );
                free( fileData );
                
                // Delete the screenshot file
                trap_FS_Delete( screenshotFilename );
                
                // Clear pending state
                screenshotPending = qfalse;
                screenshotFilename[0] = '\0';
            } else {
                // Memory allocation failed
                trap_FS_FCloseFile( f );
                trap_FS_Delete( screenshotFilename );
                screenshotPending = qfalse;
                screenshotFilename[0] = '\0';
            }
        } else {
            // File doesn't exist or is empty - check for timeout
            if ( cg.time - screenshotRequestTime > JXAC_SCREENSHOT_TIMEOUT ) {
                // Timeout - give up
                screenshotPending = qfalse;
                screenshotFilename[0] = '\0';
            }
            // Note: Don't close file handle when len <= 0 (file not opened)
        }
    }
    
    // Process screenshot chunk queue (send 1-2 chunks per frame to avoid overflow)
    if ( screenshotTransferActive && chunkQueueCount > 0 ) {
        // Send up to 2 chunks per frame to balance transfer speed and stability
        int chunksToSend = (chunkQueueCount > 2) ? 2 : chunkQueueCount;
        
        for ( int i = 0; i < chunksToSend; i++ ) {
            ScreenshotChunk* chunk = &chunkQueue[chunkQueueHead];
            
            // Send chunk to server
            trap_SendClientCommand( va("jxac_ss_data %d %d %s", 
                chunk->chunkNum, chunk->bytesToSend, chunk->hexData) );
            
            // Move to next chunk
            chunkQueueHead = (chunkQueueHead + 1) % MAX_CHUNK_QUEUE;
            chunkQueueCount--;
        }
        
        // Check if transfer is complete
        if ( chunkQueueCount == 0 ) {
            screenshotTransferActive = qfalse;
            sendScreenshotComplete();
        }
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
    
    // Validate and clamp quality
    if ( quality < JXAC_SS_QUALITY_MIN || quality > JXAC_SS_QUALITY_MAX ) {
        quality = JXAC_SS_QUALITY_DEFAULT;
    }
    
    // Store quality for potential retry
    screenshotQuality = quality;
    
    // Generate unique filename using timestamp and counter
    // Format: screenshots/jxac_TIMESTAMP_COUNTER.jpg
    Com_sprintf( screenshotFilename, sizeof(screenshotFilename), 
                 "screenshots/jxac_%d_%d.jpg", cg.time, screenshotCounter++ );
    
    // Validate filename length to prevent buffer overflow in command string
    // Command format: "screenshotJPEG filename\n" needs to fit in buffer
    if ( strlen(screenshotFilename) > JXAC_SS_MAX_FILENAME ) {
        // Filename too long - abort screenshot request
        screenshotPending = qfalse;
        screenshotFilename[0] = '\0';
        return;
    }
    
    // Record request time for timeout checking
    screenshotRequestTime = cg.time;
    
    // Trigger screenshot using engine's native command
    // This works with stock ET engine without any modifications
    // Note: Quality is controlled by engine cvars, not command parameters
    // Using separate buffer for safety to avoid potential va() buffer issues
    char cmd[512];
    Com_sprintf( cmd, sizeof(cmd), "screenshotJPEG %s\n", screenshotFilename );
    trap_SendConsoleCommand( cmd );
    
    // The frame() function will poll for the file and send it when ready
    // screenshotPending flag is already set by handleScreenshotRequest()
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendScreenshotData( const void* data, int size ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Validate parameters
    if ( !data || size <= 0 ) {
        return;
    }
    
    // Queue screenshot data in chunks for frame-based sending
    // This prevents command buffer overflow by spreading chunks across frames
    // MAX_STRING_CHARS is 1024, so we need small chunks: 450 bytes binary = 900 hex chars
    // Command format: "jxac_ss_data <num> <size> <hex>" leaves room for overhead
    const unsigned char* bytes = (const unsigned char*)data;
    const int CHUNK_SIZE = 450;  // 450 bytes binary = 900 hex chars (fits in 1024 limit)
    
    // Hex lookup table for faster conversion
    static const char hexChars[] = "0123456789abcdef";
    
    // Clear queue before starting new transfer
    chunkQueueHead = 0;
    chunkQueueTail = 0;
    chunkQueueCount = 0;
    
    int chunkNum = 0;
    int offset = 0;
    
    while ( offset < size ) {
        int bytesToSend = (size - offset > CHUNK_SIZE) ? CHUNK_SIZE : (size - offset);
        
        // Check queue capacity
        if ( chunkQueueCount >= MAX_CHUNK_QUEUE ) {
            // Queue overflow - screenshot too large, abort transfer
            chunkQueueHead = 0;
            chunkQueueTail = 0;
            chunkQueueCount = 0;
            screenshotTransferActive = qfalse;
            return;
        }
        
        // Get next free slot in queue
        ScreenshotChunk* chunk = &chunkQueue[chunkQueueTail];
        chunk->chunkNum = chunkNum;
        chunk->bytesToSend = bytesToSend;
        
        // Convert chunk to hex string (2 hex chars per byte) using lookup table
        for ( int i = 0; i < bytesToSend; i++ ) {
            unsigned char byte = bytes[offset + i];
            chunk->hexData[i * 2] = hexChars[(byte >> 4) & 0xF];
            chunk->hexData[i * 2 + 1] = hexChars[byte & 0xF];
        }
        chunk->hexData[bytesToSend * 2] = '\0';
        
        // Add to queue
        chunkQueueTail = (chunkQueueTail + 1) % MAX_CHUNK_QUEUE;
        chunkQueueCount++;
        
        offset += bytesToSend;
        chunkNum++;
    }
    
    // Mark transfer as active
    screenshotTransferActive = qtrue;
    
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
