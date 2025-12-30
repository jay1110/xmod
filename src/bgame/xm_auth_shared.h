#ifndef BGAME_XM_AUTH_SHARED_H
#define BGAME_XM_AUTH_SHARED_H

///////////////////////////////////////////////////////////////////////////////
// Authentication constants shared between client (cgame) and server (game)
///////////////////////////////////////////////////////////////////////////////

namespace xm_auth {
    // Server -> Client commands
    const char* const CMD_GUID_REQUEST = "guid_request";
    
    // Client -> Server commands
    const char* const CMD_AUTHENTICATE = "authenticate";
    
    // Constants
    const int GUID_LENGTH = 40;  // SHA1 hash length in hex
    const int HWID_LENGTH = 40;  // SHA1 hash length in hex
}

#endif // BGAME_XM_AUTH_SHARED_H
