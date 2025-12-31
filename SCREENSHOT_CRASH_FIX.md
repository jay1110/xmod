# Screenshot Crash Fix

## Issue
When the JXAC anticheat system requests a screenshot from a client, the client crashes immediately.

## Root Cause
The screenshot capture code in `src/cgame/jxac/jxac_screenshot.cpp` calls `trap_R_ReadPixels()`, which invokes the `CG_R_READPIXELS` syscall (enum value 244 in `cg_public.h`). This syscall **does not exist** in the stock Wolfenstein: Enemy Territory engine.

When the client calls `Engine::ptr(CG_R_READPIXELS, ...)`, the engine's syscall dispatcher receives an invalid syscall number and crashes.

## Solution
The `captureFramebuffer()` function in `jxac_screenshot.cpp` has been modified to:
1. Return `NULL` immediately without calling `trap_R_ReadPixels()`
2. Keep the original code commented out for future reference
3. Add comprehensive comments explaining the issue and possible solutions

## Changes
- `src/cgame/jxac/jxac_screenshot.cpp`: Disabled framebuffer capture
- `doc/JXAC.md`: Updated documentation to reflect screenshot limitations
- `doc/JXAC_IMPLEMENTATION.md`: Updated implementation status

## Impact
### Before Fix
- ❌ Server requests screenshot
- ❌ Client calls `trap_R_ReadPixels()`
- ❌ Engine crashes with invalid syscall
- ❌ Player disconnected, cannot play

### After Fix
- ✅ Server requests screenshot
- ✅ Client returns NULL (screenshot capture fails)
- ✅ No crash occurs
- ✅ Player can continue playing
- ⚠️ Screenshot functionality not available

## How to Verify
1. Build the mod with the fix applied
2. Start a server with JXAC enabled (`jxac_enable 1`)
3. Connect a client to the server
4. Request a screenshot as admin: `!jxac_screenshot <player>`
5. **Expected behavior**: Client does NOT crash, screenshot request fails silently
6. **Previous behavior**: Client would crash immediately

## Future Solutions
To restore screenshot functionality, one of these approaches is needed:

### Option 1: Custom Engine (Recommended)
Modify the ET engine to implement the `CG_R_READPIXELS` syscall:
1. Add handler for syscall 244 in the engine's cgame syscall dispatcher
2. Implement OpenGL framebuffer reading (glReadPixels)
3. Return RGBA buffer to the cgame module
4. This requires a custom ET engine build

### Option 2: File-Based Capture
Use the engine's existing screenshot command:
1. Client triggers screenshot via `trap_SendConsoleCommand("screenshotJPEG ...")`
2. Engine saves screenshot to disk
3. Client reads file from disk
4. Client sends file data to server
5. Client deletes local screenshot file

**Challenges**:
- Asynchronous (need to wait for file to be written)
- File I/O complexity
- Race conditions
- Screenshot naming conflicts

### Option 3: Disable Screenshots
Simply keep screenshots disabled and rely on other anticheat features:
- CVAR scanning
- Module/DLL scanning
- Wallhack detection
- Anti-tamper checks

## Technical Details

### Syscall Flow
```
Client (cgame.mp.x86_64.so)
  └─> trap_R_ReadPixels() in cg_syscalls.cpp
      └─> Engine::ptr(CG_R_READPIXELS, ...)
          └─> ET Engine (et.exe / et.x86 / et.x86_64)
              └─> Syscall dispatcher
                  └─> ❌ CRASH - Unknown syscall 244
```

### Code Location
The problematic call was on line 65 of the original `jxac_screenshot.cpp`:
```cpp
trap_R_ReadPixels( 0, 0, *width, *height, rgbaBuffer );
```

This line has been commented out and the function now returns NULL at line 62.

## Testing Checklist
- [x] Code compiles without errors
- [x] Build succeeds on Linux x86_64
- [x] Documentation updated
- [ ] Manual test: Server requests screenshot, client doesn't crash
- [ ] Manual test: Screenshot request fails gracefully
- [ ] Manual test: Player can continue playing after screenshot request

## Related Files
- `src/cgame/jxac/jxac_screenshot.cpp` - Screenshot capture module
- `src/cgame/jxac/jxac_screenshot.h` - Screenshot capture header
- `src/cgame/jxac/jxac_client.cpp` - JXAC client that calls screenshot module
- `src/cgame/cg_syscalls.cpp` - Defines trap_R_ReadPixels()
- `src/cgame/cg_local.h` - Declares trap_R_ReadPixels()
- `src/cgame/cg_public.h` - Defines CG_R_READPIXELS enum
- `doc/JXAC.md` - JXAC user documentation
- `doc/JXAC_IMPLEMENTATION.md` - JXAC implementation notes
