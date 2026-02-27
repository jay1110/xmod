#ifndef BGAME_XMOD_H
#define BGAME_XMOD_H

///////////////////////////////////////////////////////////////////////////////

// Various things both games should see

// Shotgun
#define M97_COUNT	11
#define	M97_SPREAD	700

// g_weapons defines
#define SBW_NOSTEN_HEAT    0x00000001 //       1 - Disable Sten overheating
#define SBW_NOMG42_HEAT    0x00000002 //       2 - Disable Mobile MG42 overheating
#define SBW_MG42_HEADSHOTS 0x00000004 //       4 - Allow headshots with Mobile MG42
#define SBW_FIRE_LEAN      0x00000008 //       8 - Allow players to fire while leaning
#define SBW_ENGI           0x00000010 //      16 - Allow Medics and Engineers to pick Sten in limbo
#define SBW_MEDIC          0x00000010 //      16 - (alias for engineers)
#define SBW_FIRE_UNDERWATER 0x00000020 //     32 - Fire underwater
#define SBW_FOPS           0x00000040 //      64 - Field-Ops with level 0 battle-sense do not spawn with binoculars
#define SBW_SYRINGE_WATER  0x00000080 //     128 - syringes function underwater
#define SBW_PLIERS_WATER   0x00000100 //     256 - pliers function underwater
#define SBW_ENG_BOMB       0x00000200 //     512 - Level 5 Engineers will spawn with a Bomb
#define SBW_ENG_MINE_DMG   0x00000400 //    1024 - Level 5 Engineers' landmines inflict +15% damage
#define SBW_BOMB_MOVERS    0x00000800 //    2048 - Allow Engineer's "Bomb" to damage movers
#define SBW_FULLBAR        0x00001000 //    4096 - "Too many air strikes requested" will restore used charge bar
#define SBW_HALFBAR        0x00002000 //    8192 - "Too many air strikes requested" will restore half of used charge bar
#define SBW_HELMET         0x00004000 //   16384 - ammo packs restore a lost helmet
#define SBW_DROPBINOCS     0x00008000 //   32768 - players with binoculars drop them upon death
#define SBW_FAIRRIFLES     0x00010000 //   65536 - allies reload rifles mid-clip
#define SBW_FASTSHOOTING   0x00020000 //  131072 - Enable fast shooting for MP40, Thompson and Sten
#define SBW_KNIFE_HEADSHOT 0x00040000 //  262144 - Knife can get headshots
#define SBW_THKNIFE_HEADSHOT 0x00080000 // 524288 - Throwing knife can get headshots
#define SBW_PSTHKNIVES     0x00100000 // 1048576 - enable poison throwing knives
#define SBW_NOADRENALINE   0x00200000 // 2097152 - disable adrenaline

// g_weaponsenable defines
#define WPEN_MOLOTOV       0x0001 //    1 - Enable Molotov-Cocktails
#define WPEN_POISONGAS     0x0002 //    2 - Enable poison gas canisters
#define WPEN_POISONMINE    0x0004 //    4 - Enable poison gas landmines
#define WPEN_TRIPMINE      0x0008 //    8 - Enable tripmines
#define WPEN_M97           0x0010 //   16 - enable Winchester M97 (shotgun)
#define WPEN_THKNIVES      0x0020 //   32 - enable throwing knives
#define WPEN_PPSH          0x0040 //   64 - enable PPSH

// Class carryovers
#define SBS_COPS		1
#define SBS_ENGI		2
#define SBS_MEDI		4
#define SBS_FOPS		8

// Skill-5 benefit bit-flags.
#define SK5_BAT_SPRINT      0x0001 // faster stamina recharge

#define SK5_LWP_RECOIL      0x0001 // reduced recoil time

#define SK5_CVO_CHARGE      0x0001 // consume less charge
#define SK5_CVO_GRENADES    0x0002 // spawn/capacity 4 grenades
#define SK5_CVO_POISON      0x0004 // enable poison gas

#define SK5_ENG_CHARGE      0x0001 // consume less charge
#define SK5_ENG_GRENADES    0x0002 // spawn/capacity 10 grenades
#define SK5_ENG_MINE_SPOT   0x0004 // landmines take longer to spot
#define SK5_ENG_MINE_DEFUSE 0x0008 // landmines take longer to defuse
#define SK5_ENG_CONSTRUCT   0x0010 // build things faster
#define SK5_ENG_LM_BBETTY	0x0020 // enable bouncing-betty landmines
#define SK5_ENG_LM_PGAS     0x0040 // enable poison-gas landmines

#define SK5_FDO_CHARGE      0x0001 // consume less charge
#define SK5_FDO_GRENADES    0x0002 // spawn/capacity 2 grenades

#define SK5_MED_CHARGE      0x0001 // consume less charge
#define SK5_MED_GRENADES    0x0002 // spawn/capacity 4 grenades
#define SK5_MED_CARRY_CVO   0x0010 // carry-over health recharge to cvops
#define SK5_MED_CARRY_ENG   0x0020 // carry-over health recharge to eng
#define SK5_MED_CARRY_FDO   0x0040 // carry-over health recharge to fdops
#define SK5_MED_CARRY_SOL   0x0080 // carry-over health recharge to soldier

#define SK5_SOL_CHARGE      0x0001 // consume less charge 
#define SK5_SOL_GRENADES    0x0002 // spawn/capacity 8 grenades
#define SK5_SOL_POISON      0x0004 // enable poison gas

