#ifndef BGAME_XM_AUTH_SHARED_H
#define BGAME_XM_AUTH_SHARED_H

///////////////////////////////////////////////////////////////////////////////
// Authentication constants shared between client (cgame) and server (game)
// Based on ETJump implementation
///////////////////////////////////////////////////////////////////////////////

namespace xm_auth {
    // Server -> Client commands
    const char* const CMD_GUID_REQUEST = "guid_request";
    
    // Client -> Server commands
    // Using "authenticate" like ETJump does - no trap_AddCommand needed
    const char* const CMD_AUTHENTICATE = "authenticate";

    // Constants
    const int GUID_LENGTH = 40;  // SHA1 hash length in hex
    const int HWID_LENGTH = 40;  // SHA1 hash length in hex
    const int AUTH_TIMEOUT_MS = 60000;  // 60 seconds timeout for authentication (increased)
}

#endif // BGAME_XM_AUTH_SHARED_H
