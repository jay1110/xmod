#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <cgame/jxac/jxac_client.h>
#include <cgame/jxac/jxac_screenshot.h>
#include <cgame/jxac/jxac_modules.h>
#include <cgame/jxac/jxac_antitamper.h>
#include <cgame/jxac/jxac_tcp_client.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

// Screenshot chunk queue structure
struct ScreenshotChunk {
    int chunkNum;
    int bytesToSend;
    char hexData[901];  // 450 bytes * 2 + null terminator
};

// Screenshot state tracking
static int screenshotRequestTime = 0;
static int screenshotQuality = 85;

// File-based screenshot capture state
static qboolean screenshotFilePending = qfalse;  // Waiting for file to be created
static char screenshotFilename[256];             // Expected filename
static int screenshotFileCheckStart = 0;         // When we started checking for file
#define SCREENSHOT_FILE_CHECK_TIMEOUT 3000       // 3 seconds to wait for file
#define SCREENSHOT_FILE_CHECK_INTERVAL 100       // Check every 100ms

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
static qboolean initialModuleScanDone = qfalse;

// TCP connection state
static qboolean tcpConnected = qfalse;
static int tcpConnectAttempts = 0;
static int lastTcpConnectAttempt = 0;
#define TCP_CONNECT_RETRY_INTERVAL  10000  // 10 seconds between retries
#define TCP_MAX_CONNECT_ATTEMPTS    5

// Screenshot buffer for TCP transfer
static unsigned char* screenshotBuffer = NULL;
static int screenshotBufferSize = 0;

// Module scanning interval (180 seconds)
#define JXAC_MODULE_SCAN_INTERVAL 180000
#define JXAC_SCREENSHOT_TIMEOUT 10000  // 10 seconds timeout for screenshot capture (file-based needs more time)

///////////////////////////////////////////////////////////////////////////////

// Helper: Check if JXAC is enabled on server
static qboolean isServerJxacEnabled() {
    return cvars::bg_jxacEnabled.ivalue ? qtrue : qfalse;
}

// Helper: Get server IP and port for TCP connection
static qboolean getServerInfo(char* ip, int ipSize, int* port) {
    // Get server address from cl_currentServerAddress CVAR
    char serverAddr[256];
    trap_Cvar_VariableStringBuffer("cl_currentServerAddress", serverAddr, sizeof(serverAddr));
    
    if (!serverAddr[0]) {
        return qfalse;
    }
    
    // Parse IP:port format
    char* colonPos = strchr(serverAddr, ':');
    if (colonPos) {
        int ipLen = colonPos - serverAddr;
        if (ipLen >= ipSize) ipLen = ipSize - 1;
        strncpy(ip, serverAddr, ipLen);
        ip[ipLen] = '\0';
        *port = atoi(colonPos + 1);
    } else {
        Q_strncpyz(ip, serverAddr, ipSize);
        *port = 27960;  // Default port
    }
    
    return qtrue;
}

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
    initialModuleScanDone = qfalse;
    screenshotRequestTime = 0;
    screenshotQuality = 85;
    
    // File-based screenshot state
    screenshotFilePending = qfalse;
    screenshotFilename[0] = '\0';
    screenshotFileCheckStart = 0;
    
    // TCP state
    tcpConnected = qfalse;
    tcpConnectAttempts = 0;
    lastTcpConnectAttempt = 0;
    screenshotBuffer = NULL;
    screenshotBufferSize = 0;
    
    // Initialize anti-tamper system
    AntiTamper::init();
    
    // Don't perform initial module scan here - wait for server to confirm JXAC is enabled
    // This prevents unnecessary client commands when server has JXAC disabled
    
    Com_Printf( "JXAC: Client initialized successfully (waiting for server status)\n" );
}

///////////////////////////////////////////////////////////////////////////////

void Client::shutdown() {
    if ( !initialized ) {
        return;
    }
    
    Com_Printf( "JXAC: Shutting down JXAC Client\n" );
    
    // Disconnect TCP
    TcpClient::disconnect();
    tcpConnected = qfalse;
    
    // Free screenshot buffer
    if (screenshotBuffer) {
        free(screenshotBuffer);
        screenshotBuffer = NULL;
        screenshotBufferSize = 0;
    }
    
    initialized = qfalse;
    enabled = qfalse;
}

///////////////////////////////////////////////////////////////////////////////

