#include <bgame/impl.h>
#include <game/xmod_database.h>
#include <game/xmod_globals.h>
#include <game/g_weaponscripts.h>

/*
====================
G_TeamMaxLandMines
====================
*/
int G_TeamMaxLandMines() {
	if (cvars::g_maxLandmines.ivalue < 0)
		return 10;
	return cvars::g_maxLandmines.ivalue;
}

/*
====================
G_TeamMaxTripmines
====================
*/
int G_TeamMaxTripmines() {
	if (cvars::g_maxTripmines.ivalue < 0)
		return 3;
	return cvars::g_maxTripmines.ivalue;
}

/*
====================
G_TeamMaxArtillery
====================
*/
int G_TeamMaxArtillery() {
	if (team_maxArtillery.integer < 0 || team_maxArtillery.integer > 6)
		return 6;
	return team_maxArtillery.integer;
}

/*
=============================
CG_SortBinocs
=============================
*/
int QDECL G_SortBinocs( const void *a, const void *b ) {
	int A = *((int*)a);
	int B = *((int*)b);

	gentity_t *entA = &g_entities[A];
	gentity_t *entB = &g_entities[B];

	if( entA->client->pers.binocCount < entB->client->pers.binocCount ) {
		return 1;
	}
	else {
		return -1;
	}
}

/*
====================
Binoc War
Printing function
====================
*/
#define MAX_BINOC_STATS 3
void G_BinocWar( qboolean endOfMatch ) {
	int i;
	gentity_t *ent;
	char str[MAX_STRING_CHARS];
	int sortedClients[MAX_CLIENTS];
	int numDisplay = min( MAX_BINOC_STATS, level.numConnectedClients );

	// Don't run if not turned on.
	if(!( cvars::bg_misc.ivalue & MISC_BINOCWAR ))
		return;

	// Do not run regular stats if in intermission
	if( !endOfMatch && cvars::gameState.ivalue == GS_INTERMISSION )
		return;

	// Don't run if no one's on.
	if ( !level.numConnectedClients )
		return;

	// Don't display if not time.
	if( level.time - level.binocWarTime < 60 * 1000 && !endOfMatch ) {
		return;
	}

	// Time to display!
	level.binocWarTime = level.time;

	// Populate the array
	for( i = 0; i < level.numConnectedClients; i++ ) {
		sortedClients[i] = level.sortedClients[i];
	}

	// Sort the players by count
	qsort( sortedClients, level.numConnectedClients, sizeof( int ), G_SortBinocs );

	// Check to make sure stats exist
	ent = g_entities + sortedClients[0];
	if( !ent || !ent->client || !ent->client->pers.binocCount )
		return;

	*str = 0;
	Q_strncpyz( str, "^3*** ", sizeof( str ));
	if( endOfMatch )
		Q_strcat( str, sizeof( str ), "^xFinal " );
	Q_strcat( str, sizeof(str), "^xBinoc War Stats:" );

	AP( va( "chat \"%s\"\n", str ));

	// Get the top 3
	for( i = 0; i < numDisplay; i++ ) {
		ent = g_entities + sortedClients[i];
		if( !ent->client->pers.binocCount )
			break;
		AP( va( "chat \"^3*** ^f%2i. ^7%s^f (^x%i^f)\"\n", i + 1, ent->client->pers.netname, ent->client->pers.binocCount ));
	}
}

/*
====================
G_InitCustomLevels
Called on server load, so you cannot
change this mid-map
====================
*/
void G_InitCustomLevels() {
	int i,j,scanned,temp;
	char *str;
	char info[MAX_INFO_STRING];

	info[0] = 0;
	for (i = 0; i < SK_NUM_SKILLS; i++) {
		switch(i) {
			case SK_LIGHT_WEAPONS:
				str = va("%s", g_levels_lightweapons.string);
				break;
			case SK_EXPLOSIVES_AND_CONSTRUCTION:
				str = va("%s", g_levels_engineer.string);
				break;
			case SK_BATTLE_SENSE:
				str = va("%s", g_levels_battlesense.string);
				break;
			case SK_FIRST_AID:
				str = va("%s", g_levels_medic.string);
				break;
			case SK_SIGNALS:
				str = va("%s", g_levels_fieldops.string);
				break;
			case SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS:
				str = va("%s", g_levels_covertops.string);
				break;
			default:
			case SK_HEAVY_WEAPONS:
				str = va("%s", g_levels_soldier.string);
				break;
		}

		// Set up for failure.
		// That way, if a value is omitted, it defaults to disabled.
		skillLevels[i][1] = -1;
		skillLevels[i][2] = -1;
		skillLevels[i][3] = -1;
		skillLevels[i][4] = -1;
		skillLevels[i][5] = -1;

		// Grab what they gave us...
		scanned = sscanf(str, "%i %i %i %i %i",
			&skillLevels[i][1],
			&skillLevels[i][2],
			&skillLevels[i][3],
			&skillLevels[i][4],
			&skillLevels[i][5] );

		// ... and error check it.
		if (!*str || scanned == 0) {
			skillLevels[i][1] = 20;
			skillLevels[i][2] = 50;
			skillLevels[i][3] = 90;
			skillLevels[i][4] = 140;
			skillLevels[i][5] = 200;
		}

		temp = 0;
		// The point here is to set it up so that it conforms to this:
		// -1 means skill is disabled.  If < 0, just set to -1.
		// skill[i-1] < skill[i], unless skill[i] is -1, or set defaults
        for (j = 1; j < NUM_SKILL_LEVELS; j++) {
			if (skillLevels[i][j] < 0)
				skillLevels[i][j] = -1;
			else if (skillLevels[i][j] < temp) {
				skillLevels[i][1] = 20;
				skillLevels[i][2] = 50;
				skillLevels[i][3] = 90;
				skillLevels[i][4] = 140;
				skillLevels[i][5] = 200;
				break;
			}
		}

		// Now construct the config string
		Info_SetValueForKey(info, va("skill_%i",i),va("%i %i %i %i %i",
			skillLevels[i][1],
			skillLevels[i][2],
			skillLevels[i][3],
			skillLevels[i][4],
			skillLevels[i][5] ));
	}
	trap_SendServerCommand( -1, va("xcs s \"%s\"", info) );
}

/*
====================
G_ApplyCustomRanks
Jaybird
Use custom ranks instead of default 20 50 90 140 200
====================
*/
void G_ApplyCustomRanks( gclient_t *client ) {
	int levels[SK_NUM_SKILLS];
	int numScanned = 0;
	int i;

	if (!g_defaultSkills.string[0])
		return;

	numScanned = sscanf(g_defaultSkills.string, "%i %i %i %i %i %i %i",
		&levels[0],
		&levels[1],
		&levels[2],
		&levels[3],
		&levels[4],
		&levels[5],
		&levels[6]);

	if (numScanned != SK_NUM_SKILLS)
		return;

	// Properly scanned string.
	// Now find out what the values for that specific level are,
	// and compare them to the existing (assuming it does) skills
	// of the individual player.
	for (i = 0; i < SK_NUM_SKILLS; i++) {
		if (levels[i] < 0)
			levels[i] = 0;
		else if (levels[i] > NUM_SKILL_LEVELS - 1)
			levels[i] = NUM_SKILL_LEVELS - 1;

		if (skillLevels[i][levels[i]] > 0 &&
			client->sess.skillpoints[i] < skillLevels[i][levels[i]]) {
			client->sess.skillpoints[i] = skillLevels[i][levels[i]];
		}
	}
	G_CalcRank(client);
}

/*
====================
G_Banners
Banner support
====================
*/
void G_Banners(void)
{
	char banner[BANNER_MAXLEN]; // use multiple banners if you want it longer!
	char *bannercmd;

	// No banners
	if( !g_banners.integer )
		return;

	// spam bad, mmmkaaay!
    if( g_bannerTime.integer < BANNER_MINTIME ) {
        trap_Cvar_Set("g_bannerTime", va("%d", BANNER_MINTIME));
    }

	// check if it is time to display a banner
	if( level.time - level.bannerTime < g_bannerTime.integer * 1000 ) {
		return;
	}

	// we are displaying, reset time
	level.bannerTime = level.time;
   
	// get banner value
	memset( banner, 0, sizeof( banner ));
	trap_Cvar_VariableStringBuffer( va( "g_banner%d", level.iBanner ), banner, sizeof( banner ));

	// move on to the next one
	level.iBanner = ( level.iBanner < g_banners.integer ) ? level.iBanner + 1 : 1;

	// empty banner... we are done
	if( !*banner )
		return;
   
	// set up banner destination
	switch( g_bannerLocation.integer ) {
	case 1:
		bannercmd = "cpm \"%s\"";
		break;
	case 2:
		bannercmd = "cp \"%s\"";
		break;
	case 3:
		bannercmd = "print \"%s\n\"";
		break;
	case 4:
		bannercmd = "bp \"%s\"";
		break;
	default:
		bannercmd = "chat \"%s\"";
	}

	// do it
	AP( va( bannercmd, banner ));
} 

