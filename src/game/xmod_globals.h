#ifndef GAME_XMOD_GLOBALS_H
#define GAME_XMOD_GLOBALS_H

#include "xmod_database.h"
#include "xmod_session.h"
#include <bgame/q_shared.h>

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

} // namespace xmod

#endif // GAME_XMOD_GLOBALS_H
