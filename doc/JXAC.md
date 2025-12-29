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
```

---

## Admin Commands

All JXAC commands require admin privileges. Use the `!` prefix in-game chat.

| Command | Description | Usage |
|---------|-------------|-------|
| `!jxac_screenshot` | Request screenshot from specific player | `!jxac_screenshot <player> [quality]` |
| `!jxac_screenshotall` | Request screenshot from all players | `!jxac_screenshotall [quality]` |
| `!jxac_status` | Show JXAC status for all or specific player | `!jxac_status [player]` |
| `!jxac_kick` | Kick player detected by JXAC | `!jxac_kick <player> [reason]` |
| `!jxac_ban` | Ban player detected by JXAC | `!jxac_ban <player> [reason]` |

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

// Kick player with custom reason
!jxac_kick john Suspected wallhack

// Ban player
!jxac_ban john Confirmed aimbot
```

---

## Network Protocol

JXAC uses the following message types for client-server communication:

| Message Type | Direction | Description |
|--------------|-----------|-------------|
| `JXAC_MSG_HEARTBEAT` | Client → Server | Regular status update (every 30 seconds) |
| `JXAC_MSG_SS_REQUEST` | Server → Client | Request screenshot |
| `JXAC_MSG_SS_DATA` | Client → Server | Screenshot data (8KB chunks) |
| `JXAC_MSG_SS_COMPLETE` | Client → Server | Screenshot transfer complete |
| `JXAC_MSG_VIOLATION` | Client → Server | Self-reported violation |
| `JXAC_MSG_STATUS` | Server → Client | JXAC status/version check |
| `JXAC_MSG_CVAR_REQUEST` | Server → Client | Request CVAR values |
| `JXAC_MSG_CVAR_RESPONSE` | Client → Server | CVAR values response |

---

## Violation Types

JXAC can detect and log the following violation types:

| Violation Type | Description |
|----------------|-------------|
| `JXAC_VIOLATION_CVAR` | Illegal CVAR detected |
| `JXAC_VIOLATION_WALLHACK` | Wallhack detected |
| `JXAC_VIOLATION_AIMBOT` | Aimbot detected |
| `JXAC_VIOLATION_SPEEDHACK` | Speedhack detected |
| `JXAC_VIOLATION_CHECKSUM` | File checksum mismatch |
| `JXAC_VIOLATION_SS_BLOCKED` | Screenshot blocked/faked |
| `JXAC_VIOLATION_TAMPER` | JXAC client tampered/disabled |
| `JXAC_VIOLATION_NO_RESPONSE` | No response from client |

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
- [x] Screenshot system framework
- [x] Violation logging system
- [x] Admin commands (!jxac_screenshot, !jxac_screenshotall, !jxac_status, !jxac_kick, !jxac_ban)
- [x] Server CVARs configuration
- [x] Player status tracking
- [x] Heartbeat timeout detection

### 🚧 Placeholder/Partial Implementation

- [ ] Actual screenshot capture (platform-specific)
- [ ] JPEG compression (requires libjpeg or stb_image_write)
- [ ] Network protocol integration (requires engine hooks)
- [ ] CVAR scanning implementation
- [ ] Wallhack detection heuristics
- [ ] Aimbot detection heuristics
- [ ] Speedhack detection
- [ ] Checksum validation
- [ ] Client anti-tamper

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