/*
====================
G_PrivateMessage
====================
*/
void G_PrivateMessage( gentity_t *ent ) 
{
	// Just for good measure
	if (!ent || !ent->client)
		return;

	// No private messages
	if (!g_privateMessages.integer)
		return;

	int clientIndex = ent - g_entities;
    
    // Get sender info using session helpers
    int senderLevel = ::xmod::getClientLevel(clientIndex);
    const std::string& senderNamex = ::xmod::getClientNamex(clientIndex);
    const std::string& senderName = ::xmod::getClientName(clientIndex);
    Client& actor = g_clientObjects[clientIndex];

	// Disallow when muted - use session helper
	if (::xmod::isClientMuted(clientIndex))
		return;

	// Get the arguments (this part sucks)
    vector<string> args;
    Engine::args( args );

    // Chop off first arg if say
    string s = args[0];
    str::toLower( s );
    if (s.find( "say" ) != string::npos)
        args.erase( args.begin() );

    using namespace text;
    Buffer ebuf;
    ebuf << xfail( args[0] + " error: " );

    // At this point, we can enforce usage
	if (args.size() < 3) {
		ebuf << ' ' << xvalue( "PLAYER_FILTER" ) << ' ' << xvalue( "MESSAGE..." );
        cmd::printChat( &actor, ebuf );
		return;
	}

    // Find matching clients
    vector<Client*> clients;
    if (cmd::matchClients( args[1], clients, ebuf )) {
        cmd::printChat( &actor, ebuf );
        return;
    }

    // Build subscriber list
    set<int> subscribers;

    const size_t cmax = clients.size();
    for ( size_t i = 0; i < cmax; i++ ) {
        Client& c = *clients[i];

        // skip if bot
        if (c.gentity.r.svFlags & SVF_BOT)
            continue;

        // skip if self
        if (actor.slot == c.slot)
            continue;

        subscribers.insert( c.slot );
    }

    // bail if no subscribers
    const size_t nsubs = subscribers.size();
    if (!nsubs)
        return;

    // Build admin list
    set<int> admins;

    for (int i = 0; i < level.numConnectedClients; i++) {
        Client& c = g_clientObjects[level.sortedClients[i]];

        // skip if bot
        if (c.gentity.r.svFlags & SVF_BOT)
            continue;

        // skip if self
        if (actor.slot == c.slot)
            continue;

        // skip if on team
        const int team = c.gclient.sess.sessionTeam;
        if (team == TEAM_AXIS || team == TEAM_ALLIES)
            continue;

        // skip if already in subscriber list
        if (subscribers.find( c.slot ) != subscribers.end())
            continue;

        // skip if no privilege
        if (!::xmod::hasClientPrivilege(c.slot, priv::base::specChat))
            continue;

        admins.insert( c.slot );
    }

	// Message
    string message;
    str::concatArgs( args, message, 2 );

    // Prepare our blind carbon copy
    Buffer bcc;

	// Send to subscribers
    {
        int pmcount = 0;
        const set<int>::iterator max = subscribers.end();
        for ( set<int>::iterator it = subscribers.begin(); it != max; it++ ) {
            Client& c = g_clientObjects[*it];
            int recipientLevel = ::xmod::getClientLevel(*it);
            const std::string& recipientNamex = ::xmod::getClientNamex(*it);

            // skip if pm-blocked and actor is not higher level
            if (c.gclient.pers.pmblock && senderLevel <= recipientLevel) {
                ebuf << xvalue( recipientNamex ) << " is blocking private messages.";
                cmd::printChat( &actor, ebuf );
                continue;
            }

		    // Send message
            {
                Buffer buf;
                buf << xvalue( senderNamex ) << " -> " << xvalue( recipientNamex ) << " (" << xvalue( nsubs ) << "): "
                    << xcbold << message;
                cmd::printPm( &c, buf, true );

    		    CPx( c.slot, va( "cp \"^3Private message from ^7%s^3.\"", senderNamex.c_str() ));
            }

            // bcc: self
            {
                Buffer buf;
                if (pmcount++)
                    bcc << '\n';
                bcc << "PM -> " << xvalue( recipientNamex ) << ": " << message;
            }
        }

        cmd::printChat( &actor, bcc );
	}

	// Send to  admins
    {
        const set<int>::iterator max = admins.end();
        for ( set<int>::iterator it = admins.begin(); it != max; it++ ) {
            Client& c = g_clientObjects[*it];

            Buffer buf;
            buf << xvalue( senderNamex ) << " -> " << xvalue( args[1] ) << " (" << xvalue( nsubs ) << "): "
                << xcbold << message;
            cmd::printPm( &c, buf, false );
        }
    }

    // Also log the chat
	G_LogPrintf( "pm: %s -> %s: %s\n", senderName.c_str(), args[1].c_str(), message.c_str() );
}

/*
====================
G_AdminChat

Admin-only chat command (/ma)
Sends a message to all admins with the adminChat privilege
====================
*/
void G_AdminChat( gentity_t *ent ) 
{
	// Just for good measure
	if (!ent || !ent->client)
		return;

	// Admin chat disabled
	if (!g_adminChat.integer)
		return;

	int clientIndex = ent - g_entities;
    
    // Check if sender has admin chat privilege
    if (!::xmod::hasClientPrivilege(clientIndex, priv::base::adminChat)) {
        trap_SendServerCommand( clientIndex, "chat \"^1You do not have permission to use admin chat.\"" );
        return;
    }
    
    // Get sender info using session helpers
    const std::string& senderNamex = ::xmod::getClientNamex(clientIndex);
    const std::string& senderName = ::xmod::getClientName(clientIndex);
    Client& actor = g_clientObjects[clientIndex];

	// Disallow when muted - use session helper
	if (::xmod::isClientMuted(clientIndex))
		return;

	// Get the arguments
    vector<string> args;
    Engine::args( args );

    // Chop off first arg if say
    string s = args[0];
    str::toLower( s );
    if (s.find( "say" ) != string::npos)
        args.erase( args.begin() );

    using namespace text;
    Buffer ebuf;
    ebuf << xfail( args[0] + " error: " );

    // At this point, we can enforce usage
	if (args.size() < 2) {
		ebuf << ' ' << xvalue( "MESSAGE..." );
        cmd::printChat( &actor, ebuf );
		return;
	}

	// Message
    string message;
    str::concatArgs( args, message, 1 );

    // Build admin list
    set<int> admins;

    for (int i = 0; i < level.numConnectedClients; i++) {
        Client& c = g_clientObjects[level.sortedClients[i]];

        // skip if bot
        if (c.gentity.r.svFlags & SVF_BOT)
            continue;

        // skip if no privilege
        if (!::xmod::hasClientPrivilege(c.slot, priv::base::adminChat))
            continue;

        admins.insert( c.slot );
    }

    // bail if no admins
    if (admins.empty())
        return;

	// Send to all admins
    {
        const set<int>::iterator max = admins.end();
        for ( set<int>::iterator it = admins.begin(); it != max; it++ ) {
            Client& c = g_clientObjects[*it];

            Buffer buf;
            buf << "^3[AdminChat] " << xvalue( senderNamex ) << "^3: " << xcbold << message;
            cmd::printChat( &c, buf );
        }
    }

    // Also log the admin chat
	G_LogPrintf( "adminchat: %s: %s\n", senderName.c_str(), message.c_str() );
}

/*
====================
G_PlayDead
====================
*/
void G_PlayDead(gentity_t *ent) 
{
	if (!ent->client)
        return;

	if (!g_playDead.integer)
        return;

	if(G_IsPoisoned( ent ))
		return;

	ent->client->ps.pm_type = PM_PLAYDEAD;
}

/*
====================
G_DragCorpse
====================
*/
qboolean G_DragCorpse(gentity_t *dragger, gentity_t *corpse) 
{
	vec3_t	dir, pull, res;
	float dist;

	if (!g_dragCorpse.integer)
        return qfalse;

	VectorSubtract(dragger->r.currentOrigin, corpse->r.currentOrigin, dir);
	dir[2] = 0;
	dist = VectorNormalize(dir);

	// Don't pull corpses past the dragger's head and don't start dragging
	// Until both players look like they're in contact
	if (dist > 85 || dist < 40)
        return qfalse;

	VectorScale(dir, 150, pull);

	// Prevent some zipping around when the corpse doesn't have much friction
	VectorSubtract(pull, corpse->client->ps.velocity, res);

	// No vertical component
	res[2] = 0;

    // Move the player
	VectorAdd(corpse->s.pos.trDelta, res, corpse->s.pos.trDelta );
	VectorAdd(corpse->client->ps.velocity, res, corpse->client->ps.velocity);

	return qtrue;
 }

/*
====================
G_PushPlayer
Mostly lifted from WolfRevivePushEnt
====================
*/
qboolean G_PushPlayer(gentity_t *ent, gentity_t *victim) 
{
	vec3_t	dir, push;

    // Feature disabled
	if (!g_shove.integer)
		return qfalse;

    // No playdead or dead players
    if (ent->client->ps.eFlags & (EF_PLAYDEAD | EF_DEAD))
        return qfalse;

    // Pushed not too long ago
	if ((level.time - ent->client->pmext.shoveTime) < 500) {
		return qfalse;
	}

	ent->client->pmext.shoveTime = level.time;

	VectorSubtract(victim->r.currentOrigin, ent->r.currentOrigin, dir);

	VectorNormalizeFast(dir);

	VectorScale(dir, (g_shove.value * 5.0f), push);

	if ((fabs(push[2]) > fabs(push[0])) && (fabs(push[2]) > fabs(push[1]))) {
		// player is being boosted
		if(g_shoveNoZ.integer) {
			return qfalse;
		}
		push[2] = g_shove.value;
		push[0] = push[1] = 0;
	}
	else {
		// give them a little hop
		push[2] = 24;
	}

	VectorAdd(victim->s.pos.trDelta, push, victim->s.pos.trDelta);
	VectorAdd(victim->client->ps.velocity, push, victim->client->ps.velocity);

    victim->client->pmext.wasShoved = qtrue;
    victim->client->pmext.shover = ent - g_entities;

    G_AddEvent( victim, EV_GENERAL_SOUND, G_SoundIndex("sound/xmod/push.wav" ));

	return qtrue;
}