// Helper: Check if screenshot file exists and process it
static void checkScreenshotFile() {
    if (!screenshotFilePending) {
        return;
    }
    
    // Check for timeout
    if (cg.time - screenshotFileCheckStart > SCREENSHOT_FILE_CHECK_TIMEOUT) {
        CG_Printf("JXAC: Screenshot file timeout\n");
        screenshotFilePending = qfalse;
        screenshotPending = qfalse;
        return;
    }
    
    // Try to open the screenshot file
    fileHandle_t f;
    int len = trap_FS_FOpenFile(screenshotFilename, &f, FS_READ);
    
    if (len <= 0) {
        // File not ready yet
        return;
    }
    
    CG_Printf("JXAC: Found screenshot file: %s (%d bytes)\n", screenshotFilename, len);
    
    // Allocate buffer and read file
    unsigned char* fileData = (unsigned char*)malloc(len);
    if (!fileData) {
        trap_FS_FCloseFile(f);
        screenshotFilePending = qfalse;
        screenshotPending = qfalse;
        return;
    }
    
    trap_FS_Read(fileData, len, f);
    trap_FS_FCloseFile(f);
    
    // Screenshot file captured - clear the pending state
    screenshotFilePending = qfalse;
    
    // Send the screenshot data to server
    Client::sendScreenshotData(fileData, len);
    
    free(fileData);
    
    // Note: The file remains on disk (ET doesn't have trap_FS_Delete)
    // This is unavoidable with the file-based approach
}

///////////////////////////////////////////////////////////////////////////////

