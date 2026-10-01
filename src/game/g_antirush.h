#ifndef GAME_ANTIRUSH_H
#define GAME_ANTIRUSH_H

// Include after bgame/impl.h. Native port of the supplied AutoAdmin 4.4.5.
namespace antirush {
struct Message {
    std::string channel;
    std::string text;
};
struct Settings {
    int adminLevel = 999;
    int userLevel = -1;
    double protectFraction = 1.0 / 3.0;
    float antirushRange = 500;
    float trickplantRange = 200;
    float legacyDrainMultiplier = 2;
    int countdownMs = 60000;
    int startMessageDelayMs = 5000;
    bool messagesEnabled = true;
    std::string logPath = "antirush/antirush.log";
    std::map<std::string, Message> messages;
};
struct Point {
    bool trickplant = false;
    std::string name;
    vec3_t origin = {0, 0, 0};
    float radius = 0;
    float covertRadius = 0;
    float engineerRadius = 0;
};
struct GuidEntry {
    std::string guid;
    std::string name;
};
struct State {
    Settings settings;
    std::vector<Point> points;
    std::vector<GuidEntry> guids;
    std::string mapName;
    bool mode = true;
};
extern State state;

// Configuration/persistence. Commit memory only after a successful disk write.
void load();
bool savePoints(const std::vector<Point>& replacement);
bool saveGuids(const std::vector<GuidEntry>& replacement);
bool saveMode(bool enabled);
std::vector<std::string> configuredMaps();
std::string sanitizeText(const std::string& text);
std::string normalizeGuid(const std::string& guid); // 40 hexadecimal characters only
void logChange(int clientNum, const std::string& command, const std::string& detail);

// Runtime. Mode 2 is this native port; mode 1 retains the existing legacy rules.
void init();
void shutdown();
void frame();
void clientReset(int clientNum);
bool enabled();
bool active();
int remainingMs();
bool held(int clientNum);
bool objectivePickup(gentity_t* objective, gentity_t* player); // true permits pickup
bool dynamiteArmed(gentity_t* dynamite, gentity_t* armer); // false: caller frees it and returns
void changed(); // mode/point settings changed; reevaluate holds immediately
void notify(int clientNum);
void announce(const std::string& key, const std::map<std::string, std::string>& values,
              bool setup = false);

// Commands/authorization. Console is clientNum -1; never trust userinfo GUIDs.
bool authenticated(int clientNum, std::string* guid = nullptr, int* adminLevel = nullptr);
bool canEdit(int clientNum);
bool canManage(int clientNum);
void runCommand(int clientNum, const std::vector<std::string>& args, bool modeCommand);
bool clientCommand(gentity_t* player);
bool consoleCommand();
void resetCommands(int clientNum);
void pumpReplies();
bool entityPosition(const gentity_t* ent, vec3_t position, bool objective);
std::string entityName(const gentity_t* ent);
}
#endif
