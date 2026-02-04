#include <bgame/impl.h>

/************************************************************
*                                                           *
*   File: g_killingspree.c                                  *
*   Description: Functions that facilitate killing sprees   *
*   Author: Jaybird                                         *
*                                                           *
************************************************************/

/*
================================
Notes on the new 'ksmsg' command
--------------------------------
Usage: ksmsg "player name" "kills" "message"

This sends the command to client, and displays
the pulsing killing spree counter.  If it is
a losing spree, kills must be a negative number.
================================
*/

// Jaybird - Custom killing sprees
int		killspreeLevels[6] = { 5,	10,	15,	20,	25,	30 };
int		losespreeLevels[6] = { 10,	20,	30,	0,	0,	0 };

/*
===============
G_SendKillSpree
===============
*/
static void G_SendKillSpree( gentity_t *client, ks_t ksLevel, int kills ) {
	gentity_t* tent = G_TempEntity( vec3_origin, EV_KILLSPREE );
	tent->s.density = ksLevel;
	tent->s.eventParms[0] = client - g_entities;
	tent->s.eventParms[1] = kills;
	tent->r.svFlags = SVF_BROADCAST;
}

/*
===============
G_SendLoseSpree
===============
*/
static void G_SendLoseSpree( gentity_t *client, ks_t ksLevel, int kills ) {
	gentity_t* tent = G_TempEntity( vec3_origin, EV_LOSESPREE );
	tent->s.density = ksLevel;
	tent->s.eventParms[0] = client - g_entities;
	tent->s.eventParms[1] = kills;
	tent->r.svFlags = SVF_BROADCAST;
}

/*
=====================
G_InitKillSpreeLevels
Called on server load, so you cannot
change this mid-map
=====================
*/
void G_InitKillSpreeLevels() {
	int i, temp, scanned;
	int levels[6];
	qboolean valid = qtrue;

	if( *g_killSpreeLevels.string ) {
		scanned = sscanf(g_killSpreeLevels.string, "%i %i %i %i %i %i",
			&levels[0],
			&levels[1],
			&levels[2],
			&levels[3],
			&levels[4],
			&levels[5]
		);

		if( scanned == 6 ) {
			temp = 0;
			for( i = 0; i < 6; i++ ) {
				if( levels[i] < temp )
					valid = qfalse;
			}
			if( valid ) {
				for( i = 0; i < 6; i++ ) {
					killspreeLevels[i] = levels[i];
				}
			}
		}
	}

	if( *g_loseSpreeLevels.string ) {
		scanned = sscanf(g_loseSpreeLevels.string, "%i %i %i %i %i %i",
			&levels[0],
			&levels[1],
			&levels[2],
			&levels[3],
			&levels[4],
			&levels[5]
		);

		if( scanned == 6 ) {
			temp = 0;
			for( i = 0; i < 6; i++ ) {
				if( levels[i] < temp )
					valid = qfalse;
			}
			if( valid ) {
				for( i = 0; i < 6; i++ ) {
					losespreeLevels[i] = levels[i];
				}
			}
		}
	}
}

/*
============
G_FirstBlood
============
*/
void G_FirstBlood( gentity_t *ent ) {
	gentity_t *tent;

	if ( *level.firstBloodPlayer )
		return;

	Q_strncpyz(level.firstBloodPlayer,ent->client->pers.netname, sizeof(level.firstBloodPlayer));

	if( g_killingSpree.integer ) {
		AP(va("cp \"%s ^fhad ^ffirst ^fblood!\n\" 2", ent->client->pers.netname));

		// Send sound
		tent = G_TempEntity( vec3_origin, EV_FIRSTBLOOD );
		tent->r.svFlags = SVF_BROADCAST;
	}
}

/*
================
G_UpdateLastKill
================
*/
void G_UpdateLastKill(gentity_t *ent) {
	Q_strncpyz(level.lastBloodPlayer,ent->client->pers.netname, sizeof(level.lastBloodPlayer));
}

/*
=================
G_DisplayLastKill
=================
*/
void G_DisplayLastKill() {
	// Enabled?
	if( !g_killingSpree.integer )
		return;

	// Exists?
	if( !*level.lastBloodPlayer )
		return;

	AP( va( "chat \"^3*** ^fLast kill of the match goes to ^7%s^f!\"\n", level.lastBloodPlayer ));
}

/*
==============
G_AddLoseSpree
==============
*/
void G_AddLoseSpree(gentity_t *ent) {
	int kills, i;

	if(!( ent && ent->client ))
		return;

	kills = ++ent->client->pers.losespreekills;

	// Check exit now, so that the spree still gets recorded
	if( !g_killingSpree.integer )
		return;

	// See if an announcement needs to be made
	for (i = 0; i < KS_NUMLEVELS; i++) {
		if (kills == losespreeLevels[i]) {
			G_SendLoseSpree(ent, (ks_t)i, kills);
		}
	}
}

