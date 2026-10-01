#include <bgame/impl.h>
#include <game/g_maprecords.h>
#include <game/xmod_globals.h>

namespace {
xmod::MapRecords roundRecords;

bool DatabaseReady() {
    return xmod::g_database && xmod::g_database->isOpened();
}

std::string SafeRecordText(const std::string& input) {
    std::string output;
    for (unsigned char c : input) {
        if (c >= 32 && c != '"' && c != '\\' && c != 127) output += c;
        if (output.size() == 255) break;
    }
    return output;
}

void PrintLine(int clientNum, const char* message) {
    const std::string safe = SafeRecordText(message);
    if (clientNum < 0) G_Printf("%s\n", safe.c_str());
    else trap_SendServerCommand(clientNum, va("print \"%s\n\"", safe.c_str()));
}

void RecordLine(int clientNum, qboolean announce, const char* label, int value,
                const std::string& player, time_t timestamp, bool improved) {
    char date[32] = "date unknown";
    if (timestamp > 0) {
        const tm* when = localtime(&timestamp);
        if (when) strftime(date, sizeof(date), "%Y-%m-%d %H:%M", when);
    }
    const std::string safePlayer = SafeRecordText(player);
    const char* message = value > 0 ?
        va("^3%sMap %s record: ^7%i ^3by ^7%s ^3(%s)", improved ? "New " : "", label, value, safePlayer.c_str(), date) :
        va("^3Map %s record: ^7none yet", label);
    if (announce) trap_SendServerCommand(-1, va("chat \"%s\"", message));
    else PrintLine(clientNum, message);
}
}

void G_InitMapRecords() {
    // This must also run when the VM stays loaded across a map restart.
    roundRecords = xmod::MapRecords();
    if (!g_mapRecords.integer || !DatabaseReady() || !*level.rawmapname) return;

    // Preserve existing map.db spree records when enabling the SQLite feature.
    if (currentMap && currentMap != &MapRecord::BAD && currentMap->longestSpree > 0) {
        xmod::MapRecords stored;
        if (!xmod::g_database->getMapRecords(level.rawmapname, stored)) {
            G_Printf("[MapRecords] Cannot read records for %s\n", level.rawmapname);
            return;
        }
        if (currentMap->longestSpree > stored.spreeRecord &&
            !xmod::g_database->updateMapSpreeRecord(level.rawmapname, currentMap->longestSpree,
                currentMap->longestSpreeNamex, currentMap->longestSpreeTime)) {
            G_Printf("[MapRecords] Cannot migrate the spree record for %s\n", level.rawmapname);
        }
    }
}

void G_TrackMapRecords(gentity_t* ent) {
    if (!g_mapRecords.integer || cvars::gameState.ivalue != GS_PLAYING ||
        !ent || !ent->client || ent->client->pers.connected != CON_CONNECTED ||
        (ent->client->sess.sessionTeam != TEAM_AXIS && ent->client->sess.sessionTeam != TEAM_ALLIES)) return;

    const gclient_t* cl = ent->client;
    // Keep the winners independently of client slots so disconnecting, joining
    // spectators or slot reuse cannot erase another player's round result.
    if (cl->pers.killspreekills > roundRecords.spreeRecord) {
        roundRecords.spreeRecord = cl->pers.killspreekills;
        roundRecords.spreePlayer = cl->pers.netname;
        roundRecords.spreeDate = time(NULL);
    }
    if (cl->sess.kills > roundRecords.fragRecord) {
        roundRecords.fragRecord = cl->sess.kills;
        roundRecords.fragPlayer = cl->pers.netname;
        roundRecords.fragDate = time(NULL);
    }
}

void G_SaveMapRecords(qboolean announce) {
    if (!g_mapRecords.integer || !DatabaseReady() || !*level.rawmapname) return;
    int changed;
    if (!xmod::g_database->updateMapRecords(level.rawmapname, roundRecords, changed)) {
        G_Printf("[MapRecords] Records for %s could not be saved\n", level.rawmapname);
        return;
    }
    if (!announce) return;
    xmod::MapRecords stored;
    if (!xmod::g_database->getMapRecords(level.rawmapname, stored)) return;
    if (stored.spreeRecord > 0)
        RecordLine(-1, qtrue, "Spree", stored.spreeRecord, stored.spreePlayer, stored.spreeDate, (changed & 1) != 0);
    if (stored.fragRecord > 0)
        RecordLine(-1, qtrue, "Frag", stored.fragRecord, stored.fragPlayer, stored.fragDate, (changed & 2) != 0);
}

void G_PrintMapRecords(int clientNum) {
    if (!g_mapRecords.integer) {
        PrintLine(clientNum, "^3Map records are disabled (g_mapRecords 0).");
        return;
    }
    xmod::MapRecords stored;
    if (!DatabaseReady() || !xmod::g_database->getMapRecords(level.rawmapname, stored)) {
        PrintLine(clientNum, "^1Map records are unavailable: a valid SQLite database is required.");
        return;
    }
    PrintLine(clientNum, va("^3Records for ^7%s", level.rawmapname));
    RecordLine(clientNum, qfalse, "Spree", stored.spreeRecord, stored.spreePlayer, stored.spreeDate, false);
    RecordLine(clientNum, qfalse, "Frag", stored.fragRecord, stored.fragPlayer, stored.fragDate, false);
}
