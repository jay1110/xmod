// cg_scoreboard -- draw the scoreboard on top of the game screen

#include <bgame/impl.h> 

#define	SCOREBOARD_WIDTH	(31*BIGCHAR_WIDTH)

// Ping thresholds for color coding
#define PING_THRESHOLD_GOOD		100		// 0-99: Green
#define PING_THRESHOLD_AVERAGE	251		// 100-250: Yellow, 251+: Red

vec4_t clrUiBack = { 0.f, 0.f, 0.f, .6f };
vec4_t clrUiBar = { .16f, .2f, .17f, .8f };

// Modern scoreboard colors
vec4_t clrAxisRed = { 0.8f, 0.2f, 0.2f, 1.0f };
vec4_t clrAlliesBlue = { 0.2f, 0.4f, 0.8f, 1.0f };
vec4_t clrPingGreen = { 0.2f, 0.8f, 0.2f, 1.0f };
vec4_t clrPingYellow = { 0.9f, 0.9f, 0.2f, 1.0f };
vec4_t clrPingRed = { 0.9f, 0.2f, 0.2f, 1.0f };

/*
=================
CG_DrawFlag

Draw a country flag at the specified position

All flags are stored in one single image where they are aligned
into a grid of 16x16 fields. Each flag has an id number starting
with 0 at the top left corner and ending with 255 in the bottom right
corner. Client's flag id is stored in the "u" field of configstrings.

Returns qtrue if the flag was drawn
=================
*/
static qboolean CG_DrawFlag(float x, float y, float fade, int clientNum) {
	int client_flag = atoi(Info_ValueForKey(CG_ConfigString(clientNum + CS_PLAYERS), "u"));  // uci

	if (client_flag >= 0 && client_flag < MAX_COUNTRY_NUM) {
		const int flag_size = 32;  // dimensions of a single flag
		const int all_flags = 512; // dimensions of the picture containing all flags

		float alpha[4] = { 1.f, 1.f, 1.f, fade };
		float x1       = (float)((client_flag * flag_size) % all_flags);
		float y1       = (float)(floor((client_flag * flag_size) / all_flags) * flag_size);
		float x2       = x1 + flag_size;
		float y2       = y1 + flag_size;

		trap_R_SetColor(alpha);

		CG_DrawPicST(x, y, 14, 14, x1 / all_flags, y1 / all_flags, x2 / all_flags, y2 / all_flags, cgs.media.countryFlags);

		trap_R_SetColor(NULL);
		return qtrue;
	}
	return qfalse;
}

/*
=================
CG_DrawClassIcon

Draw a class icon at the specified position
Returns qtrue if the icon was drawn
=================
*/
static qboolean CG_DrawClassIcon(float x, float y, float fade, int playerClass) {
	if (playerClass >= PC_SOLDIER && playerClass < NUM_PLAYER_CLASSES) {
		vec4_t iconColor = { 1.f, 1.f, 1.f, fade };
		trap_R_SetColor(iconColor);
		CG_DrawPic(x, y, 14, 14, cgs.media.limboClassButtons[playerClass]);
		trap_R_SetColor(NULL);
		return qtrue;
	}
	return qfalse;
}

/*
=================
WM_DrawObjectives
=================
*/

// Column widths - modified for new layout: Flag, Class, Name, K/D, XP, Ping
#define INFO_FLAG_WIDTH			16
#define INFO_PLAYER_WIDTH		160
#define INFO_SCORE_WIDTH		64
#define INFO_XP_WIDTH			32
#define INFO_KD_WIDTH			44
#define INFO_CLASS_WIDTH		16
#define INFO_LATENCY_WIDTH		36
#define INFO_TEAM_HEIGHT		24
#define INFO_BORDER				2
#define INFO_LINE_HEIGHT		30
#define INFO_TOTAL_WIDTH		(INFO_FLAG_WIDTH + INFO_CLASS_WIDTH + INFO_PLAYER_WIDTH + INFO_KD_WIDTH + INFO_XP_WIDTH + INFO_LATENCY_WIDTH)

