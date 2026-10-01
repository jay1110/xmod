Weapon Scripts Documentation
============================

Weapon scripts allow server administrators to customize weapon properties
through .weap files. This system is based on NoQuarter's weaponscripts system.

Configuration
-------------

Set the g_weaponScriptsDir cvar to point to your weapon scripts directory:

    set g_weaponScriptsDir "weapons_custom"

This will load weapon scripts from the specified directory. Place your .weap 
files in that directory with the following naming convention:

    knife.weap
    thompson.weap
    mp40.weap
    etc.

File Format
-----------

Weapon scripts use a simple text format with nested sections:

```
weaponDef
{
    both {
        // Server-side properties that affect gameplay
        name                "Thompson"
        damage              18
        spread              400
        headshotWeapon
        
        // Ammo settings
        maxammo             90
        maxclip             30
        startammo           30
        startclip           30
        
        // Timing settings (in milliseconds)
        reloadTime          2400
        fireDelayTime       100
        nextShotTime        150
        
        // Heat settings (for weapons that overheat)
        maxHeat             0
        coolRate            0
        
        // Kill messages
        selfKillMessage     "found a way to shoot himself!"
        KillMessage         "was killed by"
        KillMessage2        "'s Thompson."
    }

    client {
        // Client-side visual/audio properties
        // (These are processed by the client game, not server)
    }
}
```

Available Properties (both section)
-----------------------------------

### Basic Info
- name          - Display name of the weapon
- statname      - Name used in stats display

### Damage
- damage        - Base damage per hit
- splashdamage  - Splash damage amount
- splashdamage_radius / splashRadius - Splash damage radius

### Spread
- spread        - Base spread value
- spreadRatio   - Spread ratio multiplier (float, e.g., 0.6)

### Ammo
- uses          - Rounds consumed per shot (0 allows firing without consuming ammo)
- maxammo       - Maximum ammo capacity
- maxclip       - Maximum clip size  
- startammo     - Starting ammo
- startclip     - Starting clip

### Timing (all in milliseconds)
- reloadTime    - Time to reload
- fireDelayTime - Delay before first shot fires
- nextShotTime  - Time between consecutive shots

### Heat (for weapons that overheat)
- maxHeat       - Maximum heat before overheat
- coolRate      - Cooling rate

### Flags (yes/no, true/false, 1/0; presence alone means yes)
- headshotWeapon    - Weapon can headshot
- bulletReflection  - Opt-in single bullet ricochet from solid, non-sky surfaces
- DistanceFalloff   - Damage falls off with distance
- GibbingWeapon     - Allow or prevent this weapon from gibbing players

### Movement
- movementSpeedScale - Positive multiplier replacing the usual weapon slowdown;
                       0 keeps normal ET movement (including heavy-weapon penalties)

### Kill Messages
- selfKillMessage   - Message when player kills self
- KillMessage       - Kill message prefix
- KillMessage2      - Kill message suffix

Example: thompson.weap
----------------------

```
weaponDef
{
    both {
        name                "Thompson"
        statname            "Thompson"
        damage              18
        spread              400
        spreadRatio         0.6
        headshotWeapon
        DistanceFalloff

        maxammo             90
        maxclip             30
        startammo           30
        startclip           30
        
        reloadTime          2400
        fireDelayTime       100
        nextShotTime        150
        
        selfKillMessage     "found a way to shoot himself!"
        KillMessage         "was killed by"
        KillMessage2        "'s Thompson."
    }

    client {
        // Client visual properties...
    }
}
```

Notes
-----

1. Gameplay values in "both" and "both_altweap" are read by the server and
   synchronized to client prediction, including after vid_restart.
2. "client" defines local models, sounds, animations and icons. Custom media must
   be present in a PK3 downloaded by the client. The custom directory is sent in
   serverinfo. Missing/invalid custom files fall back to weapons/<name>.weap.
3. An omitted property keeps its normal default; explicit zero and false values
   are applied. Negative/non-finite/invalid numeric values reject the entire file.
   Integer values are limited to 65535; spreadRatio/movementSpeedScale to 100.
4. Both // and /* */ comments and UTF-8 BOMs are accepted. Invalid files never
   partially change a weapon. Unsupported properties produce a server warning.
5. g_weaponScriptsDir is latched: change it, then restart/change the map. Empty
   disables gameplay scripts. Repeated map restarts restore clean defaults first.
6. Global gameplay modes (unlimited ammo, firing delay offsets, fair rifles,
   no-overheat settings, etc.) still apply after the script ammo/timing values.
7. Scoped/set values come from both_altweap in the base weapon's file:
   m1_garand_s, k43, fg42, mg42 and mortar. Omitted alternate values keep the
   normal defaults. These weapons do not read separate scoped/set gameplay files.
8. damage is base hit damage; normal skills, hit-region bonuses and game modes
   can still adjust it. splashdamage and splashRadius apply independently.
   For gas, damage/radius control its periodic effect; for Molotov, splashdamage
   and splashRadius apply to each fire patch. Zero damage does no damage.
9. statname renames the shared statistics category. If multiple weapons sharing
   a category set a statname, the highest weapon index wins consistently on both
   server and client. name controls the weapon's display/kill-message name.

Supported Weapons
-----------------

The following weapon script filenames are supported:

- knife.weap
- luger.weap
- mp40.weap
- grenade.weap
- panzerfaust.weap
- flamethrower.weap
- colt.weap
- thompson.weap
- pineapple.weap
- sten.weap
- syringe.weap
- silenced_luger.weap
- dynamite.weap
- medpack.weap
- binocs.weap
- pliers.weap
- smokemarker.weap
- kar98.weap
- m1_garand.weap
- m1_garand_s.weap
- landmine.weap
- satchel.weap
- satchel_det.weap
- tripmine.weap
- smokegrenade.weap
- mg42.weap
- k43.weap
- fg42.weap
- mortar.weap
- akimbo_colt.weap
- akimbo_luger.weap
- gpg40.weap
- m7.weap
- silenced_colt.weap (Colt with silencer)
- adrenaline.weap
- akimbo_silenced_colt.weap
- akimbo_silenced_luger.weap
- poison.weap
- adrenaline_share.weap
- m97.weap
- poison_gas.weap
- landmine_bbetty.weap
- landmine_pgas.weap
- molotov.weap
- bombax.weap (Axis bomb)
- bomb.weap (Allied bomb)
- ppsh.weap
- arty.weap
- mapmortar.weap
- ammopack.weap
