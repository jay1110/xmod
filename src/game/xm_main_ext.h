#ifndef GAME_XM_MAIN_EXT_H
#define GAME_XM_MAIN_EXT_H

///////////////////////////////////////////////////////////////////////////////
// XMOD Main Extensions
// Handles commands that need to be processed before client state checks
// Similar to ETJump's OnClientCommand pattern
///////////////////////////////////////////////////////////////////////////////

#include <bgame/q_shared.h>

// Forward declaration
struct gentity_s;
typedef struct gentity_s gentity_t;

namespace xmod {

///////////////////////////////////////////////////////////////////////////////

// Called from ClientCommand BEFORE the ent->client check
// This allows handling of commands like 'authenticate' that arrive
// before the client is fully connected.
// Returns qtrue if command was handled, qfalse to continue normal processing
qboolean OnClientCommand(int clientNum, const char* cmd);

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod

#endif // GAME_XM_MAIN_EXT_H
