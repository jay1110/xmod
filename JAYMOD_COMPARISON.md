# Jaymod vs Xmod: CVAR Synchronization Comparison

## Overview

This document compares the CVAR synchronization implementation between:
- **jaymod**: Original repository (budjb/jaymod)
- **xmod**: Fork of jaymod (jay1110/xmod)

## Question

> "can you compare how it is in jaymod repo? budjb/jaymod"

## Answer: **THE BUG EXISTS IN JAYMOD**

The redundant `G_UpdateJaymodCS()` call that causes CVAR synchronization issues **originates from jaymod**. Xmod inherited this bug from the original codebase.

## Detailed Comparison

### Code Structure - IDENTICAL

Both repositories have the same problematic pattern:

#### jaymod (budjb/jaymod)

**File**: `src/game/g_main.cpp`

**Line 1632** - Inside `G_UpdateCvars()`:
```cpp
void G_UpdateCvars( void ) {
    int i;
    cvarTable_t *cv;
    bool jaymodChanged = false;

    Cvar::update();  // ✓ Refreshes CVAR values first

    const list<Cvar*>& vars = Cvar::getUpdateList();
    const list<Cvar*>::const_iterator end = vars.end();

    for ( list<Cvar*>::const_iterator it = vars.begin(); it != end; it++ ) {
        Cvar& v = **it;

        if (v.lastModificationCount == v.modificationCount)
            continue;

        v.lastModificationCount = v.modificationCount;

        if (v.flags & CVAR_JAYMODINFO)
            jaymodChanged = true;
    }
    
    // ... more code ...
    
    if( jaymodChanged ) {
        ammoTableNeedsUpdate = true;
        G_UpdateJaymodCS();  // ✓ CORRECT - called after Cvar::update()
    }
}
```

**Line 1922** - Inside `G_InitGame()`:
```cpp
void G_InitGame( int levelTime, int randomSeed, int restart ) {
    // ... initialization ...
    
    G_RegisterCvars();  // Line 1817 - This calls G_UpdateCvars() at end
    
    // ... lots more initialization ...
    
    trap_SetConfigstring( CS_WATERMARKINFO, cs );

    // Construct the Jaymod Config String
    G_UpdateJaymodCS();  // ✗ PROBLEMATIC - No Cvar::update() first!
    
    G_SoundIndex( "sound/misc/referee.wav" );
}
```

#### xmod (jay1110/xmod) - BEFORE FIX

**File**: `src/game/g_main.cpp`

**Line 1729** - Inside `G_UpdateCvars()`:
```cpp
void G_UpdateCvars( void ) {
    int i;
    cvarTable_t *cv;
    bool xmodChanged = false;

    Cvar::update();  // ✓ Refreshes CVAR values first

    const list<Cvar*>& vars = Cvar::getUpdateList();
    const list<Cvar*>::const_iterator end = vars.end();

    for ( list<Cvar*>::const_iterator it = vars.begin(); it != end; it++ ) {
        Cvar& v = **it;

        if (v.lastModificationCount == v.modificationCount)
            continue;

        v.lastModificationCount = v.modificationCount;

        if (v.flags & CVAR_XMODINFO)
            xmodChanged = true;
    }
    
    // ... more code ...
    
    if( xmodChanged ) {
        ammoTableNeedsUpdate = true;
        G_UpdateXmodCS();  // ✓ CORRECT - called after Cvar::update()
    }
}
```

**Line 2032** - Inside `G_InitGame()` (BEFORE FIX):
```cpp
void G_InitGame( int levelTime, int randomSeed, int restart ) {
    // ... initialization ...
    
    G_RegisterCvars();  // Line 1914 - This calls G_UpdateCvars() at end
    
    // ... lots more initialization ...
    
    trap_SetConfigstring( CS_WATERMARKINFO, cs );

    // Construct the Xmod Config String
    G_UpdateXmodCS();  // ✗ PROBLEMATIC - No Cvar::update() first!
    
    G_SoundIndex( "sound/misc/referee.wav" );
}
```

### Line Number Comparison

| Component | jaymod | xmod (before fix) |
|-----------|--------|-------------------|
| `G_UpdateCvars()` definition | Line 1470 | Line 1567 |
| Correct `G_Update*CS()` call in `G_UpdateCvars()` | Line 1632 | Line 1729 |
| `G_RegisterCvars()` definition | Line 1420 | Line 1517 |
| `G_RegisterCvars()` calls `G_UpdateCvars()` | Line 1665 | Line 1762 |
| `G_InitGame()` definition | Line 1778 | Line 1875 |
| `G_InitGame()` calls `G_RegisterCvars()` | Line 1817 | Line 1914 |
| **Problematic redundant call** | **Line 1922** | **Line 2032** |

### Shared Components

Both repositories have:

1. **New Cvar System**
   - `src/bgame/cvar/Cvar.cpp` and `Cvar.h`
   - `src/game/static.cpp` for Cvar object definitions
   - `Cvar::init()` and `Cvar::update()` methods

