# Bot AddBot Crash Fix - Verification Guide

## Problem Description

### Issue 1: Server Crash on `bot addbot`
- **Symptom**: Server crashes with SIGSEGV when executing `bot addbot` command
- **Error**: Omni-bot crash in `Utils::ConvertString` during `IGame::CheckServerSettings`
- **Root Cause**: Race condition where `Bot_Interface_Update()` calls `pfnUpdate()` on the same frame as bot creation, before bot data is fully stable

### Issue 2: Bots Stop Moving After Warmup
- **Symptom**: Bots spawned before warmup end stop moving when warmup transitions to playing
- **Status**: Already fixed in GAME_INIT (lines 640-705 in g_main.cpp)

### Issue 3: Server Crash on `bot kickbot`
- **Symptom**: Server crashes with SIGSEGV when executing `bot kickbot` command on moving bots
- **Error**: Same crash pattern - Omni-bot crash in `Utils::ConvertString` during `IGame::CheckServerSettings`
- **Root Cause**: Race condition where `RemoveBot()` disconnects a bot during `pfnConsoleCommand()`, and the immediate disconnection causes Omnibot's internal state to become inconsistent
- **Solution**: Deferred kick mechanism using `m_PendingKick` flag

## Solution Implemented

### Core Fix 1: Deferred Client Connection Notification (for addbot)

Added a deferred notification mechanism for bot client connections, similar to the existing entity creation pattern:

1. **New BotEntity flag**: Added `m_NewClient` flag to queue client connection notifications
2. **Queue function**: Implemented `Bot_Queue_ClientConnected()` to defer notification
3. **Processing loop**: Modified `Bot_Interface_Update()` to process queued connections before `pfnUpdate()`
4. **Updated AddBot**: Changed to use `Bot_Queue_ClientConnected()` instead of immediate notification

### Core Fix 2: Deferred Bot Kick (for kickbot)

Added a deferred kick mechanism to prevent crashes during bot removal:

1. **New BotEntity flag**: Added `m_PendingKick` flag to mark bots for deferred kicking
2. **Updated RemoveBot**: Changed to set `m_PendingKick` flag instead of calling `trap_DropClient()` immediately
3. **Processing loop**: Added processing of pending kicks in `Bot_Interface_Update()` AFTER `pfnUpdate()` completes

### Event Sequence - AddBot (Before Fix)
```
Frame N:
  - bot addbot command executed
  - AddBot() creates bot entity
  - ClientBegin() initializes bot
  - Bot_Event_EntityCreated() called
  - Bot_Event_ClientConnected() called
  - Bot_Interface_Update() called
    - pfnUpdate() triggers CheckServerSettings
    - CRASH: Bot data not fully stable yet
```

### Event Sequence - AddBot (After Fix)
```
Frame N:
  - bot addbot command executed
  - AddBot() creates bot entity
  - ClientBegin() initializes bot
  - Bot_Event_EntityCreated() called
  - Bot_Queue_ClientConnected() queues notification
  - Bot_Interface_Update() called
    - pfnUpdate() runs (bot not yet registered as connected)

Frame N+1:
  - Bot_Interface_Update() called
    - Process queued client connections
    - Bot_Event_ClientConnected() called NOW
    - pfnUpdate() runs safely
```

### Event Sequence - Kickbot (Before Fix)
```
Frame N:
  - bot kickbot command executed
  - Bot_Interface_ConsoleCommand() calls pfnConsoleCommand()
  - Omni-bot calls RemoveBot()
  - RemoveBot() calls trap_DropClient() IMMEDIATELY
  - ClientDisconnect() notifies Omni-bot via Bot_Event_ClientDisConnected()
  - Control returns to pfnConsoleCommand() which continues processing
  - CRASH: Omni-bot accesses invalidated bot data
```

