#ifndef CGAME_XM_CLIENT_AUTH_H
#define CGAME_XM_CLIENT_AUTH_H

#include <string>

///////////////////////////////////////////////////////////////////////////////
// Client-side authentication module
///////////////////////////////////////////////////////////////////////////////

namespace xm_client_auth {

// Initialize authentication system
void init();

// Shutdown authentication system
void shutdown();

// Handle server command for GUID request
void handleGuidRequest();

// Login function - sends authentication to server
void login();

// Get or generate GUID
std::string getGuid();

// Collect hardware ID
std::string getHwid();

} // namespace xm_client_auth

#endif // CGAME_XM_CLIENT_AUTH_H
