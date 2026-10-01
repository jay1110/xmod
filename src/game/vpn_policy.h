#ifndef GAME_VPN_POLICY_H
#define GAME_VPN_POLICY_H

#include <cstdint>
#include <string>
#include <vector>

namespace vpnblocker { namespace policy {

enum class List { Whitelist, Blacklist };
enum class Decision { None, Whitelist, Blacklist };
struct IpEntry {
    int id = 0;
    List list = List::Whitelist;
    std::string first;
    std::string last;
    std::string reason;
};
struct GuidEntry {
    std::string guid;
    std::string reason;
    bool legacy = false; // retained Nitmod 32-character GUIDs never authorize Xmod
};
struct Counts {
    bool available = false;
    int whitelist = 0;
    int blacklist = 0;
    int guids = 0;
    int legacyGuids = 0;
};

// Main engine thread only; bounded in-memory lookups after transactional loading.
// physicalPath must already be resolved under the server's home/game directory.
bool open(const std::string& physicalPath, std::string& error);
void close();
bool reload(std::string& error); // retain the last valid snapshot on failure
Counts counts();
std::uint64_t revision(); // increments after a successful load/mutation/close
Decision lookupIp(const std::string& ip);
bool containsGuid(const std::string& confirmedGuid);
std::string normalizeGuid(const std::string& guid); // native 40-hex only

// Pages are one-based and limited to 8 entries. False returns a public-safe error.
bool listIps(List list, int page, std::vector<IpEntry>& entries, int& total, std::string& error);
bool addIp(List list, const std::string& first, const std::string& last,
    const std::string& reason, int& id, std::string& error); // empty last = single IP
bool removeIp(List list, int id, std::string& error);
bool listGuids(int page, std::vector<GuidEntry>& entries, int& total, std::string& error);
bool addGuid(const std::string& nativeGuid, const std::string& reason, std::string& error);
bool removeGuid(const std::string& guid, std::string& error); // may delete retained legacy GUIDs

} }
#endif
