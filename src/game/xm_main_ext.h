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

// Called from ClientCommand BEFORE the CS_ACTIVE check
// Returns qtrue if command was handled, qfalse to continue normal processing
qboolean OnClientCommand(gentity_t *ent);

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod

#endif // GAME_XM_MAIN_EXT_H