/*
==============
G_EndLoseSpree
==============
*/
void G_EndLoseSpree(gentity_t *self) {

	// Not a player
	if (!self->client)
		return;

	// Reset the counter
	self->client->pers.losespreekills = 0;
}

/*
==============
G_AddKillSpree
==============
*/
void G_AddKillSpree(gentity_t *ent) {
	int kills, i;

	// Not a player
	if (!ent || !ent->client)
		return;

	// Increment kills
	kills = ++ent->client->pers.killspreekills;

	// Also update the map's longest spree
	if( kills > level.longestSpree ) {
		level.longestSpree = kills;
        Q_strncpyz(level.longestSpreeName, ent->client->pers.netname, sizeof(level.longestSpreeName));
	}

	// Not in use
	if( !g_killingSpree.integer )
		return;

	// See if an announcement needs to be made
	for (i = 0; i < KS_NUMLEVELS; i++) {
		if (kills == killspreeLevels[i]) {
			G_SendKillSpree(ent, (ks_t)i, kills);
		}
	}
}

void G_LSFinalizeMap( void ) {
	int        i;
	gentity_t* ent;

	if(!( g_killingSpree.integer & KS_RECORD ))
		return;

	// Find the longest spree for this round
	for( i = 0; i < level.numConnectedClients; i++ ) {
		ent = g_entities + level.sortedClients[i];
		if( ent->client->pers.killspreekills > level.longestSpree ) {
			level.longestSpree = ent->client->pers.killspreekills;
			Q_strncpyz(level.longestSpreeName, ent->client->pers.netname, sizeof(level.longestSpreeName));
		}
	}

	// Found a record, if it's not new, just print it and return
	if( currentMap->longestSpree > level.longestSpree ) {
		char timestr[32];
		struct tm *lt = localtime( &currentMap->longestSpreeTime );
		strftime( timestr, sizeof( timestr ), "%c", lt );

		AP("chat \"^3*** ^fThe longest killing spree on this map was:\"");
		AP(va("chat \"^3*** ^x%i ^fkills by ^7%s ^fon ^x%s\"", currentMap->longestSpree, currentMap->longestSpreeNamex.c_str(), timestr ));
		return;
	}

	if( !level.longestSpree ) {
		// Disregard this if there are no kills.
		return;
	}

	// New record
	currentMap->longestSpree = level.longestSpree;
	currentMap->longestSpreeTime = time(NULL);
	currentMap->longestSpreeNamex = level.longestSpreeName;
    char tmpName[MAX_NETNAME];
    SanitizeString(level.longestSpreeName, tmpName, qtrue);
    currentMap->longestSpreeName = tmpName;

	AP("chat \"^3*** ^fThere is a new killing spree record for this map!\"");
	AP(va("chat \"^3*** ^fCongratulations to ^7%s ^fwith a ^x%i ^fspree on this map!\"", currentMap->longestSpreeNamex.c_str(), currentMap->longestSpree ));
}

void G_AddMultiKill( gentity_t *ent ) {
	gentity_t* tent;

	if (!g_killingSpree.integer)
		return;

	if (ent->client->pers.multikilltime < level.time - 3000) {
		ent->client->pers.multikills = 0;
	}

	ent->client->pers.multikills++;
	ent->client->pers.multikilltime = level.time;

	if (ent->client->pers.multikills > MK_NUMLEVELS + 1 || ent->client->pers.multikills < 2)
		return;

	tent = G_TempEntity( vec3_origin, EV_MULTIKILL );
	tent->r.singleClient = ent - g_entities;
	tent->s.density = ent->client->pers.multikills - 2;
	tent->s.eventParm = ent - g_entities;
	tent->r.svFlags = SVF_SINGLECLIENT | SVF_BROADCAST;
}

/*
====================
G_ResetDamageTracking
Reset damage tracking for a player (called on spawn)
====================
*/
void G_ResetDamageTracking( gentity_t *ent ) {
	if (!ent || !ent->client)
		return;

	memset(ent->client->damageReceivedFrom, 0, sizeof(ent->client->damageReceivedFrom));
}