// Skill-5 global constants.
#define SK5G_SPRINT_FACTOR  2.00f // should be > level-2's 1.60f
#define SK5G_CHARGE_FACTOR  0.75f // 75% charge penalty of level-4
#define SK5G_MEDCARRY_TIMER 3500  // milliseconds to add 1 health
#define SK5G_RECOIL_FACTOR  0.85f // applied to existing recoil times (85%)

// bg_covertops
#define COPS_KEEPDISGUISE     1
#define COPS_MEDKIT           2
#define COPS_AMMOPACK         4
#define COPS_MINES            8
#define COPS_LIVEUNI          16
#define COPS_DRAWNAME         32

// g_misc defines
#define MISC_BINOCWAR         0x0001 //    1 - binoc-war - enables binocular pickup stats
#define MISC_ADMINSONLY       0x0002 //    2 - only admins can connect
#define MISC_PACKZ            0x0004 //    4 - throw packs vertically
#define MISC_BSREVIVE         0x0008 //    8 - level-4 battle-sense revivees get full health
#define MISC_REALAIMSPREAD    0x0010 //   16 - more realistic weapons aim-spread
#define MISC_NOXPINACTIVE     0x0020 //   32 - No XP for killing inactive players
#define MISC_NOXPMEDPACKS     0x0040 //   64 - No XP for med packs
#define MISC_NOXPAMMOPACKS    0x0080 //  128 - No XP for ammo packs
#define MISC_VISIBLEMINES     0x0100 //  256 - Visible enemy landmines red/blue instead of white
#define MISC_REALISTICLEAN    0x0200 //  512 - Realistic lean animation
#define MISC_NODROWREVIVE     0x0400 // 1024 - Drowned players can't be revived

// g_doubleJump values
#define DJUMP_DISABLED        0      // disabled
#define DJUMP_XMOD            1      // xmod style (850ms window)
#define DJUMP_NITMOD          2      // nitmod style (endless delay)
#define DJUMP_ETPUB           3      // etpub style (850ms window, same as xmod)

// userinfo JayFlags
//#define	JAYFLAGS_KILLSPREESOUNDS	1
//#define JAYFLAGS_PMSOUNDS				2
#define	JAYFLAGS_PMBLOCK				4

// Antilag/Prediction debug
#define DEBUGDELAG_ANTILAG              1
#define DEBUGDELAG_PREDICTION           2

// PERS_JAYFLAGS/effectXTime flags
#define JF_LOSTPANTS					1

// poison
#define POISON_ON						1 // Jaybird - not used really heh
#define POISON_NODISORIENT				2

// poison tracking
#define MAX_POISONEVENTS				10
#define POISONINTERVAL					1500
#define POISONDAMAGE					10
typedef struct poison_s {
	int poisoner;
	int fireTime;
} poison_t;

// Shotgun
// Animation time = 1000 / fps * numFrames
#define M97_RLT_RELOAD1			400		// Reload normal shell start
#define	M97_RLT_RELOAD2			900		// Reload normal shell loop
#define	M97_RLT_RELOAD2_QUICK	600		// Reload normal shell loop FAST
#define	M97_RLT_RELOAD3			550		// Reload normal shell end
#define	M97_RLT_ALTSWITCHFROM	2000	// Reload first shell and pump start
#define	M97_RLT_ALTSWITCHTO		300		// Reload first shell and pump to loop
#define	M97_RLT_DROP2			375		// Reload first shell and pump end

// Shotgun reload states
typedef enum {
	M97_READY,							// Not reloading
	M97_RELOADING_BEGIN,				// Reload normal shell start
	M97_RELOADING_BEGIN_PUMP,			// Reload first shell and pump start
	M97_RELOADING_AFTER_PUMP,			// Reload first shell and pump to loop
	M97_RELOADING_LOOP,					// Reload normal shell loop
} m97state_t;

typedef enum {
	HOLDABLE_HIT,   // hit counter: 16-bits @ 4-bits each { HEAD, HAND, TORSO, FOOT }
	HOLDABLE_HITF,  // hit counter (friendly): 16-bits @ 4-bits each { HEAD, HAND, TORSO, FOOT }
	HOLDABLE_M97,
} holdable_t;

// Jaybird
#define MAX_FIRETEAM_MEMBERS 9

#define MAX_CPU_BUFFER  32

// Functions
void BG_cpuUpdate      ( );
void PM_BeginM97Reload ( );
void PM_M97Reload      ( );
bool BG_IsPercent      ( const char* );

#if defined( CGAMEDLL ) || defined( GAMEDLL )
extern bool ammoTableNeedsUpdate;
void BG_updateAmmoTable();
#endif

// Killing Spree
typedef enum {
	KS_KILLINGSPREE,
	KS_RAMPAGE,
	KS_DOMINATING,
	KS_UNSTOPPABLE,
	KS_GODLIKE,
	KS_WICKEDSICK,
	KS_NUMLEVELS
} ks_t;

typedef enum {
	MK_DOUBLEKILL,
	MK_MULTIKILL,
	MK_MEGAKILL,
	MK_ULTRAKILL,
	MK_MONSTERKILL,
	MK_LUDICROUSKILL,
	MK_HOLYSHIT,
	MK_NUMLEVELS
} mk_t;

///////////////////////////////////////////////////////////////////////////////

typedef enum {
    BVF_NONE    = 0x00000000,
    BVF_ENABLED = 0x00000001,
} bulletVolumeFlags_t;

typedef enum {
    HVF_NONE    = 0x00000000,
    HVF_ENABLED = 0x00000001,
    HVF_HIT     = 0x00000002,
    HVF_DRAW    = 0x00000004,
} hitVolumeFlags_t;

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_XMOD_H