/*
====================
G_ClassStealFixWeapons()
A helper for handling weapons for G_ClassSteal
====================
*/
void G_ClassStealFixWeapons(gentity_t* stealer, gentity_t* deadguy) 
{
    // Class tools
	COM_BitClear(stealer->client->ps.weapons, WP_DYNAMITE);
	COM_BitClear(stealer->client->ps.weapons, WP_PLIERS);
	COM_BitClear(stealer->client->ps.weapons, WP_LANDMINE);
	COM_BitClear(stealer->client->ps.weapons, WP_LANDMINE_BBETTY);
	COM_BitClear(stealer->client->ps.weapons, WP_LANDMINE_PGAS);
	COM_BitClear(stealer->client->ps.weapons, WP_SMOKE_BOMB);
	COM_BitClear(stealer->client->ps.weapons, WP_SATCHEL);
	COM_BitClear(stealer->client->ps.weapons, WP_SATCHEL_DET);
	COM_BitClear(stealer->client->ps.weapons, WP_SMOKE_MARKER);
	COM_BitClear(stealer->client->ps.weapons, WP_AMMO);
	COM_BitClear(stealer->client->ps.weapons, WP_MEDKIT);
	COM_BitClear(stealer->client->ps.weapons, WP_MEDIC_SYRINGE);
	COM_BitClear(stealer->client->ps.weapons, WP_MEDIC_ADRENALINE);
	COM_BitClear(stealer->client->ps.weapons, WP_ADRENALINE_SHARE);
	COM_BitClear(stealer->client->ps.weapons, WP_POISON_SYRINGE);
	COM_BitClear(stealer->client->ps.weapons, WP_POISON_GAS);

	// Primary weapons
	COM_BitClear(stealer->client->ps.weapons, WP_FG42);
	COM_BitClear(stealer->client->ps.weapons, WP_FG42SCOPE);
	COM_BitClear(stealer->client->ps.weapons, WP_STEN);
	COM_BitClear(stealer->client->ps.weapons, WP_MP40);
	COM_BitClear(stealer->client->ps.weapons, WP_THOMPSON);
	COM_BitClear(stealer->client->ps.weapons, WP_M97);
	COM_BitClear(stealer->client->ps.weapons, WP_PPSH);
	
    // Heavy weapons
	COM_BitClear(stealer->client->ps.weapons, WP_PANZERFAUST);
	COM_BitClear(stealer->client->ps.weapons, WP_FLAMETHROWER);
	COM_BitClear(stealer->client->ps.weapons, WP_MOBILE_MG42);
	COM_BitClear(stealer->client->ps.weapons, WP_MOBILE_MG42_SET);
	COM_BitClear(stealer->client->ps.weapons, WP_MORTAR);
	COM_BitClear(stealer->client->ps.weapons, WP_MORTAR_SET);

    // Axis rifles
	COM_BitClear(stealer->client->ps.weapons, WP_KAR98);
	COM_BitClear(stealer->client->ps.weapons, WP_GPG40);
	COM_BitClear(stealer->client->ps.weapons, WP_K43);
	COM_BitClear(stealer->client->ps.weapons, WP_K43_SCOPE);

    // Allied rifles
	COM_BitClear(stealer->client->ps.weapons, WP_CARBINE);
	COM_BitClear(stealer->client->ps.weapons, WP_M7);
	COM_BitClear(stealer->client->ps.weapons, WP_GARAND);
	COM_BitClear(stealer->client->ps.weapons, WP_GARAND_SCOPE);

    // Silence pistols are different than regular pistols so swap them
	if (stealer->client->sess.playerType == PC_COVERTOPS) {
		stealer->client->pmext.silencedSideArm = 1;
		COM_BitSet(stealer->client->ps.weapons, WP_SILENCER);
		COM_BitSet(stealer->client->ps.weapons, WP_SILENCED_COLT);

        // Also check for akimbos
		if (COM_BitCheck(stealer->client->ps.weapons, WP_AKIMBO_LUGER)) {
			COM_BitClear(stealer->client->ps.weapons, WP_AKIMBO_LUGER);
			COM_BitSet(stealer->client->ps.weapons, WP_AKIMBO_SILENCEDLUGER);
		}
		else if (COM_BitCheck(stealer->client->ps.weapons, WP_AKIMBO_COLT)) {
			COM_BitClear(stealer->client->ps.weapons, WP_AKIMBO_COLT);
			COM_BitSet(stealer->client->ps.weapons, WP_AKIMBO_SILENCEDCOLT);
		}
	} else {
		stealer->client->pmext.silencedSideArm = 0;
		COM_BitClear(stealer->client->ps.weapons, WP_SILENCER);
		COM_BitClear(stealer->client->ps.weapons, WP_SILENCED_COLT);

        // Also check for akimbos
		if (COM_BitCheck(stealer->client->ps.weapons, WP_AKIMBO_SILENCEDLUGER)) {
			COM_BitClear(stealer->client->ps.weapons, WP_AKIMBO_SILENCEDLUGER);
			COM_BitSet(stealer->client->ps.weapons, WP_AKIMBO_LUGER);
		}
		else if (COM_BitCheck(stealer->client->ps.weapons, WP_AKIMBO_SILENCEDCOLT)) {
			COM_BitClear(stealer->client->ps.weapons, WP_AKIMBO_SILENCEDCOLT);
			COM_BitSet(stealer->client->ps.weapons, WP_AKIMBO_COLT);
 		}
 	}
 
	// Now give the primary weapon
	if (!G_IsWeaponDisabled(stealer, (weapon_t)BODY_WEAPON(deadguy), qtrue) && BG_CanUseWeapon(stealer->client->sess.playerType, stealer->client->sess.sessionTeam, (weapon_t)BODY_WEAPON(deadguy))) {
		COM_BitSet(stealer->client->ps.weapons, BODY_WEAPON(deadguy));
		stealer->client->ps.weapon = BODY_WEAPON(deadguy);
	}
	else if(BODY_TEAM(deadguy) == TEAM_AXIS) {
		COM_BitSet(stealer->client->ps.weapons, WP_MP40);
		stealer->client->ps.weapon = WP_MP40;
	}
	else {
		COM_BitSet(stealer->client->ps.weapons, WP_THOMPSON);
		stealer->client->ps.weapon = WP_THOMPSON;
	}

	// Give the weapon alt fires
	if (COM_BitCheck(stealer->client->ps.weapons, WP_CARBINE)) {
		COM_BitSet(stealer->client->ps.weapons, WP_M7);
	}
	else if (COM_BitCheck(stealer->client->ps.weapons, WP_KAR98)) {
		COM_BitSet(stealer->client->ps.weapons, WP_GPG40);
	}
    else if (COM_BitCheck(stealer->client->ps.weapons, WP_K43)) {
        COM_BitSet(stealer->client->ps.weapons, WP_K43_SCOPE);
    }
    else if (COM_BitCheck(stealer->client->ps.weapons, WP_GARAND)) {
        COM_BitSet(stealer->client->ps.weapons, WP_GARAND_SCOPE);
    }
    else if (COM_BitCheck(stealer->client->ps.weapons, WP_MORTAR)) {
        COM_BitSet(stealer->client->ps.weapons, WP_MORTAR_SET);
    }
    else if (COM_BitCheck(stealer->client->ps.weapons, WP_MOBILE_MG42)) {
        COM_BitSet(stealer->client->ps.weapons, WP_MOBILE_MG42_SET);
    }

    // In order to work right, the player must have ammo, so give them whatever the player had
    stealer->client->ps.ammo[stealer->client->ps.weapon] = BODY_AMMO(deadguy);

	// Give them all the tool-like weapons for the class
	G_AddClassSpecificTools(stealer->client);
}

/*
====================
G_ClassSteal
tjw
====================
*/
qboolean G_ClassSteal(gentity_t *stealer, gentity_t *deadguy) 
{
	if (!g_classChange.integer)
		return qfalse;
	if (BODY_CLASS(deadguy) == stealer->client->sess.playerType)
		return qfalse;
	if (deadguy->activator)
		return qfalse; // already used corpse
	if (BODY_TEAM(deadguy) >= 4)
		return qfalse;
	if (BODY_VALUE(deadguy) < 250) {
		if(BODY_VALUE(deadguy) == 0) {
			CPx(stealer->s.number, "cp \"Switching Classes\" 1");
		}
		BODY_VALUE(deadguy) += 5;
		return qtrue;
	}

	deadguy->nextthink = deadguy->timestamp + BODY_TIME(BODY_TEAM(deadguy));

	deadguy->activator = stealer;
	deadguy->s.time2 =	1;

	// sound effect
	G_AddEvent(stealer, EV_DISGUISE_SOUND, 0);

	stealer->client->sess.playerType = BODY_CLASS(deadguy);

	G_ClassStealFixWeapons(stealer, deadguy);

	// Jaybird - lose disguise unless specified to keep
	if (!(cvars::bg_covertops.ivalue & COPS_KEEPDISGUISE))
		stealer->client->ps.powerups[PW_OPS_DISGUISED] = 0;

	// I guess this disables future use of the corpse 
	BODY_TEAM(deadguy) += 4;

	ClientUserinfoChanged(stealer->s.clientNum);
    G_AddEvent( stealer, EV_WEAPONSWITCHED, 0 );

	return qtrue;
}

/*
==================
G_UniformSteal
==================
*/
qboolean G_UniformSteal(gentity_t *ent, gentity_t *traceEnt) {
	if (traceEnt->activator)
		return qfalse; //already used corpse
	if (ent->client->sess.playerType != PC_COVERTOPS)
		return qfalse;
	if(ent->health <= 0) 
		return qfalse;
	if (ent->client->ps.powerups[PW_OPS_DISGUISED])
		return qfalse;
	if (ent->client->ps.powerups[PW_BLUEFLAG] ||
		ent->client->ps.powerups[PW_REDFLAG])
		return qfalse;
	if (BODY_TEAM(traceEnt) >= 4)
		return qfalse;
	if (BODY_TEAM(traceEnt) == ent->client->sess.sessionTeam)
		return qfalse;
	if (BODY_VALUE(traceEnt) < 250 ) {
		BODY_VALUE(traceEnt) += 5;
		return qfalse;
	}

	traceEnt->nextthink = traceEnt->timestamp + BODY_TIME(BODY_TEAM(traceEnt));
			
	ent->client->ps.powerups[PW_OPS_DISGUISED] = 1;
	ent->client->ps.powerups[PW_OPS_CLASS_1] = BODY_CLASS(traceEnt) & 1;
	ent->client->ps.powerups[PW_OPS_CLASS_2] = BODY_CLASS(traceEnt) & 2;
	ent->client->ps.powerups[PW_OPS_CLASS_3] = BODY_CLASS(traceEnt) & 4;

	BODY_TEAM(traceEnt) += 4;
	traceEnt->activator = ent;

	traceEnt->s.time2 =	1;

	// sound effect
	G_AddEvent( ent, EV_DISGUISE_SOUND, 0 );

	G_AddSkillPoints( ent, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, 5.f );
	G_DebugAddSkillPoints( ent, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, 5, "stealing uniform" ); 

	Q_strncpyz( ent->client->disguiseNetname, g_entities[traceEnt->s.clientNum].client->pers.netname, sizeof(ent->client->disguiseNetname) );
	ent->client->disguiseRank = g_entities[traceEnt->s.clientNum].client ? g_entities[traceEnt->s.clientNum].client->sess.rank : 0;

	ClientUserinfoChanged( ent->s.clientNum );

	return qtrue;
}

