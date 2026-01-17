#ifndef GAME_XMOD_GLOBALS_H
#define GAME_XMOD_GLOBALS_H

#include "xmod_database.h"
#include "xmod_session.h"
#include <bgame/q_shared.h>

// Forward declaration
class Privilege;

///////////////////////////////////////////////////////////////////////////////
// Global xmod instances and initialization
///////////////////////////////////////////////////////////////////////////////

namespace xmod {

// Global database instance
extern Database* g_database;

// Session instances (one per client slot)
extern Session* g_sessions[MAX_CLIENTS];

// Initialize xmod systems
void initXmod();

// Shutdown xmod systems
void shutdownXmod();

// Update session for client
void updateClientSession(int clientNum);

///////////////////////////////////////////////////////////////////////////////
// Global helper functions for accessing session data
// These provide safe access with fallback to connectedUsers for backward compatibility
///////////////////////////////////////////////////////////////////////////////

// Check if player is muted (session-first, fallback to User)
bool isClientMuted(int clientNum);

// Set mute status on both session and User
void setClientMuted(int clientNum, bool muted);

// Get player GUID
const std::string& getClientGuid(int clientNum);

// Get player name
const std::string& getClientName(int clientNum);

// Get player formatted name
const std::string& getClientNamex(int clientNum);

// Get player auth level
int getClientLevel(int clientNum);

// Set fake GUID status on both session and User
void setClientFakeGuid(int clientNum, bool fakeguid);

// Check if client has a fake GUID
bool isClientFakeGuid(int clientNum);

// Get mute expiry time
time_t getClientMuteExpiry(int clientNum);

// Check if client has a specific privilege
bool hasClientPrivilege(int clientNum, const Privilege& privilege);

// Set mute metadata (time, reason, authority) on both session and User
void setClientMuteData(int clientNum, time_t muteTime, const std::string& reason, 
                       const std::string& authority, const std::string& authorityx, time_t expiry);

// Clear mute metadata on both session and User
void clearClientMuteData(int clientNum);

// Get client IP address
const std::string& getClientIp(int clientNum);

// Set client auth level (on both session and User)
void setClientLevel(int clientNum, int level);

// Get client MAC address
const std::string& getClientMac(int clientNum);

// Get client timestamp
time_t getClientTimestamp(int clientNum);

// Set client timestamp
void setClientTimestamp(int clientNum, time_t ts);

// Get client greeting text
const std::string& getClientGreetingText(int clientNum);

// Set client greeting text
void setClientGreetingText(int clientNum, const std::string& text);

// Get client greeting audio
const std::string& getClientGreetingAudio(int clientNum);

// Set client greeting audio
void setClientGreetingAudio(int clientNum, const std::string& audio);

// Get mute authority formatted name
const std::string& getClientMuteAuthorityx(int clientNum);

} // namespace xmod

#endif // GAME_XMOD_GLOBALS_H
