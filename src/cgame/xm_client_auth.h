#ifndef CGAME_XM_CLIENT_AUTH_H
#define CGAME_XM_CLIENT_AUTH_H

#include <string>

// Undefine min/max macros that conflict with C++ Standard Library
// These are defined in q_shared.h but conflict with std::min/std::max used internally by STL
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

///////////////////////////////////////////////////////////////////////////////
// Client-side authentication module
///////////////////////////////////////////////////////////////////////////////

namespace xm_client_auth {

// Initialize authentication system
void init();

// Process per-frame authentication tasks
void frame();

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