/*
====================
G_ChatShortcuts
====================
*/
void G_ChatShortcuts(gentity_t *ent, string &buf) {

	if( !g_shortcuts.integer )
		return;

	const char* s = buf.c_str();
	stringstream result;

	while (*s) {
		if ( *s == '[') {
            // Last person who gave you ammo
			if (!Q_strncmp( s, "[a]", 3 ) || !Q_strncmp( s, "[A]", 3 )) {
                result << ent->client->pers.lastammo;
				s += 3;
				continue;
			}

            // Last person who killed you
			if (!Q_strncmp( s, "[d]", 3) || !Q_strncmp( s, "[D]", 3 )) {
                result << ent->client->pers.lastkilled;
				s += 3;
				continue;
			}

            // Last person who gave you health
			if (!Q_strncmp( s, "[h]", 3) || !Q_strncmp( s, "[H]", 3 )) {
                result << ent->client->pers.lasthealth;
				s += 3;
				continue;
			}

            // Last person you killed
			if (!Q_strncmp( s, "[k]", 3) || !Q_strncmp( s, "[K]", 3 )) {
                result << ent->client->pers.lastkill;
				s += 3;
				continue;
			}

            // Your location
			if (!Q_strncmp( s, "[l]", 3) || !Q_strncmp( s, "[L]", 3 )) {
                result << BG_GetLocationString(ent->r.currentOrigin);
				s += 3;
				continue;
			}

            // Your name
			if (!Q_strncmp( s, "[n]", 3) || !Q_strncmp( s, "[N]", 3 )) {
                result << ent->client->pers.netname;
				s += 3;
				continue;
			}

            // Last person who revived you
			if (!Q_strncmp( s, "[r]", 3) || !Q_strncmp( s, "[R]", 3 )) {
                result << ent->client->pers.lastrevive;
				s += 3;
				continue;
			}

            // Player under cursor
			if (!Q_strncmp( s, "[p]", 3) || !Q_strncmp( s, "[P]", 3 )) {
				vec3_t		end;
				vec3_t		forward, right, up, offset;

				AngleVectors (ent->client->ps.viewangles, forward, right, up);
				CalcMuzzlePointForActivate (ent, forward, right, up, offset);
				VectorMA (offset, 8192, forward, end);

                // Check for a player under the cursor
                TraceContext trx(*ent, *ent, MASK_PLAYERSOLID, reinterpret_cast<vec_t(&)[3]>(*offset), reinterpret_cast<vec_t(&)[3]>(*end) );
                if (AbstractBulletModel::fireWorld( trx ) && trx.resultIsPlayer()) {
                    gentity_t* traceEnt = &g_entities[ trx.data.entityNum ];
				    if (ent->client->sess.sessionTeam == traceEnt->client->sess.sessionTeam) {
                        result << traceEnt->client->pers.netname;
					    s += 3;
					    continue;
				    }
                }
	
                // Not pointing at anyone
                result << "nobody";
				s += 3;
				continue;
			}

            // Current health
			if (!Q_strncmp( s, "[s]", 3) || !Q_strncmp( s, "[S]", 3 )) {
                result << ent->health;
				s += 3;
				continue;
			}

            // Current weapon
			if (!Q_strncmp( s, "[w]", 3) || !Q_strncmp( s, "[W]", 3 )) {
				char * weapon;

                // Find description of current weapon
                weapon = G_WeaponNumToStr( (weapon_t)ent->client->ps.weapon );
				if (weapon) {
					result << weapon;
					s += 3;
					continue;
				}

                // No valid weapon
				result << "nothing";
				s += 3;
				continue;
			}

            // Current ammo total for current weapon
			if (!Q_strncmp( s, "[t]", 3) || !Q_strncmp( s, "[T]", 3 )) {
				int total = 0;

                // Get total ammo
				total += ent->client->ps.ammoclip[ent->client->ps.weapon];
				total += ent->client->ps.ammo[ent->client->ps.weapon];

				result << total;
				s += 3;
				continue;
			}
		}

        // Just a normal character
		result << *(s++);
	}

    buf = result.str();

    // Make sure the text isn't too long
    if (buf.length() >= MAX_SAY_TEXT)
        buf.resize(MAX_SAY_TEXT - 1);
}

/*
====================
G_FallDamage
Facilitates goomba kills
From ETPub
====================
*/
void G_FallDamage( gentity_t *ent, int event ) {
	int damage = 0;
	int kb_time = 0;
	gentity_t *victim;

	if ( ent->s.eType != ET_PLAYER ) {
		return;		// not in the player model
	}

	victim = &level.gentities[ent->s.groundEntityNum];
	// groundEntityNum won't be set to the entity number
	// of a wounded player if you landed on one.
	// trace to see if we're on a wounded player.

	if( !victim->client ) {
		trace_t tr;
		vec3_t start, stop;

		VectorCopy( ent->r.currentOrigin, start );
		VectorCopy( ent->r.currentOrigin, stop );
		stop[2] -= 4;
		trap_Trace( &tr, start, NULL, NULL, stop, ent->s.number, MASK_SHOT );
		victim = &level.gentities[tr.entityNum];
		
	}

	switch( event ) {
	case EV_FALL_NDIE:
		damage = 500;
		break;
	case EV_FALL_DMG_50:
		damage = 50;
		kb_time = 1000;
		break;
	case EV_FALL_DMG_25:
		damage = 25;
		kb_time = 500;
		break;
	case EV_FALL_DMG_15:
		damage = 15;
		kb_time = 250;
		break;
	case EV_FALL_DMG_10:
		damage = 10;
		kb_time = 250;
		break;
	case EV_FALL_SHORT:
		break;
	default:
		return;
	}

	if( !g_goomba.integer || !victim || !victim->client || !victim->takedamage ) {
		if( damage ) {
			if( kb_time ) {
				ent->client->ps.pm_time = kb_time;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
			}

			// no normal pain sound
			ent->pain_debounce_time = level.time + 200;
			G_Damage( ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING );
		}
		return;
	}

	// if we make it this far, do goomba damage to victim
	if( !damage )
		damage = 5;

	if( (g_friendlyFire.integer & FF_ENABLE) || !OnSameTeam( ent, victim )) {
		if( kb_time ) {
			victim->client->ps.pm_time = kb_time;
			victim->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}

		// no normal pain sound
		victim->pain_debounce_time = level.time + 200;	
		G_Damage( victim, ent, ent, NULL, NULL, (damage * g_goomba.integer), 0, MOD_GOOMBA );

		G_AddEvent( victim, EV_GENERAL_SOUND, G_SoundIndex("sound/xmod/goomba.wav" ));

        // Check if they died
        if (victim->client->ps.stats[STAT_HEALTH] < 0)
            G_AddSkillPoints( ent, SK_BATTLE_SENSE, 3.f );
	}

	if( damage > 5 ) {
		// faller has a soft landing
		damage = int( damage * 0.2f );
		G_Damage( ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING );
	}
}

/*
================
G_UpdateXmodCS
----------------
Jaybird
Sends Xmod's client side data via server commands (RPCS approach).
This avoids counting towards the 16000 byte MAX_GAMESTATE_CHARS limit.
Data is sent via "xcs" server command instead of configstrings.
If a cvar is here and is a game-tunable it should probably be flagged with CVAR_XMODINFO.
================
*/

/*
================
G_BuildXmodCS
Build the xmod configstring data into the provided buffers.
Used by both G_UpdateXmodCS (broadcast) and G_SendXmodCS (per-client).
================
*/
static void G_BuildXmodCS( char* cs, int csSize, char* cs2, int cs2Size ) {
    cs[0] = '\0';
    cs2[0] = '\0';

    // XMODINFO - Original CVARs
    Info_SetValueForKey( cs, "jver", XMOD_title );

    // CVARS
    Info_SetValueForKey( cs, "0", cvars::bg_bulletmode.svalue );
    Info_SetValueForKey( cs, "1", cvars::bg_hitmode.svalue );

    Info_SetValueForKey( cs, "2", cvars::bg_ammoUnlimited.svalue );
    Info_SetValueForKey( cs, "3", cvars::bg_ammoFireDelayNudge.svalue );
    Info_SetValueForKey( cs, "4", cvars::bg_ammoNextDelayNudge.svalue );

    Info_SetValueForKey( cs, "5", cvars::bg_dynamiteTime.svalue );
    Info_SetValueForKey( cs, "6", cvars::bg_glow.svalue );
    Info_SetValueForKey( cs, "7", cvars::bg_misc.svalue );
    Info_SetValueForKey( cs, "8", cvars::bg_panzerWar.svalue );
    Info_SetValueForKey( cs, "9", cvars::bg_poisonSyringes.svalue );
    Info_SetValueForKey( cs, "a", cvars::bg_skills.svalue );
    Info_SetValueForKey( cs, "b", cvars::bg_sniperWar.svalue );
    Info_SetValueForKey( cs, "c", cvars::bg_weapons.svalue );
    Info_SetValueForKey( cs, "d", cvars::bg_wolfrof.svalue );

    // Class Restrictions
    char val_buf[16];
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxEngineers.ivalue);
    Info_SetValueForKey( cs, "e", val_buf );
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxMedics.ivalue);
    Info_SetValueForKey( cs, "f", val_buf );
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxFieldOps.ivalue);
    Info_SetValueForKey( cs, "g", val_buf );
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxCovertOps.ivalue);
    Info_SetValueForKey( cs, "h", val_buf );

    // Weapon Restrictions
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxPanzers.ivalue);
    Info_SetValueForKey( cs, "i", val_buf );
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxMG42s.ivalue);
    Info_SetValueForKey( cs, "j", val_buf );
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxMortars.ivalue);
    Info_SetValueForKey( cs, "k", val_buf );
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxGrenLaunchers.ivalue);
    Info_SetValueForKey( cs, "l", val_buf );
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxFlamers.ivalue);
    Info_SetValueForKey( cs, "m", val_buf );
    Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxM97s.ivalue);
	Info_SetValueForKey( cs, "n", val_buf );
	Com_sprintf(val_buf, sizeof(val_buf), "%d", cvars::bg_maxPPSHs.ivalue);
	Info_SetValueForKey(cs, "K", val_buf);

    // Skill-5 cvars
    Info_SetValueForKey( cs, "o", cvars::bg_sk5_battle.svalue );
    Info_SetValueForKey( cs, "p", cvars::bg_sk5_lightweap.svalue );
    Info_SetValueForKey( cs, "q", cvars::bg_sk5_cvops.svalue );
    Info_SetValueForKey( cs, "r", cvars::bg_sk5_eng.svalue );
    Info_SetValueForKey( cs, "s", cvars::bg_sk5_fdops.svalue );
    Info_SetValueForKey( cs, "t", cvars::bg_sk5_medic.svalue );
    Info_SetValueForKey( cs, "u", cvars::bg_sk5_soldier.svalue );

    // Skill Specific
    Info_SetValueForKey( cs, "v", cvars::bg_covertops.svalue );

    // Fixed physics
    // Force fixedphysicsfps range.  This is per etpub's implementation
    if (cvars::bg_fixedphysicsfps.ivalue < 60)
        cvars::bg_fixedphysicsfps.set("60");
    else if (cvars::bg_fixedphysicsfps.ivalue > 333)
        cvars::bg_fixedphysicsfps.set("333");

    Info_SetValueForKey( cs, "w", cvars::bg_fixedphysics.svalue );
    Info_SetValueForKey( cs, "x", cvars::bg_fixedphysicsfps.svalue );

    Info_SetValueForKey( cs, "z", cvars::bg_proneDelay.svalue );

    // XMODINFO2 - Newer CVARs
    Info_SetValueForKey( cs2, "A", cvars::bg_doubleJump.svalue );
    Info_SetValueForKey( cs2, "B", cvars::bg_djHeight.svalue );
    Info_SetValueForKey( cs2, "C", cvars::bg_weaponsenable.svalue );

    // g_noReload and g_noCharge for bgame
    Info_SetValueForKey( cs2, "D", cvars::g_noReload.svalue );
    Info_SetValueForKey( cs2, "E", cvars::g_noCharge.svalue );

    // g_spawnInvulNoClip for client-side prediction
    Info_SetValueForKey( cs2, "F", va("%i", g_spawnInvulNoClip.integer) );

    // JXAC enabled status - so client knows whether to run anticheat
    Info_SetValueForKey( cs2, "G", cvars::g_jxacEnable.svalue );
    
    // JXAC feature flags - client checks these before sending data
    Info_SetValueForKey( cs2, "H", cvars::g_jxacModuleScan.svalue );      // Module scan enabled
    Info_SetValueForKey( cs2, "I", cvars::g_jxacAntiTamper.svalue );      // Anti-tamper enabled
    Info_SetValueForKey( cs2, "J", cvars::g_jxacCheckSpeedhack.svalue );  // Speedhack check enabled

	// "K" is taken!!! (shit code -> shit fixes)

    // Missile Cameras
    Info_SetValueForKey( cs2, "L", va("%i", g_missileCams.integer) );
}