/*
====================
G_ProcessKillAssistance
Process kill assistances when a player dies
Awards XP and sends notification to assistants
====================
*/
void G_ProcessKillAssistance( gentity_t *victim, gentity_t *killer, int meansOfDeath ) {
	int i;
	int killerSlot = -1;
	qboolean isSuicide = qfalse;
	qboolean isTeamKill = qfalse;

	if (!victim || !victim->client)
		return;

	// Check if kill assistance is enabled
	if (!g_killAssistances.integer)
		goto cleanup;

	// Not during warmup
	if (cvars::gameState.ivalue != GS_PLAYING)
		goto cleanup;

	// Determine kill type
	if (killer && killer->client) {
		killerSlot = killer->s.number;
		if (killer == victim) {
			isSuicide = qtrue;
		} else if (OnSameTeam(victim, killer)) {
			isTeamKill = qtrue;
		}
	}

	// Check if the appropriate bitflag is set
	if (isSuicide && !(g_killAssistances.integer & KILLASSIST_SUICIDE))
		goto cleanup;
	if (isTeamKill && !(g_killAssistances.integer & KILLASSIST_TEAMKILL))
		goto cleanup;
	if (!isSuicide && !isTeamKill && !(g_killAssistances.integer & KILLASSIST_ENABLE))
		goto cleanup;

	// Process each client's damage contribution
	for (i = 0; i < MAX_CLIENTS; i++) {
		int damage = victim->client->damageReceivedFrom[i];
		gentity_t *assistant;
		float xpAward;

		// Skip if no damage
		if (damage <= 0)
			continue;

		// Skip if this is the killer
		if (i == killerSlot)
			continue;

		// Get assistant entity
		assistant = &g_entities[i];
		if (!assistant->client || !assistant->inuse)
			continue;

		// For normal kills (not teamkill, not suicide), only award assists to enemies
		// Skip if assistant is on the same team as the victim
		if (!isTeamKill && !isSuicide && OnSameTeam(victim, assistant))
			continue;

		// Calculate XP award: +1 for <50 damage, +2 for >=50 damage
		if (damage < 50) {
			xpAward = 1.0f;
		} else {
			xpAward = 2.0f;
		}

		// For teamkill assistances, XP is lost instead of gained
		if (isTeamKill) {
			// Only award negative XP to assistants on the same team as killer
			if (killer && killer->client && OnSameTeam(assistant, killer)) {
				xpAward = -xpAward;
			} else {
				continue;  // Skip non-teamkill assistants
			}
		}

		// Award XP (use battle sense skill)
		G_AddSkillPoints(assistant, SK_BATTLE_SENSE, xpAward);

		// Send notification to the assistant
		if (xpAward > 0) {
			trap_SendServerCommand(i, va("cp \"^2Kill Assist! ^7(+%.0fXP)\" 1", xpAward));
		} else {
			trap_SendServerCommand(i, va("cp \"^1TK Assist! ^7(%.0fXP)\" 1", xpAward));
		}

		G_LogPrintf("KillAssist: %i assisted killing %i (damage: %i, xp: %.0f)\n", 
			i, victim->s.number, damage, xpAward);
	}

cleanup:
	// Reset damage tracking
	G_ResetDamageTracking(victim);
}

/*
====================
G_ProcessRevive
Process revive for multi-revive tracking and announcements
====================
*/
void G_ProcessRevive( gentity_t *medic, gentity_t *patient ) {
	int multiReviveLevel;
	char *soundPath = NULL;

	if (!medic || !medic->client || !patient || !patient->client)
		return;

	// Check if revive spree options are enabled
	if (!(g_reviveSpreeOptions.integer & REVIVESPREE_ENABLE))
		return;

	// Check for multi-revive timing
	if (level.time - medic->client->lastReviveTime <= g_multiReviveTime.integer) {
		medic->client->multiReviveCount++;
	} else {
		// Announce spree end if enabled and count was high (2+ revives in quick succession)
		if ((g_reviveSpreeOptions.integer & REVIVESPREE_ANNOUNCE_END) && medic->client->multiReviveCount >= 1) {
			// Display the actual count (multiReviveCount represents consecutive revives after the first)
			trap_SendServerCommand(medic - g_entities, va("cp \"Revive spree ended! (%d revives)\" 2", medic->client->multiReviveCount + 1));
		}
		medic->client->multiReviveCount = 0;
	}

	medic->client->lastReviveTime = level.time;

	// multiReviveCount is the number of consecutive revives after the first one
	// So 1 = double revive, 2 = triple revive, etc.
	multiReviveLevel = medic->client->multiReviveCount;

	// Cap at 4 levels (mr1.wav through mr4.wav)
	if (multiReviveLevel > 4)
		multiReviveLevel = 4;

	// Announce revives if enabled (1 = double, 2 = triple, 3 = multi, 4+ = monster)
	if ((g_reviveSpreeOptions.integer & REVIVESPREE_ANNOUNCE) && multiReviveLevel >= 1) {
		const char *multiNames[] = { "DOUBLE", "TRIPLE", "MULTI", "MONSTER" };
		int nameIndex = multiReviveLevel - 1;
		if (nameIndex > 3) nameIndex = 3;
		trap_SendServerCommand(-1, va("cp \"%s ^2%s REVIVE!\" 1", 
			medic->client->pers.netname, multiNames[nameIndex]));
	}

	// Play sound - paths: sound/multirevive/mr1.wav through mr4.wav
	// These sounds are included in the xmod pk3
	if (multiReviveLevel >= 1) {
		switch (multiReviveLevel) {
			case 1: soundPath = "sound/multirevive/mr1.wav"; break;
			case 2: soundPath = "sound/multirevive/mr2.wav"; break;
			case 3: soundPath = "sound/multirevive/mr3.wav"; break;
			default: soundPath = "sound/multirevive/mr4.wav"; break;
		}

		// Send sound event to medic's team
		G_ClientSound(medic, soundPath);
	}
}
