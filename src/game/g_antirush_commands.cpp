#include <bgame/impl.h>
#undef min
#undef max
#include <algorithm>
#include <bgame/reliable_budget.h>
#include <bgame/numeric_text.h>
#include "g_antirush.h"
#include <cstdlib>
#include <cmath>
#include <sstream>

namespace antirush {
namespace {
struct ReplyState {
    std::list<std::string> lines;
    std::vector<std::string> guidIds;
    XmodReliableBudget budget;
    int requiredAccess = 0; // 0: public errors, 1: editor, 2: administrator, 3: both
};
ReplyState replies[MAX_CLIENTS + 1]; // last slot belongs to the server console

int index(int client) { return client < 0 ? MAX_CLIENTS : client; }
std::string arg(int n) {
    char text[MAX_STRING_CHARS];
    trap_Argv(n, text, sizeof(text));
    return text;
}
std::string lower(std::string text) {
    for (char& ch : text) if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    return text;
}
std::string printable(const std::string& input) {
    std::string text = sanitizeText(input);
    // Engine server commands have no portable quote escaping convention.
    for (char& ch : text) if (ch == '"' || ch == '\\') ch = '\'';
    return text.substr(0, 700);
}
void reply(int client, const std::string& text) {
    if (client < 0) { G_Printf("%s\n", printable(text).c_str()); return; }
    auto& lines = replies[index(client)].lines;
    if (lines.size() < 256) lines.push_back(printable(text));
}
bool integer(const std::string& text, int& value, int maximum) {
    if (text.empty() || text.size() > 9) return false;
    unsigned n = 0;
    for (char ch : text) {
        if (ch < '0' || ch > '9') return false;
        n = n * 10 + ch - '0';
        if (n > static_cast<unsigned>(maximum)) return false;
    }
    value = static_cast<int>(n);
    return true;
}
bool distanceValue(const std::string& text, float& value) {
    double parsed;
    if (!XmodParseFiniteDecimal(text.c_str(), parsed) || parsed < 0 || parsed > 100000) return false;
    value = static_cast<float>(parsed);
    return true;
}
bool admin(int client) {
    return canManage(client);
}
bool authorized(int client) {
    return canEdit(client);
}
bool denies(const PrivilegeSet& set, const Privilege& privilege) {
    return set.contains(privilege) || set.contains(priv::pseudo::all) || set.contains(priv::pseudo::commands);
}
bool explicitlyDenied(const User& user, const Privilege& privilege) {
    if (user.privDenied && denies(*user.privDenied, privilege)) return true;
    std::string error;
    const Level& adminLevel = levelDB.fetchByKey(user.authLevel, error);
    return adminLevel != Level::BAD && denies(adminLevel.privDenied, privilege);
}
std::string playerName(int client) {
    return sanitizeText(g_entities[client].client->pers.netname);
}
std::vector<size_t> pointIds() {
    std::vector<size_t> ids;
    for (int kind = 0; kind < 2; ++kind)
        for (size_t i = 0; i < state.points.size(); ++i)
            if (state.points[i].trickplant == (kind != 0)) ids.push_back(i);
    return ids;
}
bool pointId(int client, const std::string& text, size_t& result) {
    auto ids = pointIds();
    int id;
    if (!integer(text, id, static_cast<int>(ids.size())) || id == 0) {
        reply(client, "^1Invalid saved point ID. Use /aa list.");
        return false;
    }
    result = ids[id - 1];
    return true;
}
bool candidate(const gentity_t& ent, bool trick) {
    if (!ent.inuse || !ent.classname) return false;
    const std::string classname = lower(ent.classname);
    if (trick) return classname.find("dynamite") != std::string::npos;
    return classname == "team_wolf_objective" || classname == "team_ctf_redflag" ||
           classname == "team_ctf_blueflag" || classname == "trigger_objective_info" || classname == "func_explosive";
}
bool nearby(int client, const vec3_t position, float range) {
    return client < 0 || DistanceSquared(g_entities[client].client->ps.origin, position) <= range * range;
}
std::string describePoint(const Point& point) {
    std::ostringstream out;
    out << (point.trickplant ? "trickplant" : "antirush") << " name='" << point.name << "' position="
        << point.origin[0] << ',' << point.origin[1] << ',' << point.origin[2]
        << " radius=" << point.radius << " covert=" << point.covertRadius << " engineer=" << point.engineerRadius;
    return out.str();
}
std::string fullCommand(const std::vector<std::string>& args) {
    std::string command = "aa";
    for (const auto& value : args) command += " " + value;
    return command;
}
void pointSummary(int client, bool showDistance) {
    const auto ids = pointIds();
    reply(client, "^3Saved points for " + state.mapName + " (IDs apply to /aa edit and /aa remove):");
    if (ids.empty()) reply(client, "No configured protection points on this map.");
    for (size_t i = 0; i < ids.size(); ++i) {
        const Point& p = state.points[ids[i]];
        std::ostringstream out;
        out << i + 1 << ": " << (p.trickplant ? "trickplant" : p.name) << " radius=" << p.radius;
        if (!p.trickplant) out << " covert=" << p.covertRadius << " engineer=" << p.engineerRadius;
        if (showDistance && client >= 0) out << " distance=" << Distance(p.origin, g_entities[client].client->ps.origin);
        out << " (" << p.origin[0] << ' ' << p.origin[1] << ' ' << p.origin[2] << ')';
        reply(client, out.str());
    }
    if (ids.size() > 254) reply(client, "Point list exceeds the output limit; inspect the map configuration file.");
}
void scanOrSave(int client, bool trick, const std::vector<std::string>& args) {
    float range = trick ? state.settings.trickplantRange : state.settings.antirushRange;
    if (args.size() == 1) {
        reply(client, "^3Nearby " + std::string(trick ? "dynamite" : "objectives") + " (entity IDs):");
        int count = 0;
        for (int i = MAX_CLIENTS; i < level.num_entities; ++i) {
            vec3_t position;
            if (!candidate(g_entities[i], trick) || !entityPosition(&g_entities[i], position, !trick) || !nearby(client, position, range)) continue;
            std::ostringstream out;
            out << i << ": " << entityName(&g_entities[i]);
            if (client >= 0) out << " distance=" << Distance(g_entities[client].client->ps.origin, position);
            reply(client, out.str());
            if (++count == 250) { reply(client, "Output limited to 250 entities."); break; }
        }
        if (!count) reply(client, "None in scan range.");
        return;
    }
    int id;
    vec3_t position;
    if (args.size() != 2 || !integer(args[1], id, level.num_entities - 1) || id < MAX_CLIENTS ||
        !candidate(g_entities[id], trick) || !entityPosition(&g_entities[id], position, !trick) || !nearby(client, position, range)) {
        reply(client, "^1Invalid or out-of-range entity. Scan again with /aa " + args[0] + '.');
        return;
    }
    Point point;
    point.trickplant = trick;
    point.name = trick ? "" : sanitizeText(entityName(&g_entities[id]));
    point.radius = range;
    VectorCopy(position, point.origin);
    for (const auto& old : state.points) {
        if (old.trickplant == trick && old.name == point.name && DistanceSquared(old.origin, position) < 1) {
            reply(client, "This point is already saved. Use /aa edit to change its radius.");
            return;
        }
    }
    auto points = state.points;
    points.push_back(point);
    if (!savePoints(points)) { reply(client, "^1Could not save the map file; protection unchanged."); return; }
    changed();
    logChange(client, fullCommand(args), "added " + describePoint(point));
    reply(client, "^2Protection point saved.");
    if (!trick) announce("OBJECTIVE_SAVED", {{"objective", point.name}}, true);
}
void guidCommands(int client, const std::vector<std::string>& args) {
    if (!admin(client)) { reply(client, "^1This command also requires C/antirush access."); return; }
    if (args[0] == "addguid") {
        int slot;
        std::string guid;
        if (args.size() != 2 || !integer(args[1], slot, level.maxclients - 1) || !authenticated(slot, &guid)) {
            reply(client, "^1Usage: /aa addguid <connected, authenticated human slot>."); return;
        }
        for (const auto& entry : state.guids) if (entry.guid == guid) { reply(client, "This GUID is already authorized."); return; }
        auto entries = state.guids;
        entries.push_back({guid, playerName(slot)});
        if (!saveGuids(entries)) { reply(client, "^1Could not save GUIDs; permissions unchanged."); return; }
        logChange(client, "aa addguid", guid + " " + playerName(slot));
        reply(client, "^2Antirush editing enabled for " + playerName(slot));
        return;
    }
    auto& snapshot = replies[index(client)].guidIds;
    if (args.size() == 1) {
        snapshot.clear();
        reply(client, "^3Authorized GUIDs (use /aa removeguid <ID> from this list):");
        for (const auto& entry : state.guids) {
            snapshot.push_back(entry.guid);
            std::string name = entry.name;
            for (int slot = 0; slot < level.maxclients; ++slot) {
                std::string guid;
                if (authenticated(slot, &guid) && guid == entry.guid) { name = playerName(slot) + " (online)"; break; }
            }
            reply(client, std::to_string(snapshot.size()) + ": " + name + " " + entry.guid);
        }
        if (snapshot.empty()) reply(client, "No authorized GUID entries.");
        return;
    }
    int id;
    if (args.size() != 2 || !integer(args[1], id, static_cast<int>(snapshot.size())) || id == 0) {
        reply(client, "^1Invalid list ID. Run /aa removeguid first."); return;
    }
    auto entries = state.guids;
    auto found = std::find_if(entries.begin(), entries.end(), [&](const GuidEntry& entry) { return entry.guid == snapshot[id - 1]; });
    if (found == entries.end()) { reply(client, "This GUID was already removed. Refresh with /aa removeguid."); return; }
    const std::string detail = found->guid + " " + found->name;
    entries.erase(found);
    if (!saveGuids(entries)) { reply(client, "^1Could not save GUIDs; permissions unchanged."); return; }
    logChange(client, "aa removeguid", detail);
    reply(client, "^2Antirush GUID permission removed.");
}
void modeCommand(int client, const std::vector<std::string>& args) {
    if (!admin(client)) { reply(client, "^1This command requires C/antirush access."); return; }
    if (args.empty() || (args.size() == 1 && lower(args[0]) == "status")) {
        reply(client, "Antirush: g_antirush=" + std::to_string(g_antirush.integer) +
            " native mode=" + (state.mode ? "on" : "off") + " map=" + state.mapName +
            " remaining=" + std::to_string(active() ? remainingMs() / 1000 : 0) + "s");
        return;
    }
    const std::string mode = lower(args[0]);
    if (args.size() != 1 || (mode != "on" && mode != "off")) { reply(client, "Usage: !antirush on|off"); return; }
    if (!saveMode(mode == "on")) { reply(client, "^1Could not save mode; protection unchanged."); return; }
    trap_Cvar_Set("g_antirush", "2");
    trap_Cvar_Update(&g_antirush);
    changed();
    logChange(client, "antirush", mode);
    reply(client, "^2Antirush mode saved: " + mode);
}
void execute(int client, std::vector<std::string> args, bool mode) {
    // A new request replaces pending menu output, never appending unbounded data.
    replies[index(client)].lines.clear();
    replies[index(client)].requiredAccess = 0;
    if (mode) {
        if (admin(client)) replies[index(client)].requiredAccess = 2;
        modeCommand(client, args); return;
    }
    if (g_antirush.integer != 2) { reply(client, "Native Antirush requires g_antirush 2."); return; }
    if (!authorized(client)) { reply(client, "^1Antirush: authenticated GUID or configured admin/user level required."); return; }
    replies[index(client)].requiredAccess = 1;
    if (args.empty()) args.push_back("help");
    args[0] = lower(args[0]);
    const std::string& cmd = args[0];
    if (cmd == "help") {
        reply(client, "^3Xmod Antirush / AutoAdmin commands:");
        reply(client, "/aa info | list | distance");
        reply(client, "/aa antirush [entity ID] | trickplant [entity ID]");
        reply(client, "/aa edit <point ID> <radius or name>");
        reply(client, "/aa edit <point ID> c|e <charge drain radius; 0 disables>");
        reply(client, "/aa remove <point ID>");
        reply(client, "/aa addguid <player slot> | removeguid [list ID] (also C/antirush)");
        reply(client, "!antirush status|on|off (C/antirush; console: antirush status|on|off)");
    } else if (cmd == "info") {
        reply(client, "^3Native Antirush: map=" + state.mapName + " mode=" + (state.mode ? "on" : "off") +
            " remaining=" + std::to_string(remainingMs() / 1000) + "s");
        reply(client, "Protect fraction=" + std::to_string(state.settings.protectFraction) + "; edits save to antirush/maps/" + state.mapName + ".cfg");
        reply(client, "Configured maps:");
        for (const auto& map : configuredMaps()) reply(client, map);
    } else if (cmd == "list" || cmd == "distance") {
        pointSummary(client, cmd == "distance");
    } else if (cmd == "antirush" || cmd == "trickplant") {
        scanOrSave(client, cmd == "trickplant", args);
    } else if (cmd == "addguid" || cmd == "removeguid") {
        if (admin(client)) replies[index(client)].requiredAccess = 3;
        guidCommands(client, args);
    } else if (cmd == "remove" || cmd == "edit") {
        size_t id;
        if (args.size() < 2 || (cmd == "remove" && args.size() != 2) || (cmd == "edit" && args.size() < 3)) {
            reply(client, "Usage: /aa remove <point ID> or /aa edit <point ID> <radius|name|c distance|e distance>"); return;
        }
        if (!pointId(client, args[1], id)) return;
        auto points = state.points;
        const std::string previous = describePoint(points[id]);
        if (cmd == "remove") points.erase(points.begin() + id);
        else {
            Point& point = points[id];
            float distance;
            const auto option = lower(args[2]);
            if (option == "c" || option == "e") {
                if (point.trickplant || args.size() != 4 || !distanceValue(args[3], distance)) {
                    reply(client, "Charge drain requires an antirush point and a finite radius from 0 to 100000."); return;
                }
                (option == "c" ? point.covertRadius : point.engineerRadius) = distance;
            } else if (args.size() == 3 && distanceValue(args[2], distance)) {
                if (distance == 0) { reply(client, "Protection radius must be positive."); return; }
                point.radius = distance;
            } else {
                const size_t first = option.find_first_not_of("+-");
                const bool numericLike = first != std::string::npos &&
                    ((option[first] >= '0' && option[first] <= '9') || option[first] == '.' ||
                     option.substr(first) == "nan" || option.compare(first, 4, "nan(") == 0 ||
                     option.substr(first) == "inf" || option.substr(first) == "infinity");
                if (args.size() == 3 && option.find(' ') == std::string::npos && numericLike) {
                    reply(client, "Protection radius must be greater than 0 and at most 100000."); return;
                }
                if (point.trickplant) { reply(client, "Trickplant points only accept a positive radius."); return; }
                std::string name = args[2];
                for (size_t i = 3; i < args.size(); ++i) name += " " + args[i];
                name = sanitizeText(name);
                if (name.empty() || name.size() > 256) { reply(client, "Objective name must contain 1 to 256 characters."); return; }
                point.name = name;
            }
        }
        const std::string detail = cmd == "remove" ? "removed " + previous :
            "before=[" + previous + "] after=[" + describePoint(points[id]) + "]";
        if (!savePoints(points)) { reply(client, "^1Could not save the map file; protection unchanged."); return; }
        changed();
        logChange(client, fullCommand(args), detail);
        reply(client, "^2Protection points saved.");
    } else reply(client, "Unknown Antirush command. Use /aa help.");
}
std::vector<std::string> arguments(int start) {
    std::vector<std::string> args;
    for (int i = start; i < trap_Argc() && args.size() < 32; ++i) args.push_back(arg(i));
    return args;
}
}

bool authenticated(int client, std::string* guid, int* rank) {
    if (client < 0 || client >= level.maxclients || client >= MAX_CLIENTS) return false;
    const auto& ent = g_entities[client];
    if (!ent.inuse || !ent.client || ent.client->pers.connected != CON_CONNECTED || (ent.r.svFlags & SVF_BOT)) return false;
    const auto& object = g_clientObjects[client];
    const auto* user = connectedUsers[client];
    if (!object.authenticated || !user || user == &User::BAD || user->fakeguid) return false;
    const std::string key = normalizeGuid(object.authGuid);
    if (key.empty() || key != normalizeGuid(user->guid)) return false;
    if (guid) *guid = key;
    if (rank) *rank = user->authLevel;
    return true;
}
bool canEdit(int client) {
    if (!cvars::g_admin.ivalue) return false;
    if (client < 0) return true;
    std::string guid;
    int rank;
    if (!authenticated(client, &guid, &rank)) return false;
    const User& user = *connectedUsers[client];
    const Privilege& permission = cmd::builtins::aa._privilege;
    if (explicitlyDenied(user, permission)) return false;
    if (user.hasPrivilege(permission) || rank == state.settings.adminLevel ||
        (state.settings.userLevel >= 0 && rank == state.settings.userLevel)) return true;
    for (const auto& entry : state.guids) if (entry.guid == guid) return true;
    return false;
}
bool canManage(int client) {
    if (!cvars::g_admin.ivalue) return false;
    if (client < 0) return true;
    int rank;
    if (!authenticated(client, nullptr, &rank)) return false;
    const User& user = *connectedUsers[client];
    const Privilege& permission = cmd::builtins::antirush._privilege;
    return !explicitlyDenied(user, permission) &&
        (user.hasPrivilege(permission) || rank == state.settings.adminLevel);
}
void runCommand(int client, const std::vector<std::string>& args, bool mode) {
    execute(client, args, mode);
}
bool dispatchAlias(int client, const std::string& name) {
    replies[index(client)].lines.clear();
    replies[index(client)].requiredAccess = 0;
    if (!cvars::g_admin.ivalue) {
        reply(client, "The Xmod admin system is disabled (g_admin 0).");
        return true;
    }
    cmd::AbstractCommand::Context context(client < 0 ? nullptr : &g_clientObjects[client]);
    context._args = arguments(1);
    context._args.insert(context._args.begin(), name);
    cmd::AbstractCommand* command = cmd::commandForName(name);
    if (command) command->execute(context);
    return true;
}
bool clientCommand(gentity_t* player) {
    if (!player || !player->client) return false;
    const int client = player - g_entities;
    if (client < 0 || client >= MAX_CLIENTS) return false;
    const std::string command = lower(arg(0));
    return command == "aa" ? dispatchAlias(client, "aa") : false;
}
bool consoleCommand() {
    const std::string command = lower(arg(0));
    if (command != "aa" && command != "antirush") return false;
    return dispatchAlias(-1, command);
}
void resetCommands(int client) {
    if (client >= 0 && client < MAX_CLIENTS) replies[client] = ReplyState();
    else if (client == -1) for (auto& r : replies) r = ReplyState();
}
void pumpReplies() {
    for (int client = 0; client < level.maxclients; ++client) {
        auto& r = replies[client];
        // Losing auth immediately drops potentially sensitive pending menus.
        if (!g_entities[client].inuse || !g_entities[client].client ||
            g_entities[client].client->pers.connected != CON_CONNECTED ||
            ((r.requiredAccess == 1 || r.requiredAccess == 3) && !authorized(client)) ||
            ((r.requiredAccess == 2 || r.requiredAccess == 3) && !admin(client))) {
            r.lines.clear(); continue;
        }
        while (!r.lines.empty() && r.budget.take(static_cast<uint32_t>(level.time))) {
            trap_SendServerCommand(client, va("print \"%s\n\"", r.lines.front().c_str()));
            r.lines.pop_front();
        }
    }
}
}
