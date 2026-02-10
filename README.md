# Xmod

Xmod is a popular server-side modification for **Wolfenstein: Enemy Territory** that adds extensive gameplay features, admin tools, and customization options. This repository contains version 2.0.0.

## Table of Contents

- [Features](#features)
  - [Gameplay Features](#gameplay-features)
  - [Admin System](#admin-system)
  - [Lua Scripting](#lua-scripting)
  - [Bot Support](#bot-support)
- [Installation](#installation)
- [Server Configuration](#server-configuration)
  - [Key CVARs](#key-cvars)
  - [Game Modes](#game-modes)
- [Client Settings](#client-settings)
  - [HUD Customization](#hud-customization)
  - [Weapon Display](#weapon-display)
  - [Display Settings](#display-settings)
  - [Sound Settings](#sound-settings)
- [Admin Commands](#admin-commands)
- [Building from Source](#building-from-source)
- [Platform Support](#platform-support)
- [License](#license)

---

## Features

### Gameplay Features

| Feature | Description | CVAR |
|---------|-------------|------|
| **XP Save** | Saves player XP across map changes and reconnects | `g_xpSave` |
| **Killing Spree** | Announces killing sprees and tracks records | `g_killingSpree` |
| **Goomba Kills** | Damage from landing on other players (Mario-style) | `g_goomba` |
| **Player Shove** | Push teammates out of your way | `g_shove` |
| **Corpse Dragging** | Drag dead bodies to safety for revival | `g_dragCorpse` |
| **Play Dead** | Fake death to surprise enemies | `g_playDead` |
| **Poison Syringes** | Medics can poison enemies with special syringes | `g_poisonSyringes` |
| **Anti-Warp** | Prevents warping exploits from laggy players | `g_antiwarp` |
| **Private Messages** | Send private messages between players | `g_privateMessages` |
| **Banners** | Rotating server messages | `g_banners` |
| **Watermark** | Display server watermark on screen | `g_watermark` |
| **Trip Mines** | Engineers (skill lvl 3+) can place laser-triggered tripmine explosives | `g_weapons` (8192) |

### Fun Game Modes

| Mode | Description | CVAR |
|------|-------------|------|
| **Panzer War** | All players spawn with Panzerfausts, infinite ammo, reduced damage | `g_panzerWar` |
| **Sniper War** | All players spawn as Covert Ops with sniper rifles, instant headshot kills | `g_sniperWar` |
| **Knife Only** | Melee-only combat | `g_knifeonly` |

### Admin System

Xmod includes a powerful admin system with multi-level permissions:

- **Level-based administration** - Assign different permission levels to admins
- **User database** - Persistent storage of admin accounts and bans
- **Ban system** - IP and GUID-based banning with duration support
- **Admin logging** - Track all admin actions
- **Custom Commands** - Define your own admin commands via `commands.db`

Enable with: `g_admin 1`

### Lua Scripting

Lua 5.4.7 is fully integrated for server-side scripting:

- Create custom game logic
- Extend admin commands
- Add new gameplay features
- Supports up to 64 Lua VMs simultaneously

**Supported Platforms:**
- Linux 32-bit (i386)
- Linux 64-bit (x86_64)
- Windows 32-bit (x86)
- Windows 64-bit (x64)

Lua is compiled as a static library—no external installation required.

### Bot Support

Full **Omnibot** integration for AI-controlled players:

- Enable with: `omnibot_enable 1`
- Requires Omnibot library files (not included)

---

## Installation

1. Download the latest release from the [Releases](../../releases) page
2. Extract files to your ET server's mod folder:
   ```
   /path/to/enemy-territory/xmod/
   ```
3. Start your server with `+set fs_game xmod`

### Required Files

```
xmod/
├── xmod-2.0.0.pk3      # Client-side assets (cgame, ui)
├── qagame.mp.*.so      # Linux server module
├── qagame_mp_*.dll     # Windows server module
├── xmod.cfg            # Mod configuration
└── server.cfg          # Server configuration
```

---

## Server Configuration

### Key CVARs

#### Gameplay Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `g_xpSave` | 0 | XP save mode (0=off, 1=on, 2=reset on campaign) |
| `g_xpSaveTimeout` | 0 | Time in seconds before saved XP expires |
| `g_xpMax` | 0 | Maximum XP a player can earn (0=unlimited) |
| `g_killingSpree` | 0 | Killing spree announcements (0=off, 1=on, 2=record) |
| `g_goomba` | 0 | Goomba damage multiplier (0=disabled) |
| `g_shove` | 0 | Shove distance (0=disabled, try 100) |
| `g_dragCorpse` | 0 | Enable corpse dragging |
| `g_playDead` | 0 | Enable play dead feature |
| `g_poisonSyringes` | 0 | Enable poison syringes |
| `g_antiwarp` | 1 | Anti-warp protection flags |

#### Team Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `team_maxMedics` | -1 | Max medics per team (-1=unlimited) |
| `team_maxEngineers` | -1 | Max engineers per team |
| `team_maxFieldOps` | -1 | Max field ops per team |
| `team_maxCovertOps` | -1 | Max covert ops per team |
| `team_maxPanzers` | -1 | Max players with Panzerfaust |
| `team_maxMortars` | -1 | Max players with mortar |

#### Admin Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `g_admin` | 0 | Enable admin system |
| `g_adminLog` | "" | Admin action log file |
| `g_kickTime` | 120 | Default kick duration (seconds) |
| `g_muteTime` | 60 | Default mute duration (seconds) |

---

## Client Settings

These CVARs are set by players in their client console. All are saved to config (`CVAR_ARCHIVE`).

### HUD Customization

| CVAR | Default | Description |
|------|---------|-------------|
| `cg_hudAlpha` | 1.0 | Global HUD transparency (0.0=invisible, 1.0=fully opaque). Affects FPS, speed, clock, timer, fireteam overlay, and lagometer backgrounds. |
| `cg_hudBackgroundColor` | "0.16 0.2 0.17 0.8" | HUD element background color as "R G B A" (values 0.0-1.0). Applied to FPS counter, speed display, clock, timer, fireteam overlay, and lagometer. |
| `cg_hudBorderColor` | "0.5 0.5 0.5 0.5" | HUD element border color as "R G B A" (values 0.0-1.0). |
| `cg_althud` | 0 | Use alternative HUD layout |
| `cg_drawClock` | 0 | Display local clock on screen |
| `cg_drawSpeed` | 0 | Display player movement speed |
| `cg_speedRefresh` | 100 | Speed display refresh interval (ms) |
| `cg_drawDisconnectIcon` | 0 | Show disconnect icon for lagging players |

### Weapon Display

| CVAR | Default | Description |
|------|---------|-------------|
| `cg_drawGun` | 1 | First-person weapon display mode. 1=normal, 2-32=transparent colored weapon (see color table below), 0=hidden |
| `cg_drawGunAlpha` | 128 | Weapon transparency when using colored modes (`cg_drawGun` 2-32). Range 0-255 (0=invisible, 255=fully opaque). |
| `cg_muzzleFlash` | 1 | Show muzzle flash effects |

### Display Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `cg_console` | 0 | Custom console display mode |
| `cg_consoleShadowed` | 1 | Draw shadows on console text |
| `cg_countryflags` | 1 | Show country flags next to player names |
| `cg_watermarkOpacity` | 1.0 | Server watermark opacity (0.0-1.0) |
| `cg_mapZoom` | 5.159 | Command map zoom level |
| `cg_locationMode` | 0 | Location display mode |
| `cg_locationMaxChars` | 25 | Max characters for location display |
| `cg_locationJustify` | 0 | Location text justification |
| `cg_obituaryLocation` | 0 | Show location in kill messages |
| `cg_obituaryFilter` | 0 | Filter kill messages |

### Popup Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `cg_popupTime` | 0 | Popup message display duration |
| `cg_popupFadeTime` | 2500 | Popup message fade duration (ms) |
| `cg_popupWaitTime` | 5000 | Time between popup messages (ms) |
| `cg_numPopups` | 10 | Maximum number of popup messages on screen |

### Sound Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `cg_hitsounds` | 1 | Play hit confirmation sounds |
| `cg_killspreesounds` | 1 | Play killing spree announcement sounds |
| `cg_pmsounds` | 1 | Play private message notification sounds |

### Network Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `cg_delag` | 1 | Client-side delag (anti-lag compensation) |
| `cg_pmblock` | 0 | Block incoming private messages |
| `cg_autoRate` | 1 | Automatically adjust network rate |

---

## Admin Commands

Admin commands use the `!` prefix (e.g., `!kick player`).

### Player Management

| Command | Description | Usage |
|---------|-------------|-------|
| `!kick` | Kick a player | `!kick <player> [reason]` |
| `!ban` | Ban a player | `!ban <player> [duration] [reason]` |
| `!unban` | Remove a ban | `!unban <ban#>` |
| `!mute` | Mute a player | `!mute <player> [duration]` |
| `!unmute` | Unmute a player | `!unmute <player>` |
| `!putteam` | Move player to team | `!putteam <player> <r/b/s>` |
| `!spec` | Move player to spectator | `!spec <player>` |
| `!rename` | Rename a player | `!rename <player> <newname>` |

### Fun Commands

| Command | Description | Usage |
|---------|-------------|-------|
| `!slap` | Slap a player (damage + knockback) | `!slap <player>` |
| `!smite` | Kill a player with lightning | `!smite <player>` |
| `!fling` | Launch a player into the air | `!fling <player>` |
| `!throw` | Throw a player in view direction | `!throw <player>` |
| `!splat` | Flatten a player | `!splat <player>` |
| `!shake` | Shake a player's screen | `!shake <player>` |
| `!disorient` | Disorient a player's controls | `!disorient <player>` |
| `!glow` | Make a player glow | `!glow <player> <color>` |
| `!pants` | Remove a player's pants | `!pants <player>` |
| `!chicken` | Turn player into a chicken | `!chicken <player>` |
| `!crybaby` | Make player cry | `!crybaby <player>` |

### Server Management

| Command | Description | Usage |
|---------|-------------|-------|
| `!nextmap` | Skip to next map | `!nextmap` |
| `!restart` | Restart current map | `!restart` |
| `!reset` | Reset match | `!reset` |
| `!pause` | Pause the game | `!pause` |
| `!unpause` | Unpause the game | `!unpause` |
| `!lock` | Lock a team | `!lock <r/b>` |
| `!unlock` | Unlock a team | `!unlock <r/b>` |
| `!shuffle` | Shuffle teams by XP | `!shuffle` |
| `!swap` | Swap teams | `!swap` |

### Level Management

| Command | Description | Usage |
|---------|-------------|-------|
| `!setlevel` | Set player's admin level | `!setlevel <player> <level>` |
| `!listplayers` | List all players | `!listplayers` |
| `!levlist` | List admin levels | `!levlist` |
| `!levinfo` | Show level details | `!levinfo <level>` |
| `!levadd` | Add admin level | `!levadd <level> <name>` |
| `!levdelete` | Delete admin level | `!levdelete <level>` |

### Information Commands

| Command | Description | Usage |
|---------|-------------|-------|
| `!help` | Show command help | `!help [command]` |
| `!about` | Show mod information | `!about` |
| `!status` | Show server status | `!status` |
| `!time` | Show server time | `!time` |
| `!uptime` | Show server uptime | `!uptime` |
| `!finger` | Show player info | `!finger <player>` |
| `!seen` | When player was last seen | `!seen <name>` |

### Custom Commands

Xmod supports user-defined admin commands via a `commands.db` file. This allows server administrators to create custom commands without modifying the mod code.

**Setup:**
1. Create a `commands.db` file in your xmod folder (e.g., `etmain/xmod/commands.db`)
2. See `pkg/commands.db.sample` for format and examples

**File Format:**
```
**********

name    = beer
exec    = qsay [n] ^7gives everyone a cold ^3Beer!!!
desc    = Announce beer time
levels  = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 999
```

**Variable Substitutions:**
| Variable | Description |
|----------|-------------|
| `[n]` | Name of the player executing the command |
| `[d]` | Name of the first argument if it's a valid player |
| `[1]` | First command argument |
| `[2]` | Second command argument |
| `[3]-[9]` | Additional command arguments |

**Example Commands:**
```
# AFK command - puts yourself in spectator
**********
name    = afk
exec    = chat "^1[n] ^7is going AFK!";!putteam [n] s
desc    = Puts yourself in spectator with AFK message
levels  = 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 999

# Screenshot command - take screenshot of a player
**********
name    = ss
exec    = getss [1]
desc    = Takes screenshot from selected player
levels  = 8, 9, 10, 11, 999
```

---

## Building from Source

### Quick Start Options

**Option 1: Visual Studio 2022 (Windows) - Build All Platforms**
```powershell
# Builds Windows (VS2022) + Linux (WSL2) and creates complete release package
.\build-all.ps1
```
See [docs/BUILD_VS2022.md](docs/BUILD_VS2022.md) for detailed instructions.

**Option 2: GNU Make (Linux/MinGW) - Build Single Platform**
```bash
# Linux 64-bit
PLATFORM=linux64 make release

# Linux 32-bit (requires gcc-multilib g++-multilib)
PLATFORM=linux make release

# Windows 32-bit (cross-compile via MinGW)
PLATFORM=mingw make release

# Windows 64-bit (cross-compile via MinGW)
PLATFORM=mingw64 make release
```

**Option 3: Android NDK (Linux) - Build Android Libraries**
```bash
# Android ARM64 (requires Android NDK)
PLATFORM=android-arm64 make release

# Android x86_64
PLATFORM=android-x86_64 make release

# Android x86 (32-bit)
PLATFORM=android-x86 make release
```
See [docs/BUILD_ANDROID.md](docs/BUILD_ANDROID.md) for detailed instructions.

### Visual Studio 2022 Build System

**New!** Complete cross-platform build system for Windows developers:

- **Visual Studio 2022** - Build Windows 32-bit and 64-bit natively using MSVC
- **WSL2 Integration** - Build Linux 32-bit and 64-bit from Windows
- **One-Click Release** - Creates complete release package matching GitHub workflow output

**Prerequisites:**
- Visual Studio 2022 with "Desktop development with C++" workload
- WSL2 with Ubuntu (optional, for Linux builds)

**Quick Build:**
```powershell
.\build-all.ps1
```

**Output:** `release/xmod-2.0.0.zip` with all platform binaries, pk3 file, and configs.

📖 **Full Documentation:** [docs/BUILD_VS2022.md](docs/BUILD_VS2022.md)

### GNU Make Build System

**Prerequisites:**
- GCC/G++ (Linux) or MinGW-w64 (Windows cross-compile)
- GNU Make 3.80+
- Python 3.x
- GNU M4

**Build Targets:**

| Target | Description |
|--------|-------------|
| `make` | Build default variant |
| `make release` | Build release variant |
| `make debug` | Build debug variant |
| `make clean` | Clean build output |
| `make pkg` | Build and package |

📖 **Full Documentation:** [BUILD.md](BUILD.md) | [notes/BuildSystem.txt](notes/BuildSystem.txt)

### GitHub Actions - Automated Builds

**Don't want to build locally?** Use GitHub Actions to build automatically on every pull request!

**Quick Build (3-5 minutes):**
- Automatically builds on every PR push
- Linux 64-bit binaries only
- Download artifacts from Actions tab
- Perfect for rapid testing

**Build Multi-Platform (15-30 minutes):**
- Builds all platforms (Linux, Windows, macOS, Android)
- Available via manual trigger or automatic on main/develop
- Complete release package

**How to use:**
1. Push to your PR branch
2. Go to [Actions tab](../../actions)
3. Download artifacts when build completes

**You never need to close the PR to get builds!**

📖 **Full Documentation:** [.github/USING_GITHUB_ACTIONS.md](.github/USING_GITHUB_ACTIONS.md)

---

## Platform Support

| Platform | Architecture | Build Command | Output |
|----------|--------------|---------------|--------|
| Linux | i386 (32-bit) | `PLATFORM=linux make` | `qagame.mp.i386.so` |
| Linux | x86_64 (64-bit) | `PLATFORM=linux64 make` | `qagame.mp.x86_64.so` |
| Windows | x86 (32-bit) | `PLATFORM=mingw make` | `qagame_mp_x86.dll` |
| Windows | x64 (64-bit) | `PLATFORM=mingw64 make` | `qagame_mp_x64.dll` |
| Android | ARM64-v8a | `PLATFORM=android-arm64 make` | `libqagame.mp.android.arm64-v8a.so` |
| Android | x86_64 (64-bit) | `PLATFORM=android-x86_64 make` | `libqagame.mp.android.x86_64.so` |
| Android | x86 (32-bit) | `PLATFORM=android-x86 make` | `libqagame.mp.android.i386.so` |

All platforms include full Lua 5.4.7 integration.

📖 **Android Build Guide:** [docs/BUILD_ANDROID.md](docs/BUILD_ANDROID.md)

---

## License

This source is bound to the original terms from **id Software**. Additionally, this source is released under the **Apache 2.0 License**.

Feel free to use this codebase as you please, as long as both licenses are bundled and proper credit is given.

---

## Credits

- **Original Xmod** - Jaybird
- **Current Maintainer** - jay1110
- **Anti-Warp Code** - Zinx (June 2007)
- **Lua Integration** - Based on ET Legacy implementation

## Links

- [GitHub Repository](https://github.com/jay1110/xmod)
- [Wolfenstein: Enemy Territory](https://www.splashdamage.com/games/wolfenstein-enemy-territory/)

