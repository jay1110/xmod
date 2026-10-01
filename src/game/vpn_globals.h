#ifndef GAME_VPN_GLOBALS_H
#define GAME_VPN_GLOBALS_H

#include <bgame/q_shared.h>
#include <string>

namespace vpnblocker {
class Service;
extern vmCvar_t g_vpnBlockerEnabled;
extern vmCvar_t g_vpnBlockerApiKey1;
extern vmCvar_t g_vpnBlockerApiKey2;
extern vmCvar_t g_vpnBlockerMaxLevel;
extern vmCvar_t g_vpnBlockerBanMessageVPN;
extern vmCvar_t g_vpnBlockerDBPath;
extern vmCvar_t g_vpnBlockerBanMessageBlacklist;

// Engine-thread API. An injected service transfers ownership to this module.
void init(Service* service = nullptr);
void shutdown();
void frame();
void clientConnect(int clientNum, const char* userinfo, bool isBot);
void clientDisconnect(int clientNum);

// Engine-thread administration. Never expose provider credentials or wait for HTTP.
struct Status {
    bool available = false;
    bool enabled = false;
    bool provider1 = false;
    bool provider2 = false;
    int maxLevel = 0;
    int queued = 0;
    int allowed = 0;
    int blocked = 0;
    int unavailable = 0;
    int exempt = 0;
    int skipped = 0;
    bool database = false;
    int ipWhitelist = 0;
    int ipBlacklist = 0;
    int guidWhitelist = 0;
    int legacyGuids = 0;
    std::string databaseError;
};
enum class Recheck { Queued, Disabled, Unavailable, NoProvider, Disconnected, Skipped, Exempt };
Status status();
void setEnabled(bool wanted);
Recheck recheck(int clientNum);
bool reloadPolicy(std::string& error);
// Informational arbitrary public-IP check, never a ban or player lookup. A player
// requester must retain authenticated identity and C/vpn-check until the reply.
bool requestIpCheck(int requesterSlot, const std::string& ip, std::string& error);
}

#endif
