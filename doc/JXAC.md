# JXAC - Jays XMod AntiCheat

## Overview

JXAC (Jays XMod AntiCheat) is a comprehensive anticheat system for xmod (Wolfenstein: Enemy Territory mod) based on features from Nitmod and PunkBuster.

**Version:** 1.0.0  
**Author:** jay1110  
**License:** Apache 2.0

---

## Features

### Screenshot System
- Server can request screenshots from any connected client
- **Format: JPG** (JPEG format, configurable quality 1-100)
- Screenshots saved to `jxac/screenshots/` directory
- Naming convention: `<playername>_<timestamp>.jpg`
- Detection of blocked or fake screenshots

### Cheat Detection
- **CVAR scanning**: Detect modified/illegal CVARs
- **Wallhack detection**: Check for illegal shader/texture modifications (placeholder)
- **Aimbot detection**: Heuristics for impossible snap angles (placeholder)
- **Speedhack detection**: Server-side movement validation (placeholder)
- **Checksum validation**: Validate pk3 files and client binaries (placeholder)

### Server-Side Features
- Player tracking with JXAC status monitoring
- Violation logging with timestamps to `jxac.log`
- Configurable auto-kick/ban on detection
- Admin notifications of violations
- Screenshot storage with metadata

### Client-Side Features
- Regular heartbeat messages to server
- Screenshot capture and transmission
- CVAR protection
- Anti-tamper detection (placeholder)

---

## Server Configuration

### CVARs

Add these to your server configuration:

```
// Enable/disable JXAC
seta jxac_enable "1"

// Screenshot quality (1-100, default: 85)
seta jxac_screenshotQuality "85"

// Screenshot storage path
seta jxac_screenshotPath "jxac/screenshots/"

// Enable CVAR checking
seta jxac_checkCvars "1"

// Enable wallhack detection
seta jxac_checkWallhack "1"

// Enable speedhack detection
seta jxac_checkSpeedhack "1"

// Auto-ban on detection (0=disabled, 1=enabled)
seta jxac_autoBan "0"

// Auto-kick on detection (0=disabled, 1=enabled)
seta jxac_autoKick "1"

// Log file path
seta jxac_logFile "jxac.log"

// Heartbeat timeout in milliseconds (default: 60000 = 60 seconds)
seta jxac_heartbeatTimeout "60000"

// CVAR configuration file path
seta jxac_cvarFile "jxac/jxac_cvars.cfg"

// Cheat signature database file path
seta jxac_cheatFile "jxac/jxac_cheats.cfg"
```

---

## Admin Commands

All JXAC commands require admin privileges. Use the `!` prefix in-game chat.

| Command | Description | Usage |
|---------|-------------|-------|
| `!jxac_screenshot` | Request screenshot from specific player | `!jxac_screenshot <player> [quality]` |
| `!jxac_screenshotall` | Request screenshot from all players | `!jxac_screenshotall [quality]` |
| `!jxac_status` | Show JXAC status for all or specific player | `!jxac_status [player]` |

**Note:** The `!jxac_kick` and `!jxac_ban` commands have been removed. Use the standard xmod `!kick` and `!ban` commands instead.

### Examples

```
// Request screenshot from player "john" with quality 90
!jxac_screenshot john 90

// Request screenshots from all players with default quality (85)
!jxac_screenshotall

// Show JXAC status for all connected players
!jxac_status

// Show JXAC status for specific player
!jxac_status john

// Use standard xmod commands for kicking/banning:
!kick john Suspected wallhack
!ban john Confirmed aimbot
```

---

## Network Protocol

JXAC uses the following message types for client-server communication:

| Message Type | Direction | Description |
|--------------|-----------|-------------|
| `JXAC_MSG_HEARTBEAT` | Client → Server | Regular status update (every 30 seconds) |
| `JXAC_MSG_SS_REQUEST` | Server → Client | Request screenshot |
| `JXAC_MSG_SS_DATA` | Client → Server | Screenshot data (450 byte chunks, hex-encoded) |
| `JXAC_MSG_SS_COMPLETE` | Client → Server | Screenshot transfer complete |
| `JXAC_MSG_VIOLATION` | Client → Server | Self-reported violation |
| `JXAC_MSG_STATUS` | Server → Client | JXAC status/version check |
| `JXAC_MSG_CVAR_REQUEST` | Server → Client | Request CVAR values |
| `JXAC_MSG_CVAR_RESPONSE` | Client → Server | CVAR values response |

**Note:** Screenshot data is sent in 450-byte chunks (900 hex characters) with 2 chunks per frame to prevent command buffer overflow.

---

## Violation Types