void G_UpdateXmodCS() {
    char cs[MAX_INFO_STRING];
    char cs2[MAX_INFO_STRING];

    G_BuildXmodCS( cs, sizeof(cs), cs2, sizeof(cs2) );

    trap_SendServerCommand( -1, va("xcs 1 \"%s\"", cs) );
    trap_SendServerCommand( -1, va("xcs 2 \"%s\"", cs2) );
}

/*
================
G_SendXmodCS
----------------
Marks a connecting client for deferred xmod data sends.
Called during ClientBegin. ALL sends are deferred to G_ProcessPendingCommands()
which runs each frame and throttles to CMDS_PER_CLIENT_PER_FRAME to prevent
"CL_GetServerCommand: a reliable command was cycled out".
================
*/
void G_SendXmodCS( int clientNum ) {
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
        return;
    }

    // Mark this client for deferred XCS + NCS sends
    level.xcsPendingPhase[clientNum] = 0;
    level.ncsPendingNext[clientNum] = 0;
}

// XCS phases for per-frame incremental sending.
// Phases 0-3 are fixed commands; phases 4+ are weapon scripts (one per weapon index);
// after all weapons, remaining fixed commands are sent.
#define XCS_PHASE_CS1       0
#define XCS_PHASE_CS2       1
#define XCS_PHASE_SKILLS    2
#define XCS_PHASE_WATERMARK 3
#define XCS_PHASE_WEAPONS   4  // phases 4 through 4+WP_NUM_WEAPONS-1
#define XCS_PHASE_MAPXP_A   (XCS_PHASE_WEAPONS + WP_NUM_WEAPONS)
#define XCS_PHASE_MAPXP_B   (XCS_PHASE_MAPXP_A + 1)
#define XCS_PHASE_CHARGE    (XCS_PHASE_MAPXP_B + 1)
#define XCS_PHASE_FILTER    (XCS_PHASE_CHARGE + 1)
#define XCS_PHASE_ENDGAME   (XCS_PHASE_FILTER + 1)
#define XCS_PHASE_DONE      (XCS_PHASE_ENDGAME + 1)

/*
================
G_SendXcsPhased
----------------
Sends one XCS command per call based on the current phase.
Returns 1 if a command was sent, 0 if the phase was skipped (no data).
Advances xcsPendingPhase. Sets to -1 when all phases complete.
================
*/
static int G_SendXcsPhased( int clientNum ) {
    int phase = level.xcsPendingPhase[clientNum];

    if ( phase < 0 || phase >= XCS_PHASE_DONE ) {
        level.xcsPendingPhase[clientNum] = -1;
        return 0;
    }

    int sent = 0;

    switch ( phase ) {
    case XCS_PHASE_CS1: {
        char cs[MAX_INFO_STRING], cs2[MAX_INFO_STRING];
        G_BuildXmodCS( cs, sizeof(cs), cs2, sizeof(cs2) );
        trap_SendServerCommand( clientNum, va("xcs 1 \"%s\"", cs) );
        sent = 1;
        break;
    }
    case XCS_PHASE_CS2: {
        char cs[MAX_INFO_STRING], cs2[MAX_INFO_STRING];
        G_BuildXmodCS( cs, sizeof(cs), cs2, sizeof(cs2) );
        trap_SendServerCommand( clientNum, va("xcs 2 \"%s\"", cs2) );
        sent = 1;
        break;
    }
    case XCS_PHASE_SKILLS: {
        char info[MAX_INFO_STRING] = { '\0' };
        for (int i = 0; i < SK_NUM_SKILLS; i++) {
            Info_SetValueForKey(info, va("skill_%i",i), va("%i %i %i %i %i",
                skillLevels[i][1], skillLevels[i][2], skillLevels[i][3],
                skillLevels[i][4], skillLevels[i][5]));
        }
        trap_SendServerCommand( clientNum, va("xcs s \"%s\"", info) );
        sent = 1;
        break;
    }
    case XCS_PHASE_WATERMARK: {
        char wm[MAX_INFO_STRING] = { '\0' };
        Info_SetValueForKey( wm, "wmFA", va("%i", g_watermarkFadeAfter.integer));
        Info_SetValueForKey( wm, "wmFT", va("%i", g_watermarkFadeTime.integer));
        Info_SetValueForKey( wm, "wmFN", g_watermark.string );
        trap_SendServerCommand( clientNum, va("xcs m \"%s\"", wm) );
        sent = 1;
        break;
    }
    case XCS_PHASE_MAPXP_A:
        if ( level.axisMapsXP[0] ) {
            trap_SendServerCommand( clientNum, va("xcs a \"%s\"", level.axisMapsXP) );
            sent = 1;
        }
        break;
    case XCS_PHASE_MAPXP_B:
        if ( level.alliedMapsXP[0] ) {
            trap_SendServerCommand( clientNum, va("xcs b \"%s\"", level.alliedMapsXP) );
            sent = 1;
        }
        break;
    case XCS_PHASE_CHARGE:
        if ( level.rpcsChargeTimes[0] ) {
            trap_SendServerCommand( clientNum, va("xcs c \"%s\"", level.rpcsChargeTimes) );
            sent = 1;
        }
        break;
    case XCS_PHASE_FILTER:
        if ( level.rpcsFilterCams[0] ) {
            trap_SendServerCommand( clientNum, va("xcs f \"%s\"", level.rpcsFilterCams) );
            sent = 1;
        }
        break;
    case XCS_PHASE_ENDGAME:
        if ( level.rpcsEndgameStats[0] ) {
            trap_SendServerCommand( clientNum, va("xcs e \"%s\"", level.rpcsEndgameStats) );
            sent = 1;
        }
        break;
    default:
        // Weapon script phases (XCS_PHASE_WEAPONS + weaponIndex)
        if ( phase >= XCS_PHASE_WEAPONS && phase < XCS_PHASE_MAPXP_A ) {
            int wpIdx = phase - XCS_PHASE_WEAPONS;
            weaponScriptDef_t *script = G_GetWeaponScript( wpIdx );
            if ( script && script->hasScript ) {
                char wcs[MAX_INFO_STRING];
                wcs[0] = '\0';
                if ( script->name[0] )
                    Info_SetValueForKey( wcs, "n", script->name );
                if ( script->killMessage[0] )
                    Info_SetValueForKey( wcs, "k", script->killMessage );
                if ( script->killMessage2[0] )
                    Info_SetValueForKey( wcs, "l", script->killMessage2 );
                if ( script->selfKillMessage[0] )
                    Info_SetValueForKey( wcs, "s", script->selfKillMessage );
                trap_SendServerCommand( clientNum, va("xcs w %i \"%s\"", wpIdx, wcs) );
                sent = 1;
            }
        }
        break;
    }

    level.xcsPendingPhase[clientNum] = phase + 1;
    if ( phase + 1 >= XCS_PHASE_DONE ) {
        level.xcsPendingPhase[clientNum] = -1;
    }

    return sent;
}

