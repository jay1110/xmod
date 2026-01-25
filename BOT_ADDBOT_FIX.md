# Bot AddBot Crash Fix - Verification Guide

## Problem Description

### Issue 1: Server Crash on `bot addbot`
- **Symptom**: Server crashes with SIGSEGV when executing `bot addbot` command
- **Error**: Omni-bot crash in `Utils::ConvertString` during `IGame::CheckServerSettings`
- **Root Cause**: Race condition where `Bot_Interface_Update()` calls `pfnUpdate()` on the same frame as bot creation, before bot data is fully stable

### Issue 2: Bots Stop Moving After Warmup
- **Symptom**: Bots spawned before warmup end stop moving when warmup transitions to playing
- **Status**: Already fixed in GAME_INIT (lines 640-705 in g_main.cpp)

## Solution Implemented

### Core Fix: Deferred Client Connection Notification

Added a deferred notification mechanism for bot client connections, similar to the existing entity creation pattern:

1. **New BotEntity flag**: Added `m_NewClient` flag to queue client connection notifications
2. **Queue function**: Implemented `Bot_Queue_ClientConnected()` to defer notification
3. **Processing loop**: Modified `Bot_Interface_Update()` to process queued connections before `pfnUpdate()`
4. **Updated AddBot**: Changed to use `Bot_Queue_ClientConnected()` instead of immediate notification

### Event Sequence (Before Fix)
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

### Event Sequence (After Fix)
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

## Files Modified

1. **src/omnibot/et/g_etbot_interface.cpp**
   - Added `m_NewClient` flag to BotEntity structure (line 43)
   - Implemented `Bot_Queue_ClientConnected()` function (lines 5909-5917)
   - Updated `Bot_Interface_InitHandles()` to initialize m_NewClient (line 5112)
   - Updated `Bot_Event_EntityDeleted()` to clear m_NewClient (line 5931)
   - Added client connection processing loop in `Bot_Interface_Update()` (lines 5305-5326)
   - Modified `AddBot()` to use `Bot_Queue_ClientConnected()` (line 1722)

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

### 4. Test bot maxbots (Regression Check)
```
Console commands:
  bot load
  bot maxbots 4
```
Expected: Bots join successfully (should work as before)

### 5. Test Warmup Transition
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
- Bots through warmup: Stop moving after warmup ends

### After Fix
- `bot addbot`: No crash, bots join and function normally
- Bots through warmup: Continue moving after warmup ends
- `bot maxbots`: Still works as expected (no regression)

## Technical Notes

- The fix uses the same deferred pattern as entity creation (m_NewEntity flag)
- Client connections are processed BEFORE pfnUpdate() to ensure stable state
- SVF_BOT flag is used to preserve bot status across deferred call
- Direct Bot_Event_ClientConnected calls remain for non-bots and persistent bots (safe contexts)
- Warmup fix in GAME_INIT handles re-registration after map_restart

## Compatibility

- Platform: Linux 64-bit (tested), should work on all platforms
- Module: qagame.mp.x86_64.so
- No changes to client modules (cgame, ui)
- No changes to Omni-bot library itself