JXAC can detect and log the following violation types:

| Violation Type | Description |
|----------------|-------------|
| `JXAC_VIOLATION_CVAR` | Illegal CVAR detected |
| `JXAC_VIOLATION_WALLHACK` | Wallhack detected |
| `JXAC_VIOLATION_AIMBOT` | Aimbot detected |
| `JXAC_VIOLATION_CHECKSUM` | File checksum mismatch |
| `JXAC_VIOLATION_SS_BLOCKED` | Screenshot blocked/faked |
| `JXAC_VIOLATION_TAMPER` | JXAC client tampered/disabled |
| `JXAC_VIOLATION_NO_RESPONSE` | No response from client |

---

## Recent Updates (v1.0.0)

### Critical Bug Fixes

1. **Fixed Client Command Overflow** - Screenshots larger than ~100KB were causing "Client command overflow" errors. Fixed by implementing a chunk queue system that sends screenshot data at 2 chunks per frame instead of all at once.

2. **Fixed Heartbeat Timeout Spam** - Real players were getting spammed with heartbeat timeout violations every frame. Fixed by adding violation tracking flags that only report each violation type once until cleared.

3. **Removed Duplicate Commands** - Removed redundant `!jxac_kick` and `!jxac_ban` commands since xmod already provides `!kick` and `!ban` commands.

### New Infrastructure

- Added `jxac/jxac_cvars.cfg` template for configurable CVAR checking
- Added `jxac/jxac_cheats.cfg` template for cheat signature database  
- Added `g_jxacHeartbeatTimeout` CVAR (default: 60000ms)
- Added `g_jxacCvarFile` and `g_jxacCheatFile` CVARs
- Added config loading infrastructure (stub methods for future implementation)

---

## Platform Support

JXAC is designed to work on:

- **Windows**: 32-bit (x86) and 64-bit (x64)
- **Linux**: 32-bit (i386) and 64-bit (x86_64)

Both client-side and server-side modules are cross-platform compatible.

---

## Implementation Status

### ✅ Implemented Features

- [x] JXAC common header with shared definitions
- [x] Server-side module with player tracking
- [x] Client-side module with heartbeat
- [x] Screenshot system framework with JPEG compression
- [x] **Screenshot chunk throttling (2 chunks per frame to prevent command overflow)**
- [x] **Heartbeat timeout detection with configurable timeout CVAR**
- [x] **Violation spam prevention (only report once per violation type)**
- [x] Violation logging system
- [x] Admin commands (!jxac_screenshot, !jxac_screenshotall, !jxac_status)
- [x] Server CVARs configuration
- [x] Player status tracking
- [x] **CVAR and cheat configuration file templates**

### 🚧 Placeholder/Partial Implementation

- [ ] Actual screenshot capture (using placeholder gradient pattern)
- [ ] Network protocol integration (requires engine hooks)
- [ ] **CVAR config file parsing (stub methods in place)**
- [ ] **Cheat signature database loading (template file created)**
- [ ] CVAR scanning implementation
- [ ] Wallhack detection heuristics
- [ ] Aimbot detection heuristics
- [ ] Checksum validation
- [ ] Client anti-tamper
- [ ] **Module/DLL scanning (Windows & Linux)**
- [ ] **MD5 checksum calculation**
- [ ] **Memory pattern scanning**

---

## Technical Notes

### Screenshot Implementation

The current implementation provides the framework for screenshot capture, but actual framebuffer capture and JPEG compression are placeholders that require:

1. **Platform-specific framebuffer capture**:
   - Windows: DirectX/OpenGL framebuffer read
   - Linux: X11/OpenGL framebuffer read

2. **JPEG compression library**:
   - Option 1: libjpeg (external dependency)
   - Option 2: stb_image_write.h (single-header library, recommended)

### Network Integration

The current implementation includes placeholder comments for network message sending. Full integration requires:

1. Hooking into the game's network message system
2. Registering JXAC-specific server commands
3. Implementing client command handlers
4. Adding JXAC message parsing on both client and server

---

## Future Enhancements

- **Real-time screenshot viewing**: View screenshots directly in admin panel
- **Automated analysis**: Machine learning-based cheat detection
- **Replay system**: Record suspicious player actions for review
- **Integration with ban databases**: Share ban information across servers
- **Web dashboard**: Monitor JXAC status and violations via web interface

---

## Credits

- **Original Jaymod**: Jaybird
- **JXAC Implementation**: jay1110
- **Inspired by**: Nitmod anticheat, PunkBuster

---

## License

JXAC is part of the xmod project and is released under the Apache 2.0 License.

See LICENSE file for details.
