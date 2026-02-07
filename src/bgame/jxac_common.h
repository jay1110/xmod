#ifndef BGAME_JXAC_COMMON_H
#define BGAME_JXAC_COMMON_H

///////////////////////////////////////////////////////////////////////////////
// JXAC - Jays XMod AntiCheat
// Common definitions shared between client and server
///////////////////////////////////////////////////////////////////////////////

// JXAC Version
#define JXAC_VERSION_MAJOR  1
#define JXAC_VERSION_MINOR  0
#define JXAC_VERSION_PATCH  0
#define JXAC_VERSION_STRING "1.0.0"

// Binary Message Protocol Magic
#define JXAC_BINARY_MAGIC   0x4A584143  // "JXAC" in hex

// Binary Message Header (8 bytes)
#pragma pack(push, 1)
typedef struct jxacBinaryHeader_s {
    unsigned int    magic;      // JXAC_BINARY_MAGIC
    unsigned short  type;       // jxacMessageType_t
    unsigned short  dataLen;    // Length of data following header
} jxacBinaryHeader_t;
#pragma pack(pop)

// Network Protocol Message Types
typedef enum {
    JXAC_MSG_HEARTBEAT      = 0,    // Client -> Server: Regular status update
    JXAC_MSG_SS_REQUEST     = 1,    // Server -> Client: Request screenshot
    JXAC_MSG_SS_DATA        = 2,    // Client -> Server: Screenshot data chunk
    JXAC_MSG_SS_COMPLETE    = 3,    // Client -> Server: Screenshot transfer complete
    JXAC_MSG_VIOLATION      = 4,    // Client -> Server: Self-reported violation
    JXAC_MSG_STATUS         = 5,    // Server -> Client: JXAC status/version check
    JXAC_MSG_CVAR_REQUEST   = 6,    // Server -> Client: Request CVAR values
    JXAC_MSG_CVAR_RESPONSE  = 7,    // Client -> Server: CVAR values response
    JXAC_MSG_MODULE         = 8,    // Client -> Server: Module info (binary channel)
    JXAC_MSG_MODULE_COMPLETE= 9,    // Client -> Server: Module scan complete
    _JXAC_MSG_MAX
} jxacMessageType_t;

// Violation Types
typedef enum {
    JXAC_VIOLATION_NONE             = 0,
    JXAC_VIOLATION_CVAR             = 1,    // Illegal CVAR detected
    JXAC_VIOLATION_WALLHACK         = 2,    // Wallhack detected
    JXAC_VIOLATION_CHECKSUM         = 4,    // File checksum mismatch
    JXAC_VIOLATION_SS_BLOCKED       = 5,    // Screenshot blocked/faked
    JXAC_VIOLATION_TAMPER           = 6,    // JXAC client tampered/disabled
    JXAC_VIOLATION_NO_RESPONSE      = 7,    // No response from client
    _JXAC_VIOLATION_MAX
} jxacViolationType_t;

// Client Status Flags
typedef enum {
    JXAC_STATUS_NONE        = 0x00,
    JXAC_STATUS_CONNECTED   = 0x01, // JXAC client connected
    JXAC_STATUS_VERIFIED    = 0x02, // Client version verified
    JXAC_STATUS_HEARTBEAT   = 0x04, // Recent heartbeat received
    JXAC_STATUS_CLEAN       = 0x08, // No violations detected
    JXAC_STATUS_FLAGGED     = 0x10, // Client flagged for investigation
    JXAC_STATUS_BANNED      = 0x20, // Client banned by JXAC
} jxacStatusFlags_t;

// Screenshot Constants
#define JXAC_SS_CHUNK_SIZE      8192    // Screenshot data chunk size (8KB)
#define JXAC_SS_MAX_SIZE        (1024 * 1024 * 2) // Max screenshot size (2MB)
#define JXAC_SS_QUALITY_MIN     1
#define JXAC_SS_QUALITY_MAX     100
#define JXAC_SS_QUALITY_DEFAULT 85

// Command Data Transmission Constants
#define JXAC_CMD_DATA_CHUNK_SIZE 450    // Max binary data chunk for commands (450 bytes = 900 hex chars, fits in 1024 limit)

// Screenshot Request Obfuscation - use innocent-looking command names
#define JXAC_NUM_OBFUSCATED_CMDS 5
// Declared here, defined in jxac_server.cpp (server only)
extern const char* jxacObfuscatedCmds[JXAC_NUM_OBFUSCATED_CMDS];

// Heartbeat Constants
#define JXAC_HEARTBEAT_INTERVAL 30000   // Heartbeat interval (30 seconds)
#define JXAC_HEARTBEAT_TIMEOUT  60000   // Heartbeat timeout (60 seconds)

// Protected CVARs (examples - expand as needed)
#define JXAC_MAX_PROTECTED_CVARS 64

// JXAC Player Data Structure (Server-side)
typedef struct jxacPlayerData_s {
    int             clientNum;
    int             status;             // jxacStatusFlags_t
    int             lastHeartbeat;      // Last heartbeat time
    int             violations;         // Violation count
    int             lastViolation;      // Last violation type
    int             lastViolationTime;  // Last violation time
    char            guid[33];           // Player GUID
    qboolean        screenshotPending;  // Screenshot request pending
    int             screenshotRequestTime;
    int             ssDataReceived;     // Screenshot bytes received
    int             ssDataExpected;     // Screenshot bytes expected
    unsigned char*  ssBuffer;           // Screenshot data buffer
    qboolean        violationReported[_JXAC_VIOLATION_MAX];  // Track if violation was already reported
    // Random timing for anti-timing attack
    qboolean        scheduledScreenshot;// Scheduled screenshot pending
    int             scheduledScreenshotTime; // Time when screenshot should be requested
    int             scheduledScreenshotQuality; // Quality for scheduled screenshot
} jxacPlayerData_t;

// JXAC Screenshot Request Structure
typedef struct jxacSSRequest_s {
    int             clientNum;
    int             requestTime;
    int             quality;
} jxacSSRequest_t;

// JXAC CVAR Check Structure
typedef struct jxacCvarCheck_s {
    char            name[64];
    char            expectedValue[128];
    qboolean        exactMatch;     // true = exact match, false = contains
} jxacCvarCheck_t;

// JXAC Violation Log Entry
typedef struct jxacViolation_s {
    int             clientNum;
    int             timestamp;
    int             type;           // jxacViolationType_t
    char            guid[33];
    char            playerName[36];
    char            details[256];
} jxacViolation_t;

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_JXAC_COMMON_H