### Event Sequence - Kickbot (After Fix)
```
Frame N:
  - bot kickbot command executed
  - Bot_Interface_ConsoleCommand() calls pfnConsoleCommand()
  - Omni-bot calls RemoveBot()
  - RemoveBot() sets m_PendingKick flag (deferred)
  - pfnConsoleCommand() completes normally
  - Bot_Interface_Update() continues
    - pfnUpdate() runs safely (bot still connected)
    - Process pending kicks AFTER pfnUpdate()
    - trap_DropClient() called NOW
```

## Files Modified

1. **src/omnibot/et/g_etbot_interface.cpp**
   - Added `m_NewClient` flag to BotEntity structure
   - Added `m_PendingKick` flag to BotEntity structure
   - Implemented `Bot_Queue_ClientConnected()` function
   - Updated `Bot_Interface_InitHandles()` to initialize flags
   - Updated `Bot_Event_EntityDeleted()` to clear flags
   - Added client connection processing loop in `Bot_Interface_Update()`
   - Added pending kick processing loop in `Bot_Interface_Update()` (AFTER pfnUpdate)
   - Modified `AddBot()` to use `Bot_Queue_ClientConnected()`
   - Modified `RemoveBot()` to use deferred kick via `m_PendingKick` flag

2. **src/omnibot/et/g_etbot_interface.h**
   - Added `Bot_Queue_ClientConnected()` declaration (line 52)

3. **src/game/g_client.cpp**
   - Updated comment to reflect deferred notification approach (lines 2207-2209)

## Verification Steps

### 1. Build Verification
```bash
cd xmod
make clean
make PLATFORM=linux64 VARIANT=release
```
Expected: Build completes successfully ✓

### 2. Server Startup
```bash
cd build.linux64-release/xmod-2.0.0
./etded +set fs_game xmod-2.0.0 +set dedicated 2 +exec server.cfg
```

### 3. Test bot addbot (Primary Fix)
```
Console commands:
  bot load
  bot addbot
  bot addbot
  bot addbot
```
Expected: No crash, bots join successfully

### 4. Test bot kickbot (New Fix)
```
Console commands:
  bot load
  bot maxbots 4
  (wait for bots to spawn and move around)
  bot kickbot
```
Expected: No crash, bot is kicked successfully

### 5. Test bot maxbots (Regression Check)
```
Console commands:
  bot load
  bot maxbots 4
```
Expected: Bots join successfully (should work as before)

### 6. Test Warmup Transition
```
Setup:
  1. Set g_doWarmup 1
  2. Start match
  3. Add bots during warmup: bot maxbots 4
  4. Wait for warmup to end or use: startmatch

Expected: Bots continue moving after warmup ends
```

## Debug Output

If `g_developer 1` is set, you'll see debug output during GAME_INIT:
```
[BOT_DEBUG] Client X (BotName): sessionTeam=1 pm_flags=0x0 pm_type=0 contents=1000000 health=100
```

If bots have invalid team during warmup transition:
```
[BOT_FIX] Bot BotName had invalid team 3, assigning to AXIS
```

## Expected Behavior

### Before Fix
- `bot addbot`: CRASH (SIGSEGV in Omni-bot)
- `bot kickbot`: CRASH (SIGSEGV in Omni-bot)
- Bots through warmup: Stop moving after warmup ends

### After Fix
- `bot addbot`: No crash, bots join and function normally
- `bot kickbot`: No crash, bots are kicked successfully
- Bots through warmup: Continue moving after warmup ends
- `bot maxbots`: Still works as expected (no regression)

## Technical Notes

- The fix uses the same deferred pattern as entity creation (m_NewEntity flag)
- Client connections are processed BEFORE pfnUpdate() to ensure stable state
- SVF_BOT flag is used to preserve bot status across deferred call
- Direct Bot_Event_ClientConnected calls remain for non-bots and persistent bots (safe contexts)
- Warmup fix in GAME_INIT handles re-registration after map_restart
- **FIXED:** `EF_CONNECTION` check in `ClientEndFrame()` now skips bots entirely