2. **Dual CVAR Systems**
   - New C++ Cvar objects in `cvar::objects` namespace
   - Old vmCvar_t in gameCvarTable array

3. **CVAR Flags**
   - jaymod: `CVAR_JAYMODINFO`
   - xmod: `CVAR_XMODINFO`
   - Both used to mark CVARs for client synchronization

4. **Config String**
   - jaymod: `CS_JAYMODINFO`
   - xmod: `CS_XMODINFO`

## The Bug Mechanism (Same in Both)

### What Happens

1. **First Call (Correct)**
   - `G_RegisterCvars()` → `G_UpdateCvars()` → `Cvar::update()` → `G_Update*CS()`
   - CVARs have correct values
   - Configstring is properly populated

2. **Second Call (Problematic)**
   - `G_InitGame()` directly calls `G_Update*CS()`
   - No `Cvar::update()` is called first
   - Uses potentially stale `_data.string` values
   - If CVARs were just constructed, `_data.string` might be empty
   - Empty strings → clients interpret as "0"

### Why It Causes Issues

- **Timing dependent**: Sometimes works, sometimes fails
- **Value dependent**: Affects CVARs with non-default startup values
- **Initialization order**: Depends on when CVARs get registered vs when configstring is set

## Impact Analysis

### Both Repositories Affected

The bug affects any CVARs marked with `CVAR_JAYMODINFO`/`CVAR_XMODINFO`:

**jaymod CVARs** (from `src/game/static.cpp`):
- Class restrictions: `bg_maxEngineers`, `bg_maxMedics`, `bg_maxFieldOps`, `bg_maxCovertOps`
- Weapon restrictions: `bg_maxPanzers`, `bg_maxMG42s`, `bg_maxMortars`, etc.
- Game settings: `bg_dynamiteTime`, `bg_glow`, `bg_misc`, `bg_panzerWar`, etc.

**xmod CVARs**: Same as jaymod, plus any additional xmod-specific CVARs

### Observable Symptoms (Both)

- Limbo panel shows classes as disabled (0) when server has them enabled (-1)
- Class restrictions not enforced correctly on initial connection
- Runtime changes via rcon work (because `G_UpdateCvars()` in game loop handles it)

## The Fix

### xmod - FIXED (Feb 5, 2026)

**Commit**: `d3d4217` - "Fix: Remove redundant G_UpdateXmodCS() call that used stale CVAR values"

**Change**: Removed line 2032 in `G_InitGame()`:
```cpp
// BEFORE
trap_SetConfigstring( CS_WATERMARKINFO, cs );

// Construct the Xmod Config String
G_UpdateXmodCS();  // REMOVED

G_SoundIndex( "sound/misc/referee.wav" );

// AFTER
trap_SetConfigstring( CS_WATERMARKINFO, cs );

// Note: G_UpdateXmodCS() is called automatically by G_UpdateCvars() in G_RegisterCvars()
// No need to call it again here as it would use potentially stale CVAR values

G_SoundIndex( "sound/misc/referee.wav" );
```

### jaymod - NOT FIXED (as of Feb 5, 2026)

The redundant call still exists at line 1922. The same fix should be applied.

## Recommended Fix for jaymod

Remove the redundant `G_UpdateJaymodCS()` call from `G_InitGame()`:

```diff
--- a/src/game/g_main.cpp
+++ b/src/game/g_main.cpp
@@ -1919,8 +1919,8 @@ void G_InitGame( int levelTime, int randomSeed, int restart ) {
 	Info_SetValueForKey( cs, "wmFN", g_watermark.string );
 	trap_SetConfigstring( CS_WATERMARKINFO, cs );
 
-	// Construct the Jaymod Config String
-	G_UpdateJaymodCS();
+	// Note: G_UpdateJaymodCS() is called automatically by G_UpdateCvars() in G_RegisterCvars()
+	// No need to call it again here as it would use potentially stale CVAR values
 
 	G_SoundIndex( "sound/misc/referee.wav"	);
 	G_SoundIndex( "sound/misc/vote.wav"		);
```

## Conclusion

### Key Findings

1. ✅ **Bug originated in jaymod** - Not an xmod regression
2. ✅ **xmod faithfully inherited the code** - Including the bug
3. ✅ **The fix is correct** - Applies to both codebases
4. ✅ **jaymod would benefit** - Same fix should be applied there

### Timeline

- **Unknown**: Bug introduced in jaymod (before xmod fork)
- **Jan 24, 2026**: xmod repository created with bug already present
- **Feb 5, 2026**: Bug discovered and fixed in xmod
- **Feb 5, 2026**: Comparison shows jaymod still has the bug

### Recommendations

1. **For xmod**: Bug is fixed ✓
2. **For jaymod**: Consider submitting a pull request with the same fix
3. **For users**: Upgrade to fixed xmod version or apply patch to jaymod

---

**Analysis Date**: February 5, 2026  
**Analyzed Repositories**:
- jaymod: https://github.com/budjb/jaymod (commit: latest as of Feb 5, 2026)
- xmod: https://github.com/jay1110/xmod (commit: e86f82a with fix applied)