/*
================
G_ProcessPendingCommands
----------------
Called every server frame from G_RunFrame.
Processes all deferred reliable commands with per-client throttling:
1. CS_PLAYERS dirty flags → trap_SetConfigstring (max 4 per frame total)
2. Per-client XCS pending → sends xcs commands when throttle allows
3. Per-client NCS pending → sends ncs entries when throttle allows
4. NCS dirty flags → broadcasts runtime changes to completed clients

Limited to CMDS_PER_CLIENT_PER_FRAME reliable commands per client per frame
to prevent "CL_GetServerCommand: a reliable command was cycled out".
================
*/
void G_ProcessPendingCommands( void ) {
    // Wait for clients to finish loading after map_restart before sending deferred commands.
    // During loading, clients can't acknowledge reliable commands, so the engine's 128-slot
    // buffer accumulates. Init broadcasts (configstrings, mapconfigs) must be acknowledged
    // first before we add XCS/NCS/CS_PLAYERS to the buffer.
    if ( level.deferredSendDelay > 0 ) {
        level.deferredSendDelay--;
        return;
    }

    // Part 1: Process CS_PLAYERS dirty flags (throttled to 2 per frame total)
    // Each trap_SetConfigstring broadcasts to ALL clients, consuming one reliable
    // command slot per client. With 33+ clients, we keep this very low.
    {
        int csProcessed = 0;
        for ( int i = 0; i < level.maxclients && csProcessed < 2; i++ ) {
            if ( level.csPlayersDirty[i] ) {
                level.csPlayersDirty[i] = qfalse;
                trap_SetConfigstring( CS_PLAYERS + i, level.csPlayersData[i] );
                csProcessed++;
            }
        }
    }

    // Part 2: Process per-client deferred sends (XCS + NCS)
    for ( int c = 0; c < level.maxclients; c++ ) {
        if ( level.clients[c].pers.connected != CON_CONNECTED ) {
            level.xcsPendingPhase[c] = -1;
            level.ncsPendingNext[c] = -1;
            continue;
        }

        int budget = CMDS_PER_CLIENT_PER_FRAME;

        // 2a: Send XCS commands incrementally (one per call, phased)
        // Empty phases (no weapon scripts, no data) are skipped without consuming budget.
        // Loop is bounded by XCS_PHASE_DONE (~60 phases max).
        while ( level.xcsPendingPhase[c] >= 0 && budget > 0 ) {
            budget -= G_SendXcsPhased( c );
        }

        // 2b: Send pending NCS entries (deferred from G_SendXmodCS)
        if ( level.ncsPendingNext[c] >= 0 && budget > 0 ) {
            int sent = 0;
            int idx = level.ncsPendingNext[c];
            while ( idx < NCS_MAX && sent < budget ) {
                if ( level.ncs[idx][0] ) {
                    trap_SendServerCommand( c, va("ncs %i \"%s\"", idx, level.ncs[idx]) );
                    sent++;
                }
                idx++;
            }
            if ( idx >= NCS_MAX ) {
                level.ncsPendingNext[c] = -1;  // done
            } else {
                level.ncsPendingNext[c] = idx;  // continue next frame
            }
        }
    }

    // Part 3: Process dirty NCS entries (runtime changes like fireteam/OID updates)
    // These only go to clients that have COMPLETED their initial load (ncsPendingNext < 0).
    // Clients still receiving deferred data won't get these - they'll receive the updated
    // values when their pending scan reaches those indices (since ncs[] is already updated).
    // Throttled to NCS_DIRTY_PER_FRAME to prevent reliable command buffer overflow when
    // many NCS entries change at once (e.g. fireteam reshuffles, multiple OID updates).
    {
        int dirtyProcessed = 0;
        for ( int i = 0; i < NCS_MAX && dirtyProcessed < NCS_DIRTY_PER_FRAME; i++ ) {
            if ( !level.ncsDirty[i] ) {
                continue;
            }
            level.ncsDirty[i] = qfalse;
            dirtyProcessed++;

            const char *cmd = va( "ncs %i \"%s\"", i, level.ncs[i] );
            for ( int c = 0; c < level.maxclients; c++ ) {
                if ( level.clients[c].pers.connected == CON_CONNECTED
                     && level.ncsPendingNext[c] < 0 ) {
                    trap_SendServerCommand( c, cmd );
                }
            }
        }
    }
}

/*************************************************
				Poison Syringes
*************************************************/
void G_ResetPoisonEvent( gentity_t *ent, int eventNum ) {
	if (eventNum < MAX_POISONEVENTS && eventNum >= 0) {
		ent->client->pmext.poisonEvents[eventNum].fireTime = 0;
		ent->client->pmext.poisonEvents[eventNum].poisoner = 0;
	}
}

void G_ResetPoisonEvents( gentity_t *ent ) {
	int i;
	if (!ent || !ent->client)
		return;

	for( i = 0; i < MAX_POISONEVENTS; i++ ){
		G_ResetPoisonEvent( ent, i );
	}
}

void G_AddPoisonEvent( gentity_t *ent, gentity_t *attacker ) {
	int best = -1;
	int besttime = 0;
	int i;

	if (!ent || !ent->client || !attacker || !attacker->client)
		return;

	for( i = 0; i < MAX_POISONEVENTS; i++ ) {
		if( !ent->client->pmext.poisonEvents[i].fireTime ) {
			best = i;
			break;
		}

		if( best == -1 || ent->client->pmext.poisonEvents[i].fireTime < besttime ) {
			besttime = ent->client->pmext.poisonEvents[i].fireTime;
			best = i;
			continue;
		}
	}

	ent->client->pmext.poisonEvents[best].fireTime = level.time;
	ent->client->pmext.poisonEvents[best].poisoner = attacker-g_entities;
}

void G_RunPoisonEvents( gentity_t *ent ) {
	gentity_t *attacker;
	int i;

	if( !ent || !ent->client )
		return;

	for( i = 0; i < MAX_POISONEVENTS; i++ ) {
		if( ent->client->pmext.poisonEvents[i].fireTime && ent->client->pmext.poisonEvents[i].fireTime < level.time ) {

			// Error check the attacker
			attacker = g_entities + ent->client->pmext.poisonEvents[i].poisoner;
			if( !attacker || !attacker->client || !ISONTEAM(attacker) ) {
				G_ResetPoisonEvent( ent, i );
				continue;
			}

			// Set next interval
			ent->client->pmext.poisonEvents[i].fireTime = level.time + POISONINTERVAL;

			// Damage
			if ((g_friendlyFire.integer & FF_ENABLE) || !OnSameTeam( ent, attacker )) {
				G_Damage( ent, attacker, attacker, 0, 0, POISONDAMAGE, 0, MOD_POISON_SYRINGE );
				// XP
				if( !OnSameTeam( ent, attacker ) ) {
					G_AddSkillPoints( attacker, SK_BATTLE_SENSE, 0.5f );
				} else {
					G_LoseSkillPoints( attacker, SK_BATTLE_SENSE, 0.5f );
				}
			}
		}
	}
}

qboolean G_IsPoisoned( gentity_t *ent ) {
	int i;

	if (!ent || !ent->client)
		return qfalse;

	for( i = 0; i < MAX_POISONEVENTS; i++ ) {
		if( ent->client->pmext.poisonEvents[i].fireTime )
			return qtrue;
	}

	return qfalse;
}

void G_ResetOnePoisonEvent( gentity_t *ent ) {
	int i;

	if (!ent || !ent->client)
		return;

	if (!G_IsPoisoned( ent ))
		return;

	do {
		i = rand() % MAX_POISONEVENTS;
	} while (!ent->client->pmext.poisonEvents[i].fireTime);

	G_ResetPoisonEvent( ent, i );
}

/*
===============
G_PlaySound_Cmd
---------------
Jaybird
Plays a full volume sound for a given player
===============
*/
void G_PlaySound_Cmd( void )
{
    vector<string> args;
    Engine::args(args);

    if (args.size() < 2 || args.size() > 3) {
        G_Printf("usage: playsound [name|#] <sound>\n");
        return;
    }

    if (args.size() == 3) {
        Client* client;
        text::Buffer ebuf;
        if (cmd::matchClient(args[1].c_str(), client, ebuf)) {
            text::Buffer buf;
            buf << "playsound: " << ebuf;
            cmd::print(NULL, buf, false);
            return;
        }

    	int index = G_SoundIndex(args[2].c_str());
        gentity_t* ent = G_TempEntity(client->gentity.r.currentAngles, EV_GLOBAL_CLIENT_SOUND);
        ent->s.teamNum = client->slot;
        ent->s.eventParm = index;
    } else {
		G_globalSound(args[1].c_str());
	}
}

/*
==================
G_PlaySoundEnv_Cmd
------------------
Jaybird
Plays a sound in relative to a player's position
==================
*/
void G_PlaySoundEnv_Cmd(void)
{
    vector<string> args;
    Engine::args(args);

    if (args.size() != 3) {
		G_Printf("usage: playsound_env <name|#> <sound>\n");
		return;
	}

    Client* client;
    text::Buffer ebuf;
    if (cmd::matchClient(args[1], client, ebuf)) {
        text::Buffer buf;
        buf << "playsound_env: " << ebuf;
        cmd::print(NULL, buf, false);
        return;
    }

	int index = G_SoundIndex(args[2].c_str());
	G_Sound(&client->gentity, index);
}

/*
=====================
Shoutcaster Functions
---------------------
The following functions handle the logging in/assignment
of players to be designated as shoutcasters
=====================
*/
void G_ShoutcasterLogin(gentity_t *ent)
{
    if (!ent) {
        G_Printf("Console cannot login to shoutcaster.\n");
        return;
    }

	if (ent->client->sess.shoutcaster) {
		CP("print \"You are already a shoutcaster.\n\"");
		return;
	}

	if (!*g_shoutcastpassword.string) {
		CP("print \"Sorry, shoutcasters are disabled on this server.\n\"");
		return;
	}

    vector<string> args;
    Engine::args(args);

    if (args.size() != 2) {
		CP(va("print \"Usage: %s <password>\n\"", args[0].c_str()));
		return;
	}

	if (args[1] != g_shoutcastpassword.string) {
		CP("print \"Invalid shoutcaster password!\n\"");
		return;
	}

	ent->client->sess.shoutcaster = 1;
	ClientUserinfoChanged(ent-g_entities);
}

void G_ShoutcasterLogout(gentity_t *ent)
{
    if (!ent) {
        G_Printf("Console cannot logout of shoutcaster\n");
        return;
    }

	if (!ent->client->sess.shoutcaster) {
		CP("print \"You are not a shoutcaster.\n\"");
		return;
	}

	ent->client->sess.shoutcaster = 0;
	ClientUserinfoChanged(ent-g_entities);
}

void G_MakeShoutcaster()
{
    vector<string> args;
    Engine::args(args);

    if (args.size() != 2) {
		G_Printf("print \"Usage: %s <name|#>\n\"", args[0].c_str());
		return;
	}

    text::Buffer ebuf;
    Client* client;
    if (cmd::matchClient(args[1], client, ebuf)) {
        text::Buffer buf;
        buf << args[0] << ": " << ebuf;
        cmd::print(NULL, buf);
        return;
    }

    client->gclient.sess.shoutcaster = 1;
    ClientUserinfoChanged(client->slot);
}

void G_RemoveShoutcaster()
{
    vector<string> args;
    Engine::args(args);

    if (args.size() != 2) {
		G_Printf("print \"Usage: %s <name|#>\n\"", args[0].c_str());
		return;
	}

    text::Buffer ebuf;
    Client* client;
    if (cmd::matchClient(args[1], client, ebuf)) {
        text::Buffer buf;
        buf << args[0] << ": " << ebuf;
        cmd::print(NULL, buf);
        return;
    }

    client->gclient.sess.shoutcaster = 0;
    ClientUserinfoChanged(client->slot);
}

