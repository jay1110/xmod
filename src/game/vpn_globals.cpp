// Network work stays on the background worker; policy and engine calls stay here.
#include <array>
#include <cstdint>
#include <memory>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <game/vpn_service.h>
#include <game/vpn_policy.h>
#include <bgame/impl.h>
#include <game/vpn_globals.h>

namespace vpnblocker {
vmCvar_t g_vpnBlockerEnabled;
vmCvar_t g_vpnBlockerApiKey1;
vmCvar_t g_vpnBlockerApiKey2;
vmCvar_t g_vpnBlockerMaxLevel;
vmCvar_t g_vpnBlockerBanMessageVPN;
vmCvar_t g_vpnBlockerDBPath;
vmCvar_t g_vpnBlockerBanMessageBlacklist;

namespace {
struct Pending {
    std::uint64_t id = 0;
    std::string ip;
    bool done = false;
    bool blocked = false;
    bool blacklist = false;
    bool exempt = false;
    Verdict verdict = Verdict::Unknown;
    bool refresh = false;
    bool began = false;
    std::uint32_t beginTime = 0;
    std::uint32_t retryTime = 0;
};
struct Inspection {
    std::uint64_t id = 0;
    int requester = -1;
    std::string guid;
    std::string ip;
    std::uint32_t began = 0;
};
std::array<Pending, MAX_CLIENTS> pending;
std::array<Inspection, MAX_CLIENTS + 1> inspections;
std::unique_ptr<Service> worker;
std::uint64_t nextId = 0;
Config config;
bool enabled = false;
int maxLevel = 0;
bool configured = false;
bool policyConfigured = false;
std::string policyPath;
std::string policyError;
std::uint64_t policyGeneration = 0;
constexpr std::uint32_t AUTH_GRACE_MS = 15000;
constexpr std::uint32_t INSPECTION_TIMEOUT_MS = 30000;

bool validSlot(int n) { return n >= 0 && n < MAX_CLIENTS; }
bool active(int n) {
    return g_entities[n].client && g_entities[n].client->pers.connected != CON_DISCONNECTED &&
        !(g_entities[n].r.svFlags & SVF_BOT);
}
const User* confirmed(int n) {
    if (!validSlot(n) || !active(n) || !g_clientObjects[n].authenticated) return nullptr;
    const User* user = connectedUsers[n];
    const std::string guid = policy::normalizeGuid(g_clientObjects[n].authGuid);
    return !guid.empty() && user && user != &User::BAD && !user->fakeguid &&
        policy::normalizeGuid(user->guid) == guid ? user : nullptr;
}
bool exempt(int n) {
    const User* user = confirmed(n);
    return user && (user->authLevel > maxLevel || policy::containsGuid(user->guid));
}
bool providersReady() {
    return worker && enabled && (!config.apiKey1.empty() || !config.apiKey2.empty());
}
void forgetRequest(Pending& p) {
    if (worker && p.id) worker->cancel(p.id);
    p.id = 0;
}
void forgetInspection(Inspection& request) {
    if (worker && request.id) worker->cancel(request.id);
    request = Inspection();
}
void queue(int n, std::uint32_t now) {
    Pending& p = pending[n];
    if (!enabled || p.ip.empty() || p.done || p.id) return;
    if (exempt(n)) {
        p.done = true; p.exempt = true; p.verdict = Verdict::Allowed; return;
    }
    const policy::Decision decision = policy::lookupIp(p.ip);
    if (decision != policy::Decision::None) {
        p.done = true; p.blacklist = decision == policy::Decision::Blacklist;
        p.blocked = p.blacklist;
        p.verdict = p.blocked ? Verdict::Blocked : Verdict::Allowed;
        return;
    }
    if (!providersReady()) { p.done = true; return; }
    if (p.retryTime && static_cast<std::int32_t>(now - p.retryTime) < 0) return;
    const std::uint64_t id = ++nextId;
    if (worker->submit(id, p.ip, config, p.refresh)) {
        p.id = id; p.retryTime = 0;
    } else {
        p.retryTime = now + 1000;
    }
}
bool component(const std::string& value) {
    if (value.empty() || value == "." || value == "..") return false;
    for (char c : value) if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return false;
    return true;
}
bool directory(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return (st.st_mode & S_IFDIR) != 0;
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}
std::string databasePath(const std::string& relative) {
    if (relative.empty() || relative.size() > 200) return "";
    std::vector<std::string> parts;
    size_t begin = 0;
    do {
        const size_t slash = relative.find('/', begin);
        const std::string part = relative.substr(begin, slash == std::string::npos ? slash : slash - begin);
        if (!component(part)) return "";
        parts.push_back(part);
        if (slash == std::string::npos) break;
        begin = slash + 1;
    } while (begin <= relative.size());
    char home[MAX_OSPATH] = {}, game[MAX_QPATH] = {};
    trap_Cvar_VariableStringBuffer("fs_homepath", home, sizeof(home));
    trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
    const std::string mod = game[0] ? game : "etmain";
    if (!home[0] || !component(mod)) return "";
    std::string path = std::string(home) + "/" + mod;
    if (!directory(path)) return "";
    for (size_t n = 0; n < parts.size(); ++n) {
        path += "/" + parts[n];
        if (n + 1 < parts.size() && !directory(path)) return "";
    }
    return path;
}
void configure() {
    if (!policyConfigured || policyPath != g_vpnBlockerDBPath.string) {
        policy::close();
        policyPath = g_vpnBlockerDBPath.string;
        policyConfigured = true;
        policyError.clear();
        if (!policyPath.empty()) {
            const std::string physical = databasePath(policyPath);
            if (physical.empty()) policyError = "Invalid or inaccessible VPN database path; use a path relative to the mod directory.";
            else policy::open(physical, policyError);
            if (!policyError.empty()) G_Printf("[VPN Blocker] %s\n", policyError.c_str());
        }
    }
    const bool wanted = g_vpnBlockerEnabled.integer > 0;
    const int threshold = g_vpnBlockerMaxLevel.integer < 0 ? 0 : g_vpnBlockerMaxLevel.integer;
    if (configured && wanted == enabled && threshold == maxLevel &&
        policyGeneration == policy::revision() && config.apiKey1 == g_vpnBlockerApiKey1.string &&
        config.apiKey2 == g_vpnBlockerApiKey2.string) return;
    enabled = wanted; maxLevel = threshold;
    config.apiKey1 = g_vpnBlockerApiKey1.string;
    config.apiKey2 = g_vpnBlockerApiKey2.string;
    configured = true;
    policyGeneration = policy::revision();
    for (auto& inspection : inspections) forgetInspection(inspection);
    for (int n = 0; n < MAX_CLIENTS; ++n) {
        forgetRequest(pending[n]); pending[n] = Pending();
        if (active(n)) {
            char userinfo[MAX_INFO_STRING];
            trap_GetUserinfo(n, userinfo, sizeof(userinfo));
            NormalizeAddress(Info_ValueForKey(userinfo, "ip"), pending[n].ip);
        }
    }
    if (enabled && !worker)
        G_Printf("[VPN Blocker] Online lookups are unavailable on this server platform.\n");
    else if (enabled && config.apiKey1.empty() && config.apiKey2.empty())
        G_Printf("[VPN Blocker] No API key configured; only local policy rules are active.\n");
}
bool inspectionAuthorized(int requester, std::string* identity = nullptr) {
    if (requester == -1) return true;
    if (!cvars::g_admin.ivalue || !validSlot(requester) || !confirmed(requester) ||
        g_entities[requester].client->pers.connected != CON_CONNECTED) return false;
    cmd::AbstractCommand* command = cmd::commandForName("vpn-check");
    cmd::AbstractCommand::Context context(&g_clientObjects[requester]);
    if (!command || !command->hasPermission(context)) return false;
    if (identity) *identity = policy::normalizeGuid(g_clientObjects[requester].authGuid);
    return true;
}
void inspectionReply(int requester, const std::string& ip, const char* decision) {
    text::Buffer buf;
    buf << "vpn-check: " << text::xvalue(ip) << " - " << decision;
    cmd::printChat(requester < 0 ? nullptr : &g_clientObjects[requester], buf);
}
}

void init(Service* service) {
    shutdown();
    if (service) worker.reset(service);
    else if (supported()) worker.reset(new Service());
    configured = false;
}
void shutdown() {
    if (worker) worker->stop();
    worker.reset();
    for (auto& p : pending) p = Pending();
    for (auto& request : inspections) request = Inspection();
    policy::close(); policyConfigured = false; policyPath.clear(); policyError.clear();
    config = Config(); enabled = false; configured = false;
}
void clientDisconnect(int clientNum) {
    if (!validSlot(clientNum)) return;
    forgetRequest(pending[clientNum]); pending[clientNum] = Pending();
    forgetInspection(inspections[clientNum]);
}
void clientConnect(int clientNum, const char* userinfo, bool isBot) {
    if (!validSlot(clientNum)) return;
    clientDisconnect(clientNum);
    if (isBot || !userinfo || (g_entities[clientNum].r.svFlags & SVF_BOT)) return;
    configure();
    NormalizeAddress(Info_ValueForKey(userinfo, "ip"), pending[clientNum].ip);
    queue(clientNum, static_cast<std::uint32_t>(trap_Milliseconds()));
}
Status status() {
    configure();
    Status result;
    result.available = worker != nullptr;
    result.enabled = enabled;
    result.provider1 = !config.apiKey1.empty(); result.provider2 = !config.apiKey2.empty();
    result.maxLevel = maxLevel;
    const policy::Counts lists = policy::counts();
    result.database = lists.available; result.ipWhitelist = lists.whitelist;
    result.ipBlacklist = lists.blacklist; result.guidWhitelist = lists.guids;
    result.legacyGuids = lists.legacyGuids; result.databaseError = policyError;
    for (int n = 0; n < MAX_CLIENTS; ++n) {
        if (!g_entities[n].client || g_entities[n].client->pers.connected == CON_DISCONNECTED) continue;
        const Pending& p = pending[n];
        if (!active(n) || p.ip.empty()) ++result.skipped;
        else if (exempt(n)) ++result.exempt;
        else if (!enabled) continue;
        else if (!p.done) ++result.queued;
        else if (p.blocked) ++result.blocked;
        else if (p.verdict == Verdict::Allowed) ++result.allowed;
        else if (p.verdict == Verdict::Unknown) ++result.unavailable;
    }
    return result;
}
void setEnabled(bool wanted) {
    trap_Cvar_Set("g_vpnBlockerEnabled", wanted ? "1" : "0");
    trap_Cvar_Update(&g_vpnBlockerEnabled); configure();
}
bool reloadPolicy(std::string& error) {
    configure();
    if (!policy::counts().available) {
        policyConfigured = false; configure();
        error = policyError.empty() ? "VPN policy database is disabled." : policyError;
        if (!policy::counts().available) return false;
    }
    if (!policy::reload(error)) { policyError = error; return false; }
    policyError.clear(); configure(); return true;
}
Recheck recheck(int clientNum) {
    configure();
    if (!enabled) return Recheck::Disabled;
    if (!validSlot(clientNum) || !g_entities[clientNum].client ||
        g_entities[clientNum].client->pers.connected != CON_CONNECTED) return Recheck::Disconnected;
    if (!active(clientNum)) return Recheck::Skipped;
    char userinfo[MAX_INFO_STRING]; std::string ip;
    trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
    if (!NormalizeAddress(Info_ValueForKey(userinfo, "ip"), ip)) return Recheck::Skipped;
    if (exempt(clientNum)) return Recheck::Exempt;
    if (policy::lookupIp(ip) == policy::Decision::None) {
        if (!worker) return Recheck::Unavailable;
        if (config.apiKey1.empty() && config.apiKey2.empty()) return Recheck::NoProvider;
    }
    forgetRequest(pending[clientNum]); pending[clientNum] = Pending();
    pending[clientNum].ip = ip; pending[clientNum].refresh = true;
    queue(clientNum, static_cast<std::uint32_t>(trap_Milliseconds()));
    return Recheck::Queued;
}
bool requestIpCheck(int requester, const std::string& raw, std::string& error) {
    error.clear(); configure();
    std::string guid, ip;
    if (!inspectionAuthorized(requester, &guid)) { error = "Confirmed GUID and C/vpn-check access are required."; return false; }
    if (!enabled) { error = "The VPN blocker is disabled."; return false; }
    if (!NormalizeAddress(raw, ip)) { error = "Use a public numeric IP address; local/private/reserved addresses are skipped."; return false; }
    const policy::Decision local = policy::lookupIp(ip);
    if (local != policy::Decision::None) {
        inspectionReply(requester, ip, local == policy::Decision::Whitelist ? "allowed by IP whitelist." : "blocked by IP blacklist.");
        return true;
    }
    if (!providersReady()) { error = "No online provider is available for this address."; return false; }
    Inspection& request = inspections[requester < 0 ? MAX_CLIENTS : requester];
    if (request.id) { error = "An IP check is already pending for you."; return false; }
    const std::uint64_t id = ++nextId;
    if (!worker->submit(id, ip, config, true)) { error = "VPN checker queue is busy; try again shortly."; return false; }
    request.id = id; request.requester = requester; request.guid = guid; request.ip = ip;
    request.began = static_cast<std::uint32_t>(trap_Milliseconds());
    inspectionReply(requester, ip, "fresh background check queued; this command does not disconnect a player.");
    return true;
}
void frame() {
    configure();
    if (!enabled) return;
    const std::uint32_t now = static_cast<std::uint32_t>(trap_Milliseconds());
    if (worker) for (const Result& result : worker->poll()) {
        for (auto& inspection : inspections) {
            if (!inspection.id || inspection.id != result.id || inspection.ip != result.ip) continue;
            std::string guid;
            if (inspectionAuthorized(inspection.requester, &guid) && guid == inspection.guid)
                inspectionReply(inspection.requester, inspection.ip,
                    result.verdict == Verdict::Allowed ? "online providers report allowed."
                    : result.verdict == Verdict::Blocked ? "online providers report VPN/proxy."
                    : "provider unavailable; no decision was made.");
            inspection = Inspection(); break;
        }
        for (int n = 0; n < MAX_CLIENTS; ++n) {
            Pending& p = pending[n];
            if (!p.id || p.id != result.id || p.ip != result.ip) continue;
            p.done = true; p.verdict = result.verdict; p.blocked = result.verdict == Verdict::Blocked;
            if (result.verdict == Verdict::Unknown)
                G_Printf("[VPN Blocker] Client %d: lookup unavailable; connection retained.\n", n);
            break;
        }
    }
    for (auto& inspection : inspections) {
        if (!inspection.id) continue;
        std::string guid;
        if (!inspectionAuthorized(inspection.requester, &guid) || guid != inspection.guid) {
            forgetInspection(inspection); continue;
        }
        if (now - inspection.began >= INSPECTION_TIMEOUT_MS) {
            inspectionReply(inspection.requester, inspection.ip, "check timed out; no decision was made.");
            forgetInspection(inspection);
        }
    }
    for (int n = 0; n < MAX_CLIENTS; ++n) {
        Pending& p = pending[n];
        if (!active(n)) { clientDisconnect(n); continue; }
        if (exempt(n)) {
            forgetRequest(p); p.blocked = false; p.exempt = true; p.done = true; p.verdict = Verdict::Allowed;
            continue;
        }
        if (p.exempt) {
            const std::string ip = p.ip; p = Pending(); p.ip = ip;
        }
        queue(n, now);
        if (!p.blocked) continue;
        char userinfo[MAX_INFO_STRING]; std::string currentIp;
        trap_GetUserinfo(n, userinfo, sizeof(userinfo));
        if (!NormalizeAddress(Info_ValueForKey(userinfo, "ip"), currentIp) || currentIp != p.ip) {
            clientConnect(n, userinfo, false); continue;
        }
        if (g_entities[n].client->pers.connected != CON_CONNECTED) continue;
        if (!p.began) { p.began = true; p.beginTime = now; }
        if (!confirmed(n) && now - p.beginTime < AUTH_GRACE_MS) continue;
        std::string reason = p.blacklist ? g_vpnBlockerBanMessageBlacklist.string : g_vpnBlockerBanMessageVPN.string;
        if (reason.empty()) reason = p.blacklist ? "This address is blocked by the server's VPN policy."
            : "VPN/proxy connections are not allowed on this server.";
        p.blocked = false;
        G_Printf("[VPN Blocker] Client %d: %s; disconnecting.\n", n, p.blacklist ? "IP blacklist match" : "VPN/proxy detected");
        trap_DropClient(n, reason.c_str(), 0);
    }
}
}