## Compatibility

- Platform: Linux 64-bit (tested), should work on all platforms
- Module: qagame.mp.x86_64.so
- No changes to client modules (cgame, ui)
- No changes to Omni-bot library itself

## Update: Connection Interrupted Fix (Revised)

### Issue
Bots showed "Connection Interrupted" (ping 999) immediately after warmup end or map_restart.

### Root Cause Analysis
The previous fix attempted to initialize `lastUpdateFrame` in `ClientBegin()`, but this was insufficient because:

1. `ClientEndFrame()` runs BEFORE `Bot_Interface_Update()` in the frame order
2. Omnibot may take several frames after registration before it starts sending commands
3. Even with correct initialization, if Omnibot doesn't send commands for 3+ frames, `EF_CONNECTION` gets set

The real issue is that **bots are server-side entities without real network connections**, so the connection timeout detection in `ClientEndFrame()` is fundamentally inappropriate for them.

### Solution (Correct Fix)
Skip the `EF_CONNECTION` flag check for bots in `ClientEndFrame()`:

```cpp
// In ClientEndFrame() - src/game/g_active.cpp
if ( frames > 2 ) {
    frames = 2;
    // Skip EF_CONNECTION for bots - they have no network connection
    if ( !(ent->r.svFlags & SVF_BOT) ) {
        ent->client->ps.eFlags |= EF_CONNECTION;
        ent->s.eFlags |= EF_CONNECTION;
    }
}
```

This is the correct fix because:
- Bots are server-side entities with no network latency
- The "Connection Interrupted" display is meaningless for bots
- Omnibot may legitimately delay sending commands for several frames after bot registration

## Update: Bot Stuck Fix (Further Revised)

### Issue
Despite the `EF_CONNECTION` skip, bots still appeared stuck/frozen after warmup or map_restart.

### Root Cause Analysis
In `ClientBegin()`, persistent bots (restored from session data after map_restart) were re-registered with Omnibot using a different pattern than newly added bots:

**Problem (Old Code):**
```cpp
// In ClientBegin() - for persistent bots
Bot_Event_EntityCreated(ent);           // Direct call
Bot_Event_ClientConnected(clientNum, qtrue);  // Direct call - WRONG!
```

**How AddBot works (New Bots):**
```cpp
// In AddBot() - for new bots
Bot_ClearPendingEntityCreation(bot);    // Clear flag first
Bot_Event_EntityCreated(bot);           // Direct call
Bot_Queue_ClientConnected(num, qtrue);  // QUEUED, processed after pfnUpdate()
```

The key difference: **new bots queue the client connection**, which is processed AFTER `pfnUpdate()` in `Bot_Interface_Update()`. This deferral is critical because:

1. Omnibot's internal state needs to be ready before receiving client connections
2. The entity must be fully registered before the connection event
3. `pfnUpdate()` must complete without the newly connected bot in its client list

### Solution (Correct Fix)
Align persistent bot re-registration with new bot registration:

```cpp
// In ClientBegin() - src/game/g_client.cpp
if (client->sess.botNeedsReregister && (ent->r.svFlags & SVF_BOT)) {
    client->sess.botNeedsReregister = qfalse;
    
    // Clear pending entity creation to prevent double registration
    Bot_ClearPendingEntityCreation(ent);
    
    // Register entity immediately
    Bot_Event_EntityCreated(ent);
    
    // QUEUE client connection (processed after pfnUpdate())
    Bot_Queue_ClientConnected(clientNum, qtrue);
    // ...
}
```

### Changes Made
- Added `Bot_ClearPendingEntityCreation()` helper function
- Changed persistent bot re-registration to use `Bot_Queue_ClientConnected()` instead of direct `Bot_Event_ClientConnected()`
- This ensures both new and persistent bots use the same registration flow
