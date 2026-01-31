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

### Flags (no value, just presence)
- headshotWeapon    - Weapon can headshot
- bulletReflection  - Bullets can reflect
- DistanceFalloff   - Damage falls off with distance

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

1. Only properties in the "both" section are processed by the server
2. The "client" section is processed by the client game module
3. Properties not specified will use default values from ammoTableMP
4. Weapon scripts are loaded during game initialization
5. Changes to weapon scripts require a map restart to take effect

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
- grenade_pineapple.weap
- sten.weap
- syringe.weap
- silencer.weap
- dynamite.weap
- medkit.weap
- binoculars.weap
- pliers.weap
- smoke_marker.weap
- kar98.weap
- carbine.weap
- garand.weap
- landmine.weap
- satchel.weap
- tripmine.weap
- smoke_bomb.weap
- mobile_mg42.weap
- k43.weap
- fg42.weap
- mortar.weap
- akimbo_colt.weap
- akimbo_luger.weap
- gpg40.weap
- m7.weap
- silenced_colt.weap
- garand_scope.weap
- k43_scope.weap
- fg42_scope.weap
- mortar_set.weap
- adrenaline.weap
- akimbo_silencedcolt.weap
- akimbo_silencedluger.weap
- mobile_mg42_set.weap
- poison_syringe.weap
- m97.weap
- poison_gas.weap
- landmine_bbetty.weap
- landmine_pgas.weap
- molotov.weap