/*
==================
G_LiveUniformSteal
==================
*/
qboolean G_LiveUniformSteal(gentity_t *ent, gentity_t *traceEnt) {

	if (!(cvars::bg_covertops.ivalue & COPS_LIVEUNI))
		return qfalse;
	if ( G_HasJayFlag( traceEnt, 1, JF_LOSTPANTS ))
		return qfalse; //already lost uniform
	if (ent->client->sess.playerType != PC_COVERTOPS)
		return qfalse;
	if (ent->health <= 0) 
		return qfalse;
	if (ent->client->ps.powerups[PW_OPS_DISGUISED])
		return qfalse;
	if (ent->client->ps.powerups[PW_BLUEFLAG] || ent->client->ps.powerups[PW_REDFLAG])
		return qfalse;
	if (traceEnt->stealProgress < 250 ) {
		traceEnt->stealProgress += 5;
		traceEnt->stealTime = level.time;
		return qfalse;
	}

	// Reset steal time in case they lose their uni again :X
	traceEnt->stealProgress = 0;

	ent->client->ps.powerups[PW_OPS_DISGUISED] = 1;
	ent->client->ps.powerups[PW_OPS_CLASS_1] = traceEnt->client->sess.playerType & 1;
	ent->client->ps.powerups[PW_OPS_CLASS_2] = traceEnt->client->sess.playerType & 2;
	ent->client->ps.powerups[PW_OPS_CLASS_3] = traceEnt->client->sess.playerType & 4;

	// Take away their uniform
	G_SetJayFlag( traceEnt, 1, JF_LOSTPANTS );
	traceEnt->client->ps.powerups[PW_OPS_DISGUISED] = 0;

	// And give stealer's back
	G_RemoveJayFlag( ent, 1, JF_LOSTPANTS );

	// sound effect
	G_AddEvent( ent, EV_DISGUISE_SOUND, 0 );

	G_AddSkillPoints( ent, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, 5.f );
	G_DebugAddSkillPoints( ent, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, 5, "stealing uniform of a live player" ); 

	Q_strncpyz( ent->client->disguiseNetname, g_entities[traceEnt->s.clientNum].client->pers.netname, sizeof(ent->client->disguiseNetname) );
	ent->client->disguiseRank = g_entities[traceEnt->s.clientNum].client ? g_entities[traceEnt->s.clientNum].client->sess.rank : 0;

	ClientUserinfoChanged( ent->s.clientNum );

	return qtrue;
}

/*
====================
G_UniformRestore
Jaybird
====================
*/
qboolean G_UniformRestore(gentity_t *stealer, gentity_t *deadguy) 
{
	if ( !G_HasJayFlag( stealer, 1, JF_LOSTPANTS ))
		return qfalse;
	if (deadguy->activator)
		return qfalse; // already used corpse
	if (BODY_TEAM(deadguy) >= 4)
		return qfalse;
	if (BODY_VALUE(deadguy) < 250) {
		BODY_VALUE(deadguy) += 5;
		return qtrue;
	}

	deadguy->nextthink = deadguy->timestamp + BODY_TIME(BODY_TEAM(deadguy));

	deadguy->activator = stealer;
	deadguy->s.time2 =	1;

	// sound effect
	G_AddEvent(stealer, EV_DISGUISE_SOUND, 0);

	// I guess this disables future use of the corpse 
	BODY_TEAM(deadguy) += 4;

	G_RemoveJayFlag( stealer, 1, JF_LOSTPANTS );

	return qtrue;
}

/*
===================
JayFlags functions
===================
*/
void G_SetJayFlag( gentity_t *ent, int set, int flag ) {
	if( !ent || !ent->client )
		return;

	if( set == 1 ) {
		ent->client->ps.persistant[PERS_JAYFLAGS1] |= flag;
		ent->s.effect1Time |= flag;
	}
	else if( set == 2 ) {
		ent->client->ps.persistant[PERS_JAYFLAGS2] |= flag;
		ent->s.effect2Time |= flag;
	}
}

void G_RemoveJayFlag( gentity_t *ent, int set, int flag ) {
	if( !ent || !ent->client )
		return;

	if( set == 1 ) {
		ent->client->ps.persistant[PERS_JAYFLAGS1] &= ~flag;
		ent->s.effect1Time &= ~flag;
	}
	else if( set == 2 ) {
		ent->client->ps.persistant[PERS_JAYFLAGS2] &= ~flag;
		ent->s.effect2Time &= ~flag;
	}
}

qboolean G_HasJayFlag( gentity_t *ent, int set, int flag ) {
	if( !ent || !ent->client )
		return qfalse;
	if( set == 1 ) {
		if( ent->client->ps.persistant[PERS_JAYFLAGS1] & flag )
			return qtrue;
		if( ent->s.effect1Time & flag )
			return qtrue;
	}
	else if( set == 2 ) {
		if( ent->client->ps.persistant[PERS_JAYFLAGS2] & flag )
			return qtrue;
		if( ent->s.effect2Time & flag )
			return qtrue;
	}
	return qfalse;
}

/*
=============
G_SkillForMod
-------------
Jaybird
Returns a skill for a given method of death
=============
*/
int G_SkillForMOD( int wp ) {
	switch( wp ) {
		case MOD_KNIFE:
		case MOD_THROWING_KNIFE:
		case MOD_LUGER:
		case MOD_COLT:
		case MOD_MP40:
		case MOD_THOMPSON:
		case MOD_STEN:
		case MOD_GARAND:
		case MOD_SILENCER:
		case MOD_FG42:
		case MOD_CARBINE:
		case MOD_KAR98:
		case MOD_SILENCED_COLT:
		case MOD_K43:
		case MOD_AKIMBO_COLT:
		case MOD_AKIMBO_LUGER:
		case MOD_AKIMBO_SILENCEDCOLT:
		case MOD_AKIMBO_SILENCEDLUGER:
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
		case MOD_SMOKEGRENADE:
		case MOD_PPSH:
		case MOD_M97:
			return SK_LIGHT_WEAPONS;
		case MOD_MOBILE_MG42:
		case MOD_MACHINEGUN:
		case MOD_BROWNING:
		case MOD_MG42:
		case MOD_PANZERFAUST:
		case MOD_FLAMETHROWER:
		case MOD_MORTAR:
			return SK_HEAVY_WEAPONS;
		case MOD_GARAND_SCOPE:
		case MOD_K43_SCOPE:
		case MOD_FG42SCOPE:
		case MOD_SATCHEL:
		case MOD_POISON_GAS:
			return SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS;
		case MOD_DYNAMITE:
		case MOD_LANDMINE:
		case MOD_GPG40:
		case MOD_M7:
			return SK_EXPLOSIVES_AND_CONSTRUCTION;
		case MOD_ARTY:
		case MOD_AIRSTRIKE:
			return SK_SIGNALS;
		case MOD_POISON_SYRINGE:
        case MOD_GOOMBA:
            return SK_BATTLE_SENSE;
		default:
			return -1;
	}
}

/*
==================
G_CanisterKickTouch
------------------
Touch handler for kicking canisters (grenades, airstrikes, smokegrenades)
when g_canisterKick is enabled.
==================
*/
void G_CanisterKickTouch( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	vec3_t kickDir, kickVel;
	float kickDistance;
	float velocitySq;

	// Debug output
	if( g_developer.integer ) {
		G_Printf("G_CanisterKickTouch called: ent=%d, other=%d\n", 
			ent ? (int)(ent - g_entities) : -1, 
			other ? (int)(other - g_entities) : -1);
	}

	// Only clients can kick canisters
	if( !other->client ) {
		if( g_developer.integer ) G_Printf("  -> Failed: other is not a client\n");
		return;
	}

	// Must be enabled
	if( !g_canisterKick.integer ) {
		if( g_developer.integer ) G_Printf("  -> Failed: g_canisterKick is disabled\n");
		return;
	}

	// Don't kick if the canister is moving very fast (airborne at high speed)
	// Grenades can be kicked when they've slowed down to reasonable speeds
	velocitySq = VectorLengthSquared( ent->s.pos.trDelta );
	if( velocitySq > SQR(300) ) {
		if( g_developer.integer ) G_Printf("  -> Failed: velocity too high (%.0f > %d)\n", sqrt(velocitySq), 300);
		return;
	}

	// Get the kick direction based on player's facing direction
	VectorCopy( other->client->ps.viewangles, kickDir );
	kickDir[PITCH] = 0;  // Only use yaw for horizontal direction
	AngleVectors( kickDir, kickVel, NULL, NULL );

	// Get kick distance from cvar (default 250)
	kickDistance = g_canisterKickDistance.value;
	if( kickDistance < 50 ) {
		kickDistance = 50;  // Minimum kick
	} else if( kickDistance > 1000 ) {
		kickDistance = 1000;  // Maximum kick
	}

	// Apply kick velocity
	VectorScale( kickVel, kickDistance, kickVel );
	kickVel[2] = kickDistance * 0.4f;  // Give it some height proportional to distance

	// Update the trajectory
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	VectorCopy( kickVel, ent->s.pos.trDelta );
	ent->s.pos.trType = TR_GRAVITY;
	ent->s.pos.trTime = level.time;

	// Set new owner if enabled
	if( g_canisterKickOwner.integer && other->client ) {
		ent->r.ownerNum = other->s.number;
		ent->parent = other;
	}

	// Debug: Kick succeeded!
	if( g_developer.integer ) {
		G_Printf("  -> SUCCESS! Kicked canister with distance=%.0f, velocity=(%.0f, %.0f, %.0f)\n",
			kickDistance, kickVel[0], kickVel[1], kickVel[2]);
	}

	// Play kick sound
	G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex("sound/xmod/push.wav" ));
}

/*
==================
G_IsKickableCanister
------------------
Check if a weapon type is a kickable canister
==================
*/
qboolean G_IsKickableCanister( int weapon ) {
	switch( weapon ) {
		case WP_GRENADE_LAUNCHER:
		case WP_GRENADE_PINEAPPLE:
		case WP_SMOKE_MARKER:
		case WP_SMOKE_BOMB:
		case WP_POISON_GAS:
		case WP_BOMB:
			return qtrue;
		default:
			return qfalse;
	}
}