int WM_DrawObjectives( int x, int y, int width, float fade ) {
	const char *s, *str;
	int tempy, rows;
	int msec, mins, seconds, tens; // JPW NERVE
	vec4_t tclr =	{ 0.6f,		0.6f,		0.6f,		1.0f };

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		const char *s, *buf, *shader = NULL, *flagshader = NULL, *nameshader = NULL;

		// Moved to CG_DrawIntermission
/*		static int doScreenshot = 0, doDemostop = 0;

		// OSP - End-of-level autoactions
		if(!cg.demoPlayback) {
			if(!cg.latchVictorySound) {
				if(cg_autoAction.integer & AA_SCREENSHOT) {
					doScreenshot = cg.time + 1000;
				}
				if(cg_autoAction.integer & AA_STATSDUMP) {
					CG_dumpStats_f();
				}
				if((cg_autoAction.integer & AA_DEMORECORD) && (cgs.gametype == GT_WOLF_STOPWATCH && cgs.currentRound != 1)) {
					doDemostop = cg.time + 5000;	// stats should show up within 5 seconds
				}
			}
			if(doScreenshot > 0 && doScreenshot < cg.time) {
				CG_autoScreenShot_f();
				doScreenshot = 0;
			}
			if(doDemostop > 0 && doDemostop < cg.time) {
				trap_SendConsoleCommand("stoprecord\n");
				doDemostop = 0;
			}
		}
*/
		rows = 8;
		y += SMALLCHAR_HEIGHT * ( rows - 1 );

		s = CG_ConfigString( CS_MULTI_MAPWINNER );
		buf = Info_ValueForKey( s, "winner" );

		if ( atoi( buf ) == -1 )
			str = "ITS A TIE!";
		else if ( atoi( buf ) ) {
			str = "ALLIES";
//			shader = "ui/assets/portraits/allies_win";
			flagshader = "ui/assets/portraits/allies_win_flag.tga";
			nameshader = "ui/assets/portraits/text_allies.tga";

/*			if ( !cg.latchVictorySound ) {
				cg.latchVictorySound = qtrue;
				trap_S_StartLocalSound( trap_S_RegisterSound( "sound/music/allies_win.wav", qtrue ), CHAN_LOCAL_SOUND );	// FIXME: stream
			}*/
		}
		else {
			str = "AXIS";
//			shader = "ui/assets/portraits/axis_win";
			flagshader = "ui/assets/portraits/axis_win_flag.tga";
			nameshader = "ui/assets/portraits/text_axis.tga";

/*			if ( !cg.latchVictorySound ) {
				cg.latchVictorySound = qtrue;
				trap_S_StartLocalSound( trap_S_RegisterSound( "sound/music/axis_win.wav", qtrue ), CHAN_LOCAL_SOUND );	// FIXME: stream
			}*/
		}

		y += SMALLCHAR_HEIGHT * ( ( rows - 2 ) / 2 );

		if ( flagshader ) {
			CG_DrawPic( 100 + cgs.wideXoffset, 10, 210, 136, trap_R_RegisterShaderNoMip( flagshader ) );
			CG_DrawPic( 325 + cgs.wideXoffset, 10, 210, 136, trap_R_RegisterShaderNoMip( flagshader ) );
		}

		if ( shader )
			CG_DrawPic( 229 + cgs.wideXoffset, 10, 182, 136, trap_R_RegisterShaderNoMip( shader ) );
		if ( nameshader ) {
			CG_DrawPic( 140 + cgs.wideXoffset, 50, 127, 64, trap_R_RegisterShaderNoMip( nameshader ) );
			CG_DrawPic( 365 + cgs.wideXoffset, 50, 127, 64, trap_R_RegisterShaderNoMip( "ui/assets/portraits/text_win.tga" ) );
		}
		return y;
	}
// JPW NERVE -- mission time & reinforce time
	else {
		tempy = y;
		rows = 1;
		int reinfSeconds = 0;
		int totalTimelimit = 0;

		CG_FillRect( x-5, y-2, width+5, 21, clrUiBack );
		CG_FillRect( x-5, y-2, width+5, 21, clrUiBar );
		CG_DrawRect_FixedBorder( x-5, y-2, width+5, 21, 1, colorBlack );

		y += SMALLCHAR_HEIGHT * ( rows - 1 );
		if( cgs.timelimit > 0.0f ) {
			msec = int( ( cgs.timelimit * 60.f * 1000.f ) - ( cg.time - cgs.levelStartTime ) );
			totalTimelimit = int(cgs.timelimit);

			seconds = msec / 1000;
			mins = seconds / 60;
			seconds -= mins * 60;
			tens = seconds / 10;
			seconds -= tens * 10;
		} else {
			msec = mins = tens = seconds = 0;
		}

		// Line 1: Mission time on left with total time
		if( cgs.gamestate != GS_PLAYING ) {
			s = va("^9%s ^dWARMUP", CG_TranslateString("MISSION TIME:"));
		} else if ( msec < 0 && cgs.timelimit > 0.0f ) {
			if ( cgs.gamestate == GS_WAITING_FOR_PLAYERS )
				s = va( "^9%s ^dGAME STOPPED", CG_TranslateString( "MISSION TIME:" ) );
			else
				s = va( "^9%s ^dSUDDEN DEATH", CG_TranslateString( "MISSION TIME:" ) );
		} else {
			if (totalTimelimit > 0) {
				s = va( "^9%s ^d%i:%i%i^7/^9%i:00", CG_TranslateString( "MISSION TIME:" ), mins, tens, seconds, totalTimelimit );
			} else {
				s = va( "^9%s ^d%i:%i%i", CG_TranslateString( "MISSION TIME:" ), mins, tens, seconds );
			}
		}

		CG_Text_Paint_Ext( x, y + 13, 0.2f, 0.2f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );

		// Map name in center
		{
			char mapUpper[64];
			Q_strncpyz(mapUpper, cgs.rawmapname, sizeof(mapUpper));
			Q_strupr(mapUpper);
			s = va( "^9MAP: ^d%s", mapUpper );
			int centX = (width / 2) - (CG_Text_Width_Ext( s, 0.2f, 0, &cgs.media.limboFont1 ) / 2);
			CG_Text_Paint_Ext( x + centX, y + 13, 0.2f, 0.2f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );
		}

		// Reinforce time on right side
		if( cgs.gametype != GT_WOLF_LMS ) {
			if(cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS || cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_ALLIES) {
				reinfSeconds = CG_CalculateReinfTime( qfalse );
			}

			if (reinfSeconds > 0) {
				s = va( "^9REINFORCE TIME: ^d%d", reinfSeconds );
			} else {
				s = va( "^9REINFORCE TIME: ^d--" );
			}
			CG_Text_Paint_Ext( x + width - 5 - CG_Text_Width_Ext( s, 0.2f, 0, &cgs.media.limboFont1 ), y + 13, 0.2f, 0.2f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );
		}

		// NERVE - SMF - Additional game type info
		if ( cgs.gametype == GT_WOLF_STOPWATCH ) {
			// Stopwatch round info would go here if needed
		} else if( cgs.gametype == GT_WOLF_LMS ) {
			int w;
			s = va( "%s %i  %s %i-%i", CG_TranslateString( "ROUND" ), cgs.currentRound + 1, CG_TranslateString( "SCORE" ), cg.teamWonRounds[1], cg.teamWonRounds[0] );
			w = CG_Text_Width_Ext( s, 0.2f, 0, &cgs.media.limboFont1 );

			CG_Text_Paint_Ext( x + width - 5 - w, y + 13, 0.2f, 0.2f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );
		}
		
		y += SMALLCHAR_HEIGHT * 2;
	}
