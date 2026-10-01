#include <bgame/impl.h>
#include <bgame/numeric_text.h>
#include <game/g_antirush.h>
#include <cmath>
#include <climits>

namespace antirush {
namespace {
struct CarrierHold {
    bool pending = false;
    int team = TEAM_FREE;
    int powerup = PW_NONE;
    vec3_t pickupOrigin = {0, 0, 0};
};
CarrierHold carriers[MAX_CLIENTS];
bool roundSeen = false;
bool startAnnounced = false;
bool endAnnounced = false;
int roundStart = 0;
long long nextCountdown = 0;
int countdownPeriod = 0;
int countdownDuration = 0;

int now() { return level.timeCurrent < level.startTime ? level.startTime : level.timeCurrent; }
bool playing() {
    return cvars::gameState.ivalue == GS_PLAYING && !level.intermissionQueued && !level.intermissiontime;
}
int duration() {
    // Parse the original cvar text before arithmetic: native release builds use
    // fast-math, which can optimize away floating-point isfinite checks.
    double minutes;
    if (!XmodParseFiniteDecimal(g_timelimit.string, minutes) || minutes <= 0) return 0;
    if (minutes > INT_MAX / 60000.0) minutes = INT_MAX / 60000.0;
    const double ms = minutes * state.settings.protectFraction * 60000.0;
    if (ms <= 0) return 0;
    return ms >= INT_MAX ? INT_MAX : static_cast<int>(ms);
}
void clockRound() {
    if (!roundSeen && playing()) {
        roundSeen = true;
        roundStart = now();
        startAnnounced = endAnnounced = false;
        nextCountdown = 0;
    }
}
bool alive(const gentity_t* ent) {
    return ent && ent->inuse && ent->client && ent->client->pers.connected == CON_CONNECTED &&
        ent->health > 0 && ent->client->ps.stats[STAT_HEALTH] > 0 &&
        !(ent->client->ps.pm_flags & PMF_LIMBO) &&
        (ent->client->sess.sessionTeam == TEAM_AXIS || ent->client->sess.sessionTeam == TEAM_ALLIES);
}
bool inside(const vec3_t location, const Point& point, float radius) {
    if (!std::isfinite(radius) || radius <= 0) return false;
    double squared = 0;
    for (int i = 0; i < 3; ++i) {
        const double delta = static_cast<double>(location[i]) - point.origin[i];
        squared += delta * delta;
    }
    return squared <= static_cast<double>(radius) * radius;
}
const Point* protectionAt(const vec3_t position) {
    for (const Point& point : state.points)
        if (!point.trickplant && inside(position, point, point.radius)) return &point;
    return nullptr;
}
std::string durationText(int milliseconds) {
    const int seconds = milliseconds / 1000 + (milliseconds % 1000 != 0);
    return seconds >= 60 ? std::to_string(seconds / 60) + "m " +
        std::to_string(seconds % 60) + "s" : std::to_string(seconds) + "s";
}
std::string objectiveNames() {
    std::string names;
    for (const Point& point : state.points) {
        if (point.trickplant) continue;
        if (!names.empty()) names += ", ";
        names += point.name;
        if (names.size() > 500) { names.resize(500); names += "..."; break; }
    }
    return names;
}
// A quoted server command must fit the engine's reliable command limit.
std::string networkText(const std::string& input) {
    std::string output;
    for (char value : input) {
        if (output.size() >= 850) break;
        const unsigned char c = static_cast<unsigned char>(value);
        output += c < 32 || c == 127 ? ' ' : c == '"' ? '\'' : c == '\\' ? '/' : value;
    }
    return output;
}
void send(int recipient, const std::string& channel, const std::string& text) {
    if (channel != "chat" && channel != "cp" && channel != "cpm" && channel != "print") return;
    const std::string command = channel + " \"" + networkText(text) +
        (channel == "print" ? "\n\"" : "\"");
    trap_SendServerCommand(recipient, command.c_str());
}
void punish(gentity_t* player) {
    if (!alive(player) || !playing()) return;
    // This is a rules penalty, not weapon damage. Spawn protection cannot bypass it.
    player->client->ps.stats[STAT_HEALTH] = player->health = 0;
    player->client->ps.persistant[PERS_HWEAPON_USE] = 0;
    player_die(player, &g_entities[ENTITYNUM_WORLD], &g_entities[ENTITYNUM_WORLD], 400, MOD_UNKNOWN);
}
int offensiveTeam() {
    char info[MAX_INFO_STRING];
    trap_GetConfigstring(CS_MULTI_INFO, info, sizeof(info));
    const char* defender = Info_ValueForKey(info, "d");
    if (!strcmp(defender, "0")) return TEAM_ALLIES;
    if (!strcmp(defender, "1")) return TEAM_AXIS;
    return TEAM_FREE;
}
void drainCharge() {
    const int offensive = offensiveTeam();
    if (offensive != TEAM_AXIS && offensive != TEAM_ALLIES) return;
    for (int slot = 0; slot < level.maxclients; ++slot) {
        gentity_t* player = &g_entities[slot];
        if (!alive(player) || player->client->sess.sessionTeam != offensive) continue;
        const int playerClass = player->client->ps.stats[STAT_PLAYER_CLASS];
        if (playerClass != PC_COVERTOPS && playerClass != PC_ENGINEER) continue;
        for (const Point& point : state.points) {
            if (point.trickplant) continue;
            const float radius = playerClass == PC_COVERTOPS ? point.covertRadius : point.engineerRadius;
            if (inside(player->client->ps.origin, point, radius)) {
                player->client->ps.classWeaponTime = level.time;
                break;
            }
        }
    }
}
}

void init() {
    resetCommands(-1); // include the console's GUID-removal snapshot
    for (int slot = 0; slot < MAX_CLIENTS; ++slot) clientReset(slot);
    load();
    roundSeen = playing();
    roundStart = level.startTime;
    startAnnounced = endAnnounced = false;
    nextCountdown = 0;
    countdownPeriod = 0;
    countdownDuration = 0;
}
void shutdown() {
    resetCommands(-1);
    for (int slot = 0; slot < MAX_CLIENTS; ++slot) clientReset(slot);
    roundSeen = false;
}
void clientReset(int slot) {
    if (slot < 0 || slot >= MAX_CLIENTS) return;
    carriers[slot] = CarrierHold();
    resetCommands(slot);
}
bool enabled() { return g_antirush.integer == 2 && state.mode; }
int remainingMs() {
    clockRound();
    if (!roundSeen) return 0;
    const long long remaining = static_cast<long long>(roundStart) + duration() - now();
    return remaining <= 0 ? 0 : remaining >= INT_MAX ? INT_MAX : static_cast<int>(remaining);
}
bool active() { return enabled() && playing() && remainingMs() > 0; }
bool held(int slot) {
    if (slot < 0 || slot >= MAX_CLIENTS || !carriers[slot].pending) return false;
    const gentity_t* player = &g_entities[slot];
    if (!active() || !alive(player) || !(player->r.svFlags & SVF_BOT) ||
        player->client->sess.sessionTeam != carriers[slot].team ||
        !player->client->ps.powerups[carriers[slot].powerup] ||
        !protectionAt(carriers[slot].pickupOrigin)) {
        carriers[slot] = CarrierHold();
        return false;
    }
    return true;
}
void changed() {
    for (int slot = 0; slot < MAX_CLIENTS; ++slot) held(slot);
    nextCountdown = 0;
}
void announce(const std::string& key, const std::map<std::string, std::string>& values, bool setup) {
    if (!state.settings.messagesEnabled || (!setup && !enabled())) return;
    const auto message = state.settings.messages.find(key);
    if (message == state.settings.messages.end()) return;
    std::string text = message->second.text;
    for (const auto& value : values) {
        const std::string token = "{" + value.first + "}";
        size_t offset = 0;
        while ((offset = text.find(token, offset)) != std::string::npos) {
            text.replace(offset, token.size(), value.second);
            offset += value.second.size();
        }
    }
    send(-1, message->second.channel, text);
}
void notify(int slot) {
    if (!active() || !state.settings.messagesEnabled || objectiveNames().empty()) return;
    send(slot, "cp", "^3AntiRush:^7 objectives protected for " + durationText(remainingMs()));
}
void frame() {
    pumpReplies();
    clockRound();
    for (int slot = 0; slot < MAX_CLIENTS; ++slot) held(slot);
    if (!enabled() || !playing()) return;
    const std::string names = objectiveNames();
    const int remaining = remainingMs();
    if (remaining > 0) {
        endAnnounced = false;
        drainCharge();
        if (!startAnnounced && static_cast<long long>(now()) - roundStart >= state.settings.startMessageDelayMs) {
            if (!names.empty()) announce("START", {{"duration", durationText(duration())}, {"objectives", names}});
            startAnnounced = true;
        }
        const int interval = state.settings.countdownMs;
        if (interval > 0) {
            const long long ending = static_cast<long long>(roundStart) + duration();
            if (!nextCountdown || interval != countdownPeriod || duration() != countdownDuration) {
                // Align announcements with the expiry (whole minutes remaining).
                nextCountdown = ending - static_cast<long long>((remaining - 1) / interval) * interval;
                countdownPeriod = interval;
                countdownDuration = duration();
            }
            if (now() >= nextCountdown) {
                const long long skipped = (static_cast<long long>(now()) - nextCountdown) / interval;
                const long long boundary = nextCountdown + skipped * interval;
                const int remainingAtBoundary = static_cast<int>(ending - boundary);
                const int seconds = remainingAtBoundary / 1000 + (remainingAtBoundary % 1000 != 0);
                nextCountdown = boundary + interval;
                if (!names.empty()) announce("COUNTDOWN", {{"remaining", durationText(remainingAtBoundary)},
                    {"minutes", std::to_string(seconds / 60)}, {"seconds", std::to_string(seconds % 60)}});
            }
        } else nextCountdown = 0;
    } else if (!endAnnounced) {
        if (duration() > 0 && !names.empty()) announce("END", {{"objectives", names}});
        endAnnounced = true;
    }
}
bool objectivePickup(gentity_t* objective, gentity_t* player) {
    if (!active() || !objective || !alive(player)) return true;
    const Point* point = protectionAt(player->client->ps.origin);
    if (!point) return true;
    const std::map<std::string, std::string> values = {
        {"player", player->client->pers.netname}, {"objective", point->name}};
    if (player->r.svFlags & SVF_BOT) {
        const int slot = static_cast<int>(player - g_entities);
        if (!carriers[slot].pending) announce("BOT", values);
        carriers[slot].pending = true;
        carriers[slot].team = player->client->sess.sessionTeam;
        carriers[slot].powerup = objective->classname && !strcmp(objective->classname, "team_CTF_redflag")
            ? PW_REDFLAG : PW_BLUEFLAG;
        VectorCopy(player->client->ps.origin, carriers[slot].pickupOrigin);
        VectorClear(player->client->ps.velocity);
        return true;
    }
    announce("RUSH", values);
    punish(player);
    return false;
}
bool dynamiteArmed(gentity_t* dynamite, gentity_t* armer) {
    if (!enabled() || !playing() || !dynamite || !dynamite->inuse) return true;
    const Point* point = active() ? protectionAt(dynamite->r.currentOrigin) : nullptr;
    if (point) {
        announce("RUSH", {{"player", armer && armer->client ? armer->client->pers.netname : "unknown"},
            {"objective", point->name}});
        punish(armer);
        return false;
    }
    for (const Point& trick : state.points) {
        if (!trick.trickplant || !inside(dynamite->r.currentOrigin, trick, trick.radius)) continue;
        if (armer && armer->client) announce("TRICKPLANT", {{"player", armer->client->pers.netname}});
        else announce("TRICKPLANT_UNKNOWN", {});
        return false;
    }
    return true;
}
bool entityPosition(const gentity_t* ent, vec3_t position, bool objective) {
    if (!ent || !ent->inuse) return false;
    VectorCopy(ent->r.currentOrigin, position);
    if (objective && (ent->r.bmodel || (ent->classname && !strcmp(ent->classname, "func_explosive")))) {
        bool valid = true, extent = false;
        for (int i = 0; i < 3; ++i) {
            valid = valid && std::isfinite(ent->r.absmin[i]) && std::isfinite(ent->r.absmax[i]) &&
                ent->r.absmax[i] >= ent->r.absmin[i];
            extent = extent || ent->r.absmax[i] > ent->r.absmin[i];
        }
        if (valid && extent) for (int i = 0; i < 3; ++i)
            position[i] = ent->r.absmin[i] * 0.5f + ent->r.absmax[i] * 0.5f;
    }
    for (int i = 0; i < 3; ++i) if (!std::isfinite(position[i])) return false;
    return true;
}
std::string entityName(const gentity_t* ent) {
    if (!ent) return "unknown";
    if (ent->message && *ent->message) return sanitizeText(ent->message);
    if (ent->track && *ent->track) return sanitizeText(ent->track);
    if (ent->targetname && *ent->targetname) return sanitizeText(ent->targetname);
    if (ent->scriptName && *ent->scriptName) return sanitizeText(ent->scriptName);
    return "unknown";
}
}
