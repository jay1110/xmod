#ifndef GAME_MAPRECORDS_H
#define GAME_MAPRECORDS_H

extern vmCvar_t g_mapRecords;

void G_InitMapRecords();
void G_TrackMapRecords(gentity_t* ent);
void G_SaveMapRecords(qboolean announce);
// A negative slot prints to the server console, not to every player.
void G_PrintMapRecords(int clientNum);

#endif