/*
==================
ThrowingKnifeTouch
------------------
Jaybird - handle ammo pickup of idle
throwing knives.
==================
*/
void ThrowingKnifeTouch( gentity_t *ent, gentity_t *other, trace_t *trace) {
	ent->active = qfalse;

	// Only clients can pick knives up.
	if( !other->client ) {
		return;
   }

	// Give ammo if they can.
	if( other->client->ps.ammo[BG_FindClipForWeapon(WP_KNIFE)] < BG_MaxAmmoForWeapon( WP_KNIFE, 0 ))
		other->client->ps.ammo[BG_FindClipForWeapon(WP_KNIFE)]++;
	else
		return;

	// Pickup Sound
	if( ent->noise_index )
		G_AddEvent( other, EV_GENERAL_SOUND, ent->noise_index );

	// Item Pickup event
	if( other->client->pers.predictItemPickup )
		G_AddPredictableEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
	else
		G_AddEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );

	// Cleanup
	ent->freeAfterEvent =	qtrue;
	ent->flags |=			FL_NODRAW;
	ent->r.svFlags |=		SVF_NOCLIENT;
	ent->s.eFlags |=		EF_NODRAW;
	ent->r.contents =		0;
	ent->nextthink =		0;
	ent->think =			0;
}

qboolean IsReflectable( int mod ) {
	switch( mod ) {
	case MOD_MACHINEGUN:
	case MOD_BROWNING:
	case MOD_MG42:
	case MOD_GRENADE:
	case MOD_ROCKET:
	case MOD_KNIFE:
	case MOD_LUGER:
	case MOD_COLT:
	case MOD_MP40:
	case MOD_THOMPSON:
	case MOD_STEN:
	case MOD_GARAND:
	case MOD_SNOOPERSCOPE:
	case MOD_SILENCER:
	case MOD_FG42:
	case MOD_FG42SCOPE:
	case MOD_PANZERFAUST:
	case MOD_GRENADE_LAUNCHER:
	case MOD_FLAMETHROWER:
	case MOD_GRENADE_PINEAPPLE:
	case MOD_MAPMORTAR:
	case MOD_MAPMORTAR_SPLASH:
	case MOD_CARBINE:
	case MOD_KAR98:
	case MOD_GPG40:
	case MOD_M7:
	case MOD_SATCHEL:
	case MOD_MOBILE_MG42:
	case MOD_SILENCED_COLT:
	case MOD_GARAND_SCOPE:
	case MOD_K43:
	case MOD_K43_SCOPE:
	case MOD_MORTAR:
	case MOD_AKIMBO_COLT:
	case MOD_AKIMBO_LUGER:
	case MOD_AKIMBO_SILENCEDCOLT:
	case MOD_AKIMBO_SILENCEDLUGER:
	case MOD_SMOKEGRENADE:
	case MOD_POISON_SYRINGE:
	case MOD_THROWING_KNIFE:
	case MOD_PPSH:
    case MOD_M97:
		return qtrue;
	}
	return qfalse;
}

void G_TeamDamageRestriction( gentity_t *ent ) {
	int friendly = 0;
	int hits = 0;
	int i;
	unsigned int j;
	float percent;

	if( g_teamDamageRestriction.integer <= 0 )
		return;

	// Get totals
	for( i = 0; i < MOD_NUM_MODS; i++ ) {
		if( !IsReflectable( i ))
			continue;

		if(( j = G_weapStatIndex_MOD( i )) == WS_MAX )
			continue;

		hits += ent->client->sess.aWeaponStats[j].totalHits;
		friendly += ent->client->sess.aWeaponStats[j].friendlyHits;
	}

	// No hits yet!
	if( !hits )
		return;

	// Percentage
	percent = (float)friendly / (float)hits * 100.f;

	// Do nothing if below minimum hits
	if( friendly < g_teamDamageMinHits.integer )
		return;

	if( cmd::entityHasPermission( ent, priv::base::voteImmunity ))
		return;

	// Kick 'em?
	if( (int)percent > g_teamDamageRestriction.integer ) {
		trap_DropClient( ent-g_entities, va( "You have been automatically kicked because %.0f percent of your shots were friendly fire shots.", percent ), cache::kickTime );
	}
}

void G_AuthCheck( gentity_t *ent, char *version ) {
	char userinfo[MAX_INFO_STRING];

	if( !ent || !ent->client )
		return;

	if( !ent->clientCheckAlarm )
		return;

	trap_GetUserinfo( ent-g_entities, userinfo, sizeof( userinfo ));

	// Check Xmod version
	if( !version || !*version ) {
		version = Info_ValueForKey( userinfo, "cg_xmod_title" );
	}

	if( !*version )
		version = "[MISSING INFO]";
	else if( !Q_stricmp( version, XMOD_title ))
		version = "";

	// Jaybird - just warn them.  ET Client is too buggy.
	if( *version ) {
		CP( "cpm \"\n^1IMPORTANT: ^3See your console for a game warning!\"" );
		CP( "print \"^3*** ^1Invalid client detected ^3***^7\n\"" );
		CP( va( "print \"^xDetected: ^1%s\n^xRequired: ^2%s\n\"", version, XMOD_title));
		CP( "print \"^3This server requires the ^2" XMOD_title " ^3client.\n\"" );
		CP( "print \"^3This might be fixed by closing and restarting your game.\n\"" );
		CP( "print \"^3If a restart fails, please notify a server admin for possible server misconfiguration.^7\n\n\"" );
	}

	// They pass the server checks
	ent->clientCheckAlarm = 0;
}

void G_UpdateUptime() {
    static int lastUpdate = 0;

    char buffer[32];
    time_t now;
    time_t delta;

    now = time(0);

    if ((now - lastUpdate) < 60)
        return;

    lastUpdate = now;

    if (sv_uptimeStamp.integer < 0) {
        trap_Cvar_Set("sv_uptimeStamp", va("%ld", now));
        delta = 0;
    }
    else {
        delta = now - sv_uptimeStamp.integer;
    }

    if (delta < 0)
        delta = 0;

    Com_sprintf(buffer, sizeof(buffer), "%02dd%02dh%02dm",
        (int)( delta         / 86400 ),
        (int)( delta % 86400 /  3600 ),
        (int)( delta %  3600 /    60 ));

    trap_Cvar_Set("sv_uptime", buffer);
}

bool G_MutePlayer(gentity_t* ent, string muter, string reason)
{
    int clientNum = ent - g_entities;
    
    // Check if already muted using session helper
    if (::xmod::isClientMuted(clientNum)) {
        return false;
    }

    // Set mute on both session and User
    ::xmod::setClientMuted(clientNum, true);
    
    // Set additional mute info using global helper
    time_t muteTime = time(NULL);
    time_t muteExpiry = g_muteTime.integer ? muteTime + str::toSeconds(g_muteTime.string) : 0;
    ::xmod::setClientMuteData(clientNum, muteTime, reason, SanitizeString(muter, false), muter, muteExpiry);

    // Sync to SQLite database
    if (xmod::g_database && xmod::g_database->isOpened() && 
        xmod::g_sessions[clientNum] && xmod::g_sessions[clientNum]->isAuthenticated()) {
        int userId = xmod::g_sessions[clientNum]->getUserId();
        if (userId > 0) {
            if (!xmod::g_database->setMuted(userId, true)) {
                G_Printf("^1G_MutePlayer: Failed to update muted status in SQLite for user %d\n", userId);
            }
        }
    }

    ClientUserinfoChanged(ent->s.number);

    return true;
}

bool G_UnmutePlayer(gentity_t* ent)
{
    int clientNum = ent - g_entities;
    
    // Check if not muted using session helper
    if (!::xmod::isClientMuted(clientNum)) {
        return false;
    }

    // Set unmute on both session and User
    ::xmod::setClientMuted(clientNum, false);
    
    // Clear additional mute info using global helper
    ::xmod::clearClientMuteData(clientNum);

    // Sync to SQLite database
    if (xmod::g_database && xmod::g_database->isOpened() && 
        xmod::g_sessions[clientNum] && xmod::g_sessions[clientNum]->isAuthenticated()) {
        int userId = xmod::g_sessions[clientNum]->getUserId();
        if (userId > 0) {
            if (!xmod::g_database->setMuted(userId, false)) {
                G_Printf("^1G_UnmutePlayer: Failed to update muted status in SQLite for user %d\n", userId);
            }
        }
    }

    ClientUserinfoChanged(ent->s.number);

    return true;
}

void G_BanPlayer(gentity_t* ent, string banner, string reason, int duration)
{
    int clientNum = ent - g_entities;
    time_t expires = duration ? time(NULL) + duration : 0; // 0 is a permanent ban
    
    // Get user info using session helpers
    string guid = ::xmod::getClientGuid(clientNum);
    string name = ::xmod::getClientName(clientNum);
    string hwid = "";  // Get HWID from authenticated session if available
    string ip = ::xmod::getClientIp(clientNum);
    
    // Check if this is a fake GUID using global helper
    bool isFakeGuid = ::xmod::isClientFakeGuid(clientNum);
    
    // If this is a fake GUID, we need to generate a permanent fake GUID
    if (isFakeGuid) {
        // Construct GUID - exactly 40 characters to match xmodguid format
        // BANLOC(6) + timestamp(10) + random hex(24) = 40 chars
        stringstream guidstream;
        guidstream << "BANLOC"
            << setw(10) << setfill('0') << dec << time(NULL)
            << hex << setfill('0')
            << setw(6) << ((rand() & 0xFFFFFF))   // 6 hex digits (000000-FFFFFF)
            << setw(6) << ((rand() & 0xFFFFFF))   // 6 hex digits
            << setw(6) << ((rand() & 0xFFFFFF))   // 6 hex digits
            << setw(6) << ((rand() & 0xFFFFFF));  // 6 hex digits
        guid = guidstream.str();

        // Ensure exactly 40 characters (should be guaranteed by above)
        if (guid.length() != 40) {
            if (guid.length() > 40) {
                guid.resize(40);
            } else {
                guid.append(40 - guid.length(), '0');
            }
        }
    }

    // Get HWID from xmod session if available
    if (xmod::g_sessions[clientNum] && xmod::g_sessions[clientNum]->isAuthenticated()) {
        hwid = xmod::g_sessions[clientNum]->getHwid();
    }
    
    // Add ban to SQLite database
    if (xmod::g_database && xmod::g_database->isOpened()) {
        xmod::g_database->banUser(guid, hwid, ip, name, banner, reason, expires);
    }
}

///////////////////////////////////////////////////////////////////////////////

namespace cache {
    string kickDuration;
    string kickMessage;
    int    kickTime;
} // namespace cache