// jpw

	return y;
}

static void WM_DrawClientScore( int x, int y, score_t *score, float *color, float fade ) {
	int maxchars, offset;
	int i, j;
	float tempx;
	vec4_t hcolor;
	clientInfo_t *ci;
	char buf[64];

	if ( y + SMALLCHAR_HEIGHT >= 470 )
		return;

	ci = &cgs.clientinfo[score->client];

    // Highlight background of your slot
	if ( score->client == cg.snap->ps.clientNum ) {
		tempx = x;

		hcolor[3] = fade * 0.3;
		VectorSet( hcolor, .5f, .5f, .2f );			// DARK-YELLOW

		// Flag box first
		CG_FillRect( tempx - 3, y + 1, INFO_FLAG_WIDTH - INFO_BORDER + 3, SMALLCHAR_HEIGHT - 1, hcolor );
		tempx += INFO_FLAG_WIDTH;

		// Class box 
		CG_FillRect( tempx, y + 1, INFO_CLASS_WIDTH - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor );
		tempx += INFO_CLASS_WIDTH;

        // Player box
		CG_FillRect( tempx, y + 1, INFO_PLAYER_WIDTH - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor );
		tempx += INFO_PLAYER_WIDTH;

		if ( score->ping < 0 || (ci->team == TEAM_SPECTATOR && ci->shoutcaster)) {
            // Connecting or shoutcasters get simpler row
			int width;
			width = INFO_KD_WIDTH + INFO_XP_WIDTH + INFO_LATENCY_WIDTH;

			CG_FillRect( tempx, y + 1, width - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor );
			tempx += width;
		} else {
			if( cg_gameType.integer == GT_WOLF_LMS ) {
                // LMS gets score
				CG_FillRect( tempx, y + 1, INFO_SCORE_WIDTH - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor );
				tempx += INFO_SCORE_WIDTH;
			} else {
				// K/D Box
				CG_FillRect( tempx, y + 1, INFO_KD_WIDTH - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor );
				tempx += INFO_KD_WIDTH;
                // XP Box
				CG_FillRect( tempx, y + 1, INFO_XP_WIDTH - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor );
				tempx += INFO_XP_WIDTH;				
			}

            // Ping
			CG_FillRect( tempx, y + 1, INFO_LATENCY_WIDTH - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor );
			tempx += INFO_LATENCY_WIDTH;
		}
	}

	tempx = x;

	// DHM - Nerve
	VectorSet( hcolor, 1, 1, 1 );
	hcolor[3] = fade;

	maxchars = 16;
	offset = 0;

	// Draw country flag FIRST
	if (cg_countryflags.integer && score->ping != -1 && score->ping != 999) {
		if (CG_DrawFlag(tempx + 1, y + 1, fade, score->client)) {
			// flag drawn
		}
	}
	tempx += INFO_FLAG_WIDTH;

	// Draw class icon SECOND
	if ( ci->team == TEAM_SPECTATOR) {
		// Spectators show nothing for class
	}
	else if ( cg.snap->ps.persistant[PERS_TEAM] == ci->team || CG_mvMergedClientLocate(score->client) ) {
		CG_DrawClassIcon(tempx + 1, y + 1, fade, score->playerClass);
	}
	tempx += INFO_CLASS_WIDTH;

    // Icons - draw in order: special status icons first
	if ( ci->team != TEAM_SPECTATOR ) {
        // Have the objective
		if ( ci->powerups & ( (1 << PW_REDFLAG) | (1 << PW_BLUEFLAG) ) ) {
			CG_DrawPic( tempx, y + 1, 14, 14, cgs.media.objectiveShader );
			offset += 14;
			tempx += 14;
			maxchars -= 2;
		}

		// Uniformed (don't shoot!)
		else if( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && ci->powerups & ((1 << PW_OPS_DISGUISED))) { 
			CG_DrawPic( tempx, y + 1, 14, 14, cgs.media.friendShader ); 
			offset += 14; 
			tempx += 14; 
			maxchars -= 2; 
		} 

		// Dead
		else if( score->respawnsLeft == -2 || (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && cgs.clientinfo[score->client].health == -1 ) ) {
			CG_DrawPic( tempx, y + 1, 14, 14, cgs.media.scoreEliminatedShader );
			offset += 14;
			tempx += 14;
			maxchars -= 2;

        // Need revive
		} else if( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && cgs.clientinfo[score->client].health == 0 ) {
			CG_DrawPic( tempx, y + 1, 14, 14, cgs.media.medicIcon );
			offset += 14;
			tempx += 14;
			maxchars -= 2;			
		}

        // Jaybird - muted icon
        else if (ci->muted) {
            CG_DrawPic( tempx, y + 1, 14, 14, cgs.media.mutedShader );
            offset += 14;
            tempx += 14;
            maxchars -= 2;
        }
	}

	// Draw name with SMALLCHAR (with shadow)
	CG_DrawStringExt( int(tempx), y, ci->name, hcolor, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, maxchars );
	maxchars -= CG_DrawStrlen( ci->name );

	// Draw medals
	buf[0] = '\0';
	for( i = 0; i < SK_NUM_SKILLS; i++ ) {
		for( j = 0; j < ci->medals[i]; j++ )
			Q_strcat( buf, sizeof(buf), va( "^%c%c", COLOR_RED + i, skillNames[i][0] ) );
	}
	maxchars--;
	if (maxchars > 0)
		CG_DrawStringExt( int(tempx + (BG_drawStrlen(ci->name) * SMALLCHAR_WIDTH + SMALLCHAR_WIDTH)), y, buf, hcolor, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, maxchars );

	tempx += INFO_PLAYER_WIDTH - offset;

	if ( score->ping < 0 || (ci->team == TEAM_SPECTATOR && ci->shoutcaster)) {
        // Simpler box for connecting and shoutcasters
		const char *s;
		int w, totalwidth;

		totalwidth = INFO_KD_WIDTH + INFO_XP_WIDTH + INFO_LATENCY_WIDTH - 8;

		s = CG_TranslateString( (ci->team == TEAM_SPECTATOR && ci->shoutcaster)?"^3SHOUTCASTER":"^2CONNECTING" );
		w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;

		CG_DrawSmallString( int(tempx + totalwidth - w), y, s, fade );
		return;
	}

	if( cg_gameType.integer == GT_WOLF_LMS ) {
		// LMS: Score only
		CG_DrawSmallString( int(tempx), y, va( "%5i", score->score ), fade );
		tempx += INFO_SCORE_WIDTH;
	} else {
		// K/D ratio with colored kills (green), white slash, deaths (red) - with shadow
		vec4_t killColor = { 0.2f, 0.8f, 0.2f, fade };
		vec4_t slashColor = { 1.0f, 1.0f, 1.0f, fade };
		vec4_t deathColor = { 0.9f, 0.2f, 0.2f, fade };
		char killStr[16], slashStr[16], deathStr[16];
		int killWidth, slashWidth;

		Com_sprintf(killStr, sizeof(killStr), "%i", ci->kills);
		Com_sprintf(slashStr, sizeof(slashStr), "/");
		Com_sprintf(deathStr, sizeof(deathStr), "%i", ci->deaths);
		
		CG_DrawStringExt( int(tempx), y, killStr, killColor, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
		killWidth = CG_DrawStrlen(killStr) * SMALLCHAR_WIDTH;
		CG_DrawStringExt( int(tempx + killWidth), y, slashStr, slashColor, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
		slashWidth = CG_DrawStrlen(slashStr) * SMALLCHAR_WIDTH;
		CG_DrawStringExt( int(tempx + killWidth + slashWidth), y, deathStr, deathColor, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
		tempx += INFO_KD_WIDTH;

		// XP
		CG_DrawSmallString( int(tempx), y, va( "%4i", score->score ), fade );
		tempx += INFO_XP_WIDTH;
	}

    // Ping - with color coding and BOT display (with shadow)
	if (ci->botSkill > 0) {
		// Show BOT for bots instead of ping
		vec4_t botColor = { 0.7f, 0.7f, 0.7f, fade };
		CG_DrawStringExt( int(tempx), y, " BOT", botColor, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
	} else {
		// Color-coded ping display
		vec4_t pingColor;
		if (score->ping < PING_THRESHOLD_GOOD) {
			VectorCopy(clrPingGreen, pingColor);
		} else if (score->ping < PING_THRESHOLD_AVERAGE) {
			VectorCopy(clrPingYellow, pingColor);
		} else {
			VectorCopy(clrPingRed, pingColor);
		}
		pingColor[3] = fade;
		CG_DrawStringExt( int(tempx), y, va( "%4i", score->ping ), pingColor, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
	}
	tempx += INFO_LATENCY_WIDTH;
}

const char* WM_TimeToString( float msec ) {
	int mins, seconds, tens;

	seconds = int(msec / 1000);
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	return va( "%i:%i%i", mins, tens, seconds );
}

static void WM_DrawClientScore_Small( int x, int y, score_t *score, float *color, float fade ) {
	int maxchars, offset;
	float tempx;
	vec4_t hcolor;
	clientInfo_t *ci;

	// CHRUKER: b033 - Added to draw medals
	int i, j;
	char buf[64];

	// CHRUKER: b0?? - Was using the wrong char height for this calculation
	if ( y + MINICHAR_HEIGHT >= 470 )
		return;

	ci = &cgs.clientinfo[score->client];

	if ( score->client == cg.snap->ps.clientNum ) {
		tempx = x;

		hcolor[3] = fade * 0.3;
		VectorSet( hcolor, .5f, .5f, .2f );			// DARK-YELLOW

		// Flag box first
		CG_FillRect( tempx - 3, y + 1, INFO_FLAG_WIDTH - INFO_BORDER + 3, MINICHAR_HEIGHT - 1, hcolor );
		tempx += INFO_FLAG_WIDTH;

		// Class box
		CG_FillRect( tempx, y + 1, INFO_CLASS_WIDTH - INFO_BORDER, MINICHAR_HEIGHT - 1, hcolor );
		tempx += INFO_CLASS_WIDTH;

		CG_FillRect( tempx, y + 1, INFO_PLAYER_WIDTH - INFO_BORDER, MINICHAR_HEIGHT - 1, hcolor );
		tempx += INFO_PLAYER_WIDTH;

		if ( score->ping < 0 || (ci->team == TEAM_SPECTATOR && ci->shoutcaster)) {
			int width;
			width = INFO_KD_WIDTH + INFO_XP_WIDTH + INFO_LATENCY_WIDTH;

			CG_FillRect( tempx, y + 1, width - INFO_BORDER, MINICHAR_HEIGHT - 1, hcolor );
			tempx += width;
		} else {
			if( cg_gameType.integer == GT_WOLF_LMS ) {
				CG_FillRect( tempx, y + 1, INFO_SCORE_WIDTH - INFO_BORDER, MINICHAR_HEIGHT - 1, hcolor );
				tempx += INFO_SCORE_WIDTH;
			} else {
				// K/D Box
				CG_FillRect( tempx, y + 1, INFO_KD_WIDTH - INFO_BORDER, MINICHAR_HEIGHT - 1, hcolor );
				tempx += INFO_KD_WIDTH;
				// XP Box
				CG_FillRect( tempx, y + 1, INFO_XP_WIDTH - INFO_BORDER, MINICHAR_HEIGHT - 1, hcolor );
				tempx += INFO_XP_WIDTH;				
			}

			CG_FillRect( tempx, y + 1, INFO_LATENCY_WIDTH - INFO_BORDER, MINICHAR_HEIGHT - 1, hcolor );
			tempx += INFO_LATENCY_WIDTH;
		}
	}

	tempx = x;

	// DHM - Nerve
	VectorSet( hcolor, 1, 1, 1 );
	hcolor[3] = fade;

	maxchars = 16;
	offset = 0;

	// Draw country flag FIRST
	if (cg_countryflags.integer && score->ping != -1 && score->ping != 999) {
		if (CG_DrawFlag(tempx + 1, y + 1, fade, score->client)) {
			// flag drawn
		}
	}
	tempx += INFO_FLAG_WIDTH;

	// Draw class icon SECOND
	if ( ci->team == TEAM_SPECTATOR) {
		// Spectators show nothing for class
	}
	else if ( cg.snap->ps.persistant[PERS_TEAM] == ci->team ) {
		CG_DrawClassIcon(tempx + 1, y, fade, score->playerClass);
	}
	tempx += INFO_CLASS_WIDTH;

	if ( ci->team != TEAM_SPECTATOR ) {
        // Has the objective
		if ( ci->powerups & ( (1 << PW_REDFLAG) | (1 << PW_BLUEFLAG) ) ) {
			CG_DrawPic( tempx + 1, y + 1, 10, 10, cgs.media.objectiveShader );
			offset += 12;
			tempx += 12;
			maxchars -= 2;
		}

		// forty - draw no shoot for cov-ops 
		else if( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && ci->powerups & ((1 << PW_OPS_DISGUISED))) { 
			CG_DrawPic( tempx + 1, y + 1, 10, 10, cgs.media.friendShader ); 
			offset += 12; 
			tempx += 12; 
			maxchars -= 2;
		}

		// draw the skull icon if out of lives
		else if ( score->respawnsLeft == -2 || ( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && cgs.clientinfo[score->client].health == -1 ) ) {
			CG_DrawPic( tempx, y, 10, 10, cgs.media.scoreEliminatedShader );
			offset += 12;
			tempx += 12;
			maxchars -= 2;
		}
        
        // Medic icon
        else if( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && cgs.clientinfo[score->client].health == 0 ) {
			CG_DrawPic( tempx + 1, y + 1, 10, 10, cgs.media.medicIcon );
			offset += 12;
			tempx += 12;
			maxchars -= 2;
		}

        // Muted icon
        else if (ci->muted) {
            CG_DrawPic( tempx + 1, y + 1, 10, 10, cgs.media.mutedShader );
            offset += 12;
            tempx += 12;
            maxchars -= 2;
        }
	}

	// draw name (with shadow)
	CG_DrawStringExt( int(tempx), y, ci->name, hcolor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, maxchars );

	// CHRUKER: b033 - Added to draw medals
	maxchars -= CG_DrawStrlen( ci->name );
	
	buf[0] = '\0';
	for( i = 0; i < SK_NUM_SKILLS; i++ ) {
		for( j = 0; j < ci->medals[i]; j++ )
			Q_strcat( buf, sizeof(buf), va( "^%c%c", COLOR_RED + i, skillNames[i][0] ) );
	}
	maxchars--;
	
	if (maxchars > 0)
		CG_DrawStringExt( int(tempx + (BG_drawStrlen(ci->name) * MINICHAR_WIDTH + MINICHAR_WIDTH)), y, buf, hcolor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, maxchars );
	// b033

	// Jaybird
	hcolor[0] = hcolor[1] = hcolor[2] = 1;

	tempx += INFO_PLAYER_WIDTH - offset;
	// dhm - nerve

	if ( score->ping < 0 || (ci->team == TEAM_SPECTATOR && ci->shoutcaster)) {
		const char *s;
		int w, totalwidth;

		totalwidth = INFO_KD_WIDTH + INFO_XP_WIDTH + INFO_LATENCY_WIDTH - 8;

		s = CG_TranslateString( (ci->team == TEAM_SPECTATOR && ci->shoutcaster)?"^3SHOUTCASTER":"^2CONNECTING" );
		w = CG_DrawStrlen( s ) * MINICHAR_WIDTH;

		// CHRUKER: b034 - Using the mini char height (with shadow)
		CG_DrawStringExt( int(tempx + totalwidth - w), y, s, hcolor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );

		return;
	}

	if( cg_gameType.integer == GT_WOLF_LMS ) {
		// LMS: Score only (with shadow)
		CG_DrawStringExt( int(tempx), y, va( "%5i", score->score ), hcolor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		tempx += INFO_SCORE_WIDTH;
	} else {
		// K/D ratio with colored kills (green), white slash, deaths (red) - with shadow
		vec4_t killColor = { 0.2f, 0.8f, 0.2f, fade };
		vec4_t slashColor = { 1.0f, 1.0f, 1.0f, fade };
		vec4_t deathColor = { 0.9f, 0.2f, 0.2f, fade };
		char killStr[16], slashStr[16], deathStr[16];
		int killWidth, slashWidth;

		Com_sprintf(killStr, sizeof(killStr), "%i", ci->kills);
		Com_sprintf(slashStr, sizeof(slashStr), "/");
		Com_sprintf(deathStr, sizeof(deathStr), "%i", ci->deaths);
		
		CG_DrawStringExt( int(tempx), y, killStr, killColor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		killWidth = CG_DrawStrlen(killStr) * MINICHAR_WIDTH;
		CG_DrawStringExt( int(tempx + killWidth), y, slashStr, slashColor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		slashWidth = CG_DrawStrlen(slashStr) * MINICHAR_WIDTH;
		CG_DrawStringExt( int(tempx + killWidth + slashWidth), y, deathStr, deathColor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		tempx += INFO_KD_WIDTH;

		// XP (with shadow)
		CG_DrawStringExt( int(tempx), y, va( "%4i", score->score ), hcolor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		tempx += INFO_XP_WIDTH;
	}

	// Ping - with color coding and BOT display (with shadow)
	if (ci->botSkill > 0) {
		// Show BOT for bots instead of ping
		vec4_t botColor = { 0.7f, 0.7f, 0.7f, fade };
		CG_DrawStringExt( int(tempx), y, " BOT", botColor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
	} else {
		// Color-coded ping display
		vec4_t pingColor;
		if (score->ping < PING_THRESHOLD_GOOD) {
			VectorCopy(clrPingGreen, pingColor);
		} else if (score->ping < PING_THRESHOLD_AVERAGE) {
			VectorCopy(clrPingYellow, pingColor);
		} else {
			VectorCopy(clrPingRed, pingColor);
		}
		pingColor[3] = fade;
		CG_DrawStringExt( int(tempx), y, va( "%4i", score->ping ), pingColor, qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
	}
	tempx += INFO_LATENCY_WIDTH;
}

static int WM_DrawInfoLine( int x, int y, float fade ) {
	int w, defender, winner;
	const char *s;
	vec4_t tclr =	{ 0.6f,		0.6f,		0.6f,		1.0f };

	if ( cg.snap->ps.pm_type != PM_INTERMISSION ) {
		return y;
	}

	w = 360;
//	CG_DrawPic( 320 - w/2, y, w, INFO_LINE_HEIGHT, trap_R_RegisterShaderNoMip( "ui/assets/mp_line_strip.tga" ) );

	s = CG_ConfigString( CS_MULTI_INFO );
	defender = atoi( Info_ValueForKey( s, "defender" ) );

	s = CG_ConfigString( CS_MULTI_MAPWINNER );
	winner = atoi( Info_ValueForKey( s, "winner" ) );

	if ( cgs.currentRound ) {
		// first round
		s = va( CG_TranslateString( "CLOCK IS NOW SET TO %s!" ), WM_TimeToString( cgs.nextTimeLimit * 60.f * 1000.f ) );
	}
	else {
		// second round
		if ( !defender ) {
			if ( winner != defender )
				s = "ALLIES SUCCESSFULLY BEAT THE CLOCK!";
			else
				s = "ALLIES COULDN'T BEAT THE CLOCK!";
		}
		else {
			if ( winner != defender )
				s = "AXIS SUCCESSFULLY BEAT THE CLOCK!";
			else
				s = "AXIS COULDN'T BEAT THE CLOCK!";
		}

		s = CG_TranslateString( s );
	}

	CG_FillRect( 320 - w/2, y, w, 20, clrUiBar );
	CG_DrawRect_FixedBorder( 320 - w/2, y, w, 20, 1, colorBlack );

	w = CG_Text_Width_Ext( s, 0.25f, 0, &cgs.media.limboFont1 );

	CG_Text_Paint_Ext( 320 - w*0.5f, y + 15, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );
//	CG_DrawSmallString( 320 - w/2, ( y + INFO_LINE_HEIGHT / 2 ) - SMALLCHAR_HEIGHT / 2, s, fade );
	return y + INFO_LINE_HEIGHT + 6;
}

// CHRUKER: b035 - Added absolute maximum rows
static int WM_TeamScoreboard( int x, int y, team_t team, float fade, int maxrows, int absmaxrows ) {
	vec4_t hcolor;
	float tempx, tempy;
	int width;
	int i;
	int count = 0;
	qboolean use_mini_chars = qfalse; // CHRUKER: b035 - Needed to check if using mini chars
	vec4_t tclr = { 0.6f, 0.6f, 0.6f, 1.0f };
	float avgPing = 0;
	float pingStdDev = 0;
	int numPings = 0;

	width = INFO_FLAG_WIDTH + INFO_CLASS_WIDTH + INFO_PLAYER_WIDTH + INFO_KD_WIDTH + INFO_XP_WIDTH + INFO_LATENCY_WIDTH;

	// Calculate average ping
	for( i = 0; i < cg.numScores; i++ ) {
		if( team != cgs.clientinfo[ cg.scores[i].client ].team ) {
			continue;
		}
		if( cg.scores[i].ping > 0 && cg.scores[i].ping < 999 ) {
			avgPing += cg.scores[i].ping;
			numPings++;
		}
	}
	if (numPings > 0) {
		avgPing = avgPing / numPings;
		// Calculate standard deviation
		float sumSquares = 0;
		for( i = 0; i < cg.numScores; i++ ) {
			if( team != cgs.clientinfo[ cg.scores[i].client ].team ) {
				continue;
			}
			if( cg.scores[i].ping > 0 && cg.scores[i].ping < 999 ) {
				float diff = cg.scores[i].ping - avgPing;
				sumSquares += diff * diff;
			}
		}
		pingStdDev = sqrt(sumSquares / numPings);
	}

	// Jaybird - 10 px change
	CG_FillRect( x-5, y-12, width+5, 31, clrUiBack );
	CG_FillRect( x-5, y-12, width+5, 31, clrUiBar );	
	
	Vector4Set( hcolor, 0, 0, 0, fade );
	CG_DrawRect_FixedBorder( x-5, y-12, width+5, 31, 1, colorBlack );

	// draw header with team-colored text and avg ping with standard deviation
	if( cg_gameType.integer == GT_WOLF_LMS ) {
		char *s;
		if ( team == TEAM_AXIS ) {
			s = va( "^1%s [%d] (%d %s)", CG_TranslateString( "AXIS" ), cg.teamScores[0], cg.teamPlayers[team], CG_TranslateString("PLAYERS") );
			s = va( "%s ^3%s", s, cg.teamFirstBlood == TEAM_AXIS ? CG_TranslateString("FIRST BLOOD") : "" );

			CG_Text_Paint_Ext( x, y + 13, 0.2f, 0.2f, clrAxisRed, s, 0, 0, 0, &cgs.media.limboFont1 );
		} else if ( team == TEAM_ALLIES ) {
			s = va( "^d%s [%d] (%d %s)", CG_TranslateString( "ALLIES" ), cg.teamScores[1], cg.teamPlayers[team], CG_TranslateString("PLAYERS") );
			s = va( "%s ^3%s", s, cg.teamFirstBlood == TEAM_ALLIES ? CG_TranslateString("FIRST BLOOD") : "" );

			CG_Text_Paint_Ext( x, y + 13, 0.2f, 0.2f, clrAlliesBlue, s, 0, 0, 0, &cgs.media.limboFont1 );
		}
	} else {
		const char *avgStr;
		const char *teamStr;
		if ( team == TEAM_AXIS ) {
			teamStr = va( "^1%s [%d] (%d %s)", CG_TranslateString( "AXIS" ), cg.teamScores[0], cg.teamPlayers[team], CG_TranslateString("PLAYERS") );
			CG_Text_Paint_Ext( x, y + 13, 0.2f, 0.2f, clrAxisRed, teamStr, 0, 0, 0, &cgs.media.limboFont1 );
		} else if ( team == TEAM_ALLIES ) {
			teamStr = va( "^d%s [%d] (%d %s)", CG_TranslateString( "ALLIES" ), cg.teamScores[1], cg.teamPlayers[team], CG_TranslateString("PLAYERS") );
			CG_Text_Paint_Ext( x, y + 13, 0.2f, 0.2f, clrAlliesBlue, teamStr, 0, 0, 0, &cgs.media.limboFont1 );
		}
		// AVG Ping with ± standard deviation on the right
		avgStr = va( "^9AVG Ping: %.0f±%.0fms", avgPing, pingStdDev );
		CG_Text_Paint_Ext( x + width - 5 - CG_Text_Width_Ext( avgStr, 0.18f, 0, &cgs.media.limboFont1 ), y + 13, 0.18f, 0.18f, tclr, avgStr, 0, 0, 0, &cgs.media.limboFont1 );
	}

	y += SMALLCHAR_HEIGHT + 3;

	tempx = x;

	// CHRUKER: b076 - Adjusted y coordinate, and changed to use DrawBottom instead of DrawTopBottom
	CG_FillRect( x-5, y, width+5, 18, clrUiBack );
	trap_R_SetColor( colorBlack );
	CG_DrawBottom_NoScale( x-5, y, width+5, 18, 1 );
	trap_R_SetColor( NULL );

	// draw player info headings - new order: Flag, Class, Name, K/D, XP, Ping
	// Skip flag column header (just space)
	tempx += INFO_FLAG_WIDTH;

	// Class icon header (small C for class)
	CG_DrawSmallString( int(tempx), y, "C", fade );
	tempx += INFO_CLASS_WIDTH;

	CG_DrawSmallString( int(tempx), y, CG_TranslateString( "Name" ), fade );
	tempx += INFO_PLAYER_WIDTH;

	if( cgs.gametype == GT_WOLF_LMS ) {
		CG_DrawSmallString( int(tempx), y, CG_TranslateString( "Score" ), fade );
		tempx += INFO_SCORE_WIDTH;
	} else {
		// K/D header
		CG_DrawSmallString( int(tempx), y, "K/D", fade );
		tempx += INFO_KD_WIDTH;

		// XP header
		CG_DrawSmallString( int(tempx), y, CG_TranslateString( "XP" ), fade );
		tempx += INFO_XP_WIDTH;
	}

	CG_DrawSmallString( int(tempx), y, CG_TranslateString( "Ping" ), fade );
	tempx += INFO_LATENCY_WIDTH;
	
	// CHRUKER: b076 - The math says char height + 2 * border width (1 pixel)
	y += SMALLCHAR_HEIGHT + 2;

	cg.teamPlayers[team] = 0; // JPW NERVE
	for ( i = 0; i < cg.numScores; i++ ) {
		if ( team != cgs.clientinfo[ cg.scores[i].client ].team )
			continue;

		cg.teamPlayers[team]++;
	}

	// CHRUKER: b035 - Adjust maxrows
	if ( cg.teamPlayers[team] > maxrows ) {
		maxrows = absmaxrows;
		use_mini_chars = qtrue;
	}
	
	// save off y val
	tempy = y;
	
	// draw color bands - team-specific colors (darker colors)
	for ( i = 0; i < maxrows; i++ ) {
		if ( team == TEAM_ALLIES ) {
			// Allies: darker blue shades alternating
			if ( i % 2 == 0 )
				VectorSet( hcolor, (30.f/255.f), (50.f/255.f), (80.f/255.f) );   // Dark blue
			else
				VectorSet( hcolor, (20.f/255.f), (35.f/255.f), (60.f/255.f) );   // Darker blue
		} else {
			// Axis: darker red shades alternating
			if ( i % 2 == 0 )
				VectorSet( hcolor, (80.f/255.f), (30.f/255.f), (30.f/255.f) );   // Dark red
			else
				VectorSet( hcolor, (60.f/255.f), (20.f/255.f), (20.f/255.f) );   // Darker red
		}
		hcolor[3] = fade * 0.6;
		
		if ( use_mini_chars ) {
			// CHRUKER: b076 - Adjusted y height, and changed to DrawBottom instead of DrawTopBottom
			CG_FillRect( x-5, y, width+5, MINICHAR_HEIGHT, hcolor );
			trap_R_SetColor( colorBlack );
			CG_DrawBottom_NoScale( x-5, y, width+5, MINICHAR_HEIGHT, 1 );
			trap_R_SetColor( NULL );
			y += MINICHAR_HEIGHT;
		} else {
			// CHRUKER: b076 - Adjusted y height, and changed to DrawBottom instead of DrawTopBottom
			CG_FillRect( x-5, y, width+5, SMALLCHAR_HEIGHT, hcolor );
			trap_R_SetColor( colorBlack );
			CG_DrawBottom_NoScale( x-5, y, width+5, SMALLCHAR_HEIGHT, 1 );
			trap_R_SetColor( NULL );
			y += SMALLCHAR_HEIGHT;
		}
	}
	hcolor[3] = 1;
	y = int(tempy) - 1;
	
	// draw player info
	VectorSet( hcolor, 1, 1, 1 );
	hcolor[3] = fade;

	count = 0;
	for( i = 0; i < cg.numScores && count < maxrows; i++ ) {
		if( team != cgs.clientinfo[ cg.scores[i].client ].team ) {
			continue;
		}

		// CHRUKER: b035 - Using the flag instead
		if( use_mini_chars ) {
			WM_DrawClientScore_Small( x, y, &cg.scores[i], hcolor, fade );
			y += MINICHAR_HEIGHT;
		} else {
			WM_DrawClientScore( x, y, &cg.scores[i], hcolor, fade );
			y += SMALLCHAR_HEIGHT;
		}

		count++;
	}

	// draw spectators
	// CHRUKER: b035 - Missing support for mini char height scoreboard background
	if ( use_mini_chars )
		y += MINICHAR_HEIGHT;
	else
		y += SMALLCHAR_HEIGHT;

	for ( i = 0; i < cg.numScores; i++ ) {
		if ( cgs.clientinfo[ cg.scores[i].client ].team != TEAM_SPECTATOR )
			continue;
		if ( team == TEAM_AXIS && ( i % 2 ) )
			continue;
		if ( team == TEAM_ALLIES && ( ( i + 1 ) % 2 ) )
			continue;

		// CHRUKER: b034 - Missing support for minichars; b035 - Using the flag instead
		if( use_mini_chars ) {
			WM_DrawClientScore_Small( x, y, &cg.scores[i], hcolor, fade );
			y += MINICHAR_HEIGHT;
		} else {
			WM_DrawClientScore( x, y, &cg.scores[i], hcolor, fade );
			y += SMALLCHAR_HEIGHT;
		}
	}

	return y;
}
// -NERVE - SMF

/*
=================
CG_DrawScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawScoreboard( void ) {
	int		x = 0, y = 0, x_right;
	float	fade;
	float	*fadeColor;
	int		width;  // scoreboard width based on 640 virtual screen
	int		gap = 40;  // Gap between teams
	vec4_t	bgColor = { 0.0f, 0.0f, 0.0f, 0.7f };  // Dark background
	vec4_t	borderColor = { 0.3f, 0.3f, 0.3f, 0.8f };  // Gray border

	// Calculate x positions to center both teams with ~40px gap between them
	// Total width needed = 2 * INFO_TOTAL_WIDTH + gap
	// Center point = 320, each team starts at center - gap/2 - team_width (for left) or center + gap/2 (for right)
	x = (640 - (2 * INFO_TOTAL_WIDTH + gap)) / 2;
	y = 10;
	x_right = x + INFO_TOTAL_WIDTH + gap;
	width = 2 * INFO_TOTAL_WIDTH + gap;
	
	// Add widescreen offset to both x positions
	x += cgs.wideXoffset;
	x_right += cgs.wideXoffset;

	// don't draw anything if the menu or console is up
	if ( cg_paused.integer ) {
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	// OSP - also for pesky scoreboards in demos
	if ((cgs.gamestate == GS_WARMUP || (cg.demoPlayback && cg.snap->ps.pm_type != PM_INTERMISSION)) && !cg.showScores) {
		return qfalse;
	}

	// don't draw if in cameramode
	if( cg.cameraMode ) {
		return qtrue;
	}

	if( cg.showScores || cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		fade = 1.0;
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor( cg.scoreFadeTime, FADE_TIME );
		
		if( !fadeColor ) {
			// next time scoreboard comes up, don't print killer
			*cg.killerName = 0;
			return qfalse;
		}
 		fade = fadeColor[3];
	}

	// Draw dark background with frame around entire scoreboard
	bgColor[3] = 0.7f * fade;
	borderColor[3] = 0.8f * fade;
	CG_FillRect( x - 10, y - 5, width + 15, 470, bgColor );
	CG_DrawRect_FixedBorder( x - 10, y - 5, width + 15, 470, 2, borderColor );

	y = WM_DrawObjectives( x, y, width, fade );

	if ( cgs.gametype == GT_WOLF_STOPWATCH && ( cg.snap->ps.pm_type == PM_INTERMISSION ) ) {
		y = WM_DrawInfoLine( x, 155, fade );

		// CHRUKER: b035 - The maxrows has been split into one for when to use the mini chars and one for when to stop writing.
		WM_TeamScoreboard( x, y, TEAM_AXIS, fade, 8, 10 );
		WM_TeamScoreboard( x_right, y, TEAM_ALLIES, fade, 8, 10 );
	} else {
		if(cg.snap->ps.pm_type == PM_INTERMISSION) {
			WM_TeamScoreboard( x, y, TEAM_AXIS, fade, 9, 12 );
			WM_TeamScoreboard( x_right, y, TEAM_ALLIES, fade, 9, 12 );
		} else {
			WM_TeamScoreboard( x, y, TEAM_AXIS, fade, 25, 33 );
			WM_TeamScoreboard( x_right, y, TEAM_ALLIES, fade, 25, 33 );
		} // b035
	}

	return qtrue;
}
