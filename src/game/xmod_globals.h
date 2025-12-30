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

} // namespace xmod

#endif // GAME_XMOD_GLOBALS_H