void Client::frame() {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Check if server has JXAC enabled - if not, skip all JXAC processing
    // This prevents lag from sending commands when server doesn't need them
    if ( !isServerJxacEnabled() ) {
        // Clear any pending state when server disables JXAC
        screenshotPending = qfalse;
        screenshotFilePending = qfalse;
        screenshotTransferActive = qfalse;
        chunkQueueCount = 0;
        initialModuleScanDone = qfalse;
        // Disconnect TCP if connected
        if (tcpConnected) {
            TcpClient::disconnect();
            tcpConnected = qfalse;
        }
        return;
    }
    
    // Check for pending screenshot file
    checkScreenshotFile();
    
    // Manage TCP connection for screenshot transfer
    if (!TcpClient::isConnected()) {
        // Try to connect if we haven't exhausted retries
        if (tcpConnectAttempts < TCP_MAX_CONNECT_ATTEMPTS) {
            if (cg.time - lastTcpConnectAttempt > TCP_CONNECT_RETRY_INTERVAL || lastTcpConnectAttempt == 0) {
                char serverIP[64];
                int serverPort;
                if (getServerInfo(serverIP, sizeof(serverIP), &serverPort)) {
                    CG_Printf("JXAC TCP: Attempting to connect to %s:%d (attempt %d/%d)\n",
                             serverIP, serverPort, tcpConnectAttempts + 1, TCP_MAX_CONNECT_ATTEMPTS);
                    TcpClient::connect(serverIP, serverPort);
                    tcpConnectAttempts++;
                    lastTcpConnectAttempt = cg.time;
                }
            }
        }
        tcpConnected = qfalse;
    } else {
        if (!tcpConnected) {
            CG_Printf("JXAC TCP: Connected to server\n");
            tcpConnected = qtrue;
            tcpConnectAttempts = 0;  // Reset on successful connection
        }
    }
    
    // Process TCP client (handles connect/transfer state)
    TcpClient::frame();
    
    // Perform initial module scan when JXAC becomes enabled
    if ( !initialModuleScanDone ) {
        scanAndSendModules();
        lastModuleScan = cg.time;
        initialModuleScanDone = qtrue;
    }
    
    // Process module queue (send 2 modules per frame)
    processModuleQueue();

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
    
    // Process screenshot chunk queue (send 1-2 chunks per frame to avoid overflow)
    // This is the UDP fallback - only used if TCP is not connected
    if ( screenshotTransferActive && chunkQueueCount > 0 && !TcpClient::isReady() ) {
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
    
    // Check for screenshot timeout (if capture is pending but no data queued)
    if ( screenshotPending && !screenshotFilePending && !screenshotTransferActive && chunkQueueCount == 0 ) {
        if ( cg.time - screenshotRequestTime > JXAC_SCREENSHOT_TIMEOUT ) {
            // Timeout - clear pending state
            screenshotPending = qfalse;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendHeartbeat() {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Also check if server has JXAC enabled
    if ( !isServerJxacEnabled() ) {
        return;
    }
    
    // Send heartbeat to server (silent - no console output)
    trap_SendClientCommand( va("jxac_heartbeat %s", JXAC_VERSION_STRING) );
}

///////////////////////////////////////////////////////////////////////////////

void Client::handleScreenshotRequest( int quality ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Also check if server has JXAC enabled
    if ( !isServerJxacEnabled() ) {
        return;
    }
    
    if ( screenshotPending || screenshotFilePending ) {
        // Silent - already pending, ignore
        return;
    }
    
    screenshotPending = qtrue;
    screenshotRequestTime = cg.time;
    
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
    
    // Store quality for potential retry
    screenshotQuality = quality;
    
    // Use file-based capture via ET engine's screenshotJPEG command
    // This creates a file that we then read and send to the server
    
    // Generate a unique filename using timestamp
    qtime_t ct;
    trap_RealTime(&ct);
    
    // Use a JXAC-specific prefix to identify our screenshots
    Com_sprintf(screenshotFilename, sizeof(screenshotFilename), 
                "screenshots/jxac_%04d%02d%02d_%02d%02d%02d.jpg",
                1900 + ct.tm_year, ct.tm_mon + 1, ct.tm_mday,
                ct.tm_hour, ct.tm_min, ct.tm_sec);
    
    // Execute the screenshot command (silent - no echo)
    // The screenshotJPEG command takes an optional filename parameter
    trap_SendConsoleCommand(va("screenshotJPEG jxac_%04d%02d%02d_%02d%02d%02d\n",
                               1900 + ct.tm_year, ct.tm_mon + 1, ct.tm_mday,
                               ct.tm_hour, ct.tm_min, ct.tm_sec));
    
    // Mark that we're waiting for the file
    screenshotFilePending = qtrue;
    screenshotFileCheckStart = cg.time;
    
    // Note: The actual file reading and sending happens in checkScreenshotFile()
    // which is called from frame()
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendScreenshotData( const void* data, int size ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Try TCP first - it's more reliable for large data transfers
    if ( TcpClient::isReady() && !TcpClient::isTransferring() ) {
        // Store a copy of the data for TCP transfer
        // (TCP client needs the buffer to remain valid during async transfer)
        // Reuse existing buffer if large enough to reduce allocations
        if (screenshotBuffer && screenshotBufferSize < size) {
            free(screenshotBuffer);
            screenshotBuffer = NULL;
            screenshotBufferSize = 0;
        }
        if (!screenshotBuffer) {
            screenshotBuffer = (unsigned char*)malloc(size);
        }
        if (screenshotBuffer) {
            memcpy(screenshotBuffer, data, size);
            screenshotBufferSize = size;
            
            if (TcpClient::sendScreenshot(screenshotBuffer, size, screenshotQuality)) {
                CG_Printf("JXAC: Sending screenshot via TCP (%d bytes)\n", size);
                screenshotPending = qfalse;  // TCP handles the transfer
                return;
            } else {
                // TCP send failed, fall through to UDP
                // Keep buffer allocated for reuse
            }
        }
    }
    
    // Fallback to UDP chunked transfer
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
    
    CG_Printf("JXAC: Sending screenshot via UDP (%d bytes, %d chunks)\n", size, chunkNum);
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
    // This function is deprecated - use Screenshot::compressRawToJpeg instead
    // Kept for API compatibility
    *outData = NULL;
    *outSize = 0;
    
    if ( !rawData || !outData || !outSize ) {
        return;
    }
    
    int channels = 3;  // Assume RGB
    *outData = Screenshot::compressRawToJpeg( rawData, width, height, channels, quality, outSize );
}

///////////////////////////////////////////////////////////////////////////////

void Client::sendBinaryMessage( jxacMessageType_t type, const void* data, int dataLen ) {
    if ( !initialized || !enabled ) {
        return;
    }
    
    // Check if server has JXAC enabled
    if ( !cvars::bg_jxacEnabled.ivalue ) {
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
    trap_SendMessage( msgBuf, sizeof(jxacBinaryHeader_t) + dataLen );
}

///////////////////////////////////////////////////////////////////////////////

void Client::handleBinaryMessage( const char* buf, int buflen ) {
    if ( !initialized || !enabled || !buf || buflen < (int)sizeof(jxacBinaryHeader_t) ) {
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
        case JXAC_MSG_SS_REQUEST:
            // Extract quality from data (4-byte int) using memcpy for alignment safety
            if ( header->dataLen >= 4 ) {
                int quality;
                memcpy( &quality, data, sizeof(quality) );
                handleScreenshotRequest( quality );
            }
            break;
            
        case JXAC_MSG_CVAR_REQUEST:
            // Extract CVAR name from data (null-terminated string)
            // Verify null terminator exists within bounds
            if ( header->dataLen > 0 ) {
                bool hasNull = false;
                for ( int i = 0; i < header->dataLen; i++ ) {
                    if ( data[i] == '\0' ) {
                        hasNull = true;
                        break;
                    }
                }
                if ( hasNull ) {
                    handleCvarRequest( data );
                }
            }
            break;
            
        case JXAC_MSG_STATUS:
            // Server status check - just acknowledge by updating heartbeat
            lastHeartbeat = cg.time;
            break;
            
        default:
            // Unknown message type - ignore
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
