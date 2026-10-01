#include <bgame/impl.h>
#include <game/vpn_globals.h>
#include <game/vpn_policy.h>

namespace cmd {
namespace {

bool positiveNumber(const std::string& text, int& value)
{
    if (text.empty()) return false;
    value = 0;
    for (char c : text) {
        if (c < '0' || c > '9' || value > (2147483647 - (c - '0')) / 10) return false;
        value = value * 10 + c - '0';
    }
    return value > 0;
}

std::string reasonFrom(const std::vector<std::string>& args, size_t first)
{
    std::string reason;
    for (size_t i = first; i < args.size(); ++i) {
        if (i != first) reason += ' ';
        reason += args[i];
    }
    return reason;
}

// Imported reasons are untrusted text; keep each database row on one line.
std::string displayReason(const std::string& reason)
{
    std::string result;
    for (unsigned char c : reason) {
        if (result.size() >= 120) { result += "..."; break; }
        result += c < 32 || c == 127 ? ' ' : static_cast<char>(c);
    }
    return result;
}

} // namespace

VpnListCommand::VpnListCommand(const char* name) : AbstractBuiltin(name) {}

bool VpnListCommand::hasPermission(const Context& txt)
{
    if (!AbstractBuiltin::hasPermission(txt)) return false;
    if (!txt._client) return true;
    const int slot = txt._client->slot;
    if (slot < 0 || slot >= MAX_CLIENTS || !g_entities[slot].inuse ||
        !g_entities[slot].client || (g_entities[slot].r.svFlags & SVF_BOT) ||
        g_entities[slot].client->pers.connected != CON_CONNECTED) return false;
    const User* user = connectedUsers[slot];
    const std::string& guid = g_clientObjects[slot].authGuid;
    return g_clientObjects[slot].authenticated && user && user != &User::BAD &&
        !user->fakeguid && user->guid == guid && txt._user.guid == guid &&
        !vpnblocker::policy::normalizeGuid(guid).empty();
}

VpnIpList::VpnIpList(const char* name, bool blacklist)
    : VpnListCommand(name), _blacklist(blacklist)
{
    __usage << xvalue(std::string("!") + name) << ' ' << xvalue("show") << ' ' << _ovalue("PAGE")
        << '\n' << xvalue(std::string("!") + name) << ' ' << xvalue("add-ip IP REASON")
        << '\n' << xvalue(std::string("!") + name) << ' ' << xvalue("add-ip-range START END REASON")
        << '\n' << xvalue(std::string("!") + name) << ' ' << xvalue("remove-entry ID");
    __descr << (blacklist ? "Manage persistent IPv4 blocks and ranges. "
        : "Manage persistent IPv4 exemptions and ranges. ")
        << "Changes are saved immediately and connected players are re-evaluated. "
        << "Whitelist rules take priority over blacklist rules.";
}

Whitelist::Whitelist() : VpnIpList("whitelist", false) {}
Blacklist::Blacklist() : VpnIpList("blacklist", true) {}

AbstractCommand::PostAction VpnIpList::doExecute(Context& txt)
{
    vpnblocker::status(); // load the configured server database, also while disabled
    const auto list = _blacklist ? vpnblocker::policy::List::Blacklist : vpnblocker::policy::List::Whitelist;
    const std::string name = _blacklist ? "blacklist" : "whitelist";
    const std::string action = txt._args.size() > 1 ? str::toLowerCopy(txt._args[1]) : "show";
    std::string error;
    if (action == "show") {
        int page = 1;
        if (txt._args.size() > 3) return PA_USAGE;
        if (txt._args.size() == 3 && !positiveNumber(txt._args[2], page)) return PA_USAGE;
        std::vector<vpnblocker::policy::IpEntry> entries;
        int total = 0;
        if (!vpnblocker::policy::listIps(list, page, entries, total, error)) {
            txt._ebuf << error; return PA_ERROR;
        }
        Buffer buf;
        buf << xheader(_blacklist ? "-VPN BLACKLIST" : "-VPN WHITELIST")
            << " Page " << xvalue(page) << "/" << xvalue(total ? (total + 7) / 8 : 1)
            << "; " << xvalue(total) << " entries";
        for (const auto& entry : entries) {
            buf << '\n' << xvalue(entry.id) << "  " << xvalue(entry.first);
            if (entry.last != entry.first && !entry.last.empty()) buf << " - " << xvalue(entry.last);
            buf << "  " << displayReason(entry.reason);
        }
        print(txt._client, buf);
        return PA_NONE;
    }
    if (action == "add-ip" || action == "add-ip-range") {
        const size_t reasonStart = action == "add-ip" ? 3 : 4;
        if (txt._args.size() <= reasonStart) return PA_USAGE;
        int id = 0;
        if (!vpnblocker::policy::addIp(list, txt._args[2],
                reasonStart == 4 ? txt._args[3] : "", reasonFrom(txt._args, reasonStart), id, error)) {
            txt._ebuf << error; return PA_ERROR;
        }
        Buffer buf;
        buf << name << ": saved entry " << xvalue(id) << '.';
        printChat(txt._client, buf);
        return PA_NONE;
    }
    if (action == "remove-entry") {
        int id;
        if (txt._args.size() != 3 || !positiveNumber(txt._args[2], id)) return PA_USAGE;
        if (!vpnblocker::policy::removeIp(list, id, error)) {
            txt._ebuf << error; return PA_ERROR;
        }
        Buffer buf;
        buf << name << ": removed entry " << xvalue(id) << '.';
        printChat(txt._client, buf);
        return PA_NONE;
    }
    return PA_USAGE;
}

NguidList::NguidList() : VpnListCommand("nguidlist")
{
    __usage << xvalue("!nguidlist show") << ' ' << _ovalue("PAGE")
        << '\n' << xvalue("!nguidlist add-nguid GUID REASON")
        << '\n' << xvalue("!nguidlist remove-nguid GUID");
    __descr << "Manage persistent VPN exemptions for confirmed 40-character Xmod GUIDs. "
        "Imported 32-character Nitmod GUIDs remain visible but cannot exempt Xmod players.";
}

AbstractCommand::PostAction NguidList::doExecute(Context& txt)
{
    vpnblocker::status();
    const std::string action = txt._args.size() > 1 ? str::toLowerCopy(txt._args[1]) : "show";
    std::string error;
    if (action == "show") {
        int page = 1;
        if (txt._args.size() > 3) return PA_USAGE;
        if (txt._args.size() == 3 && !positiveNumber(txt._args[2], page)) return PA_USAGE;
        std::vector<vpnblocker::policy::GuidEntry> entries;
        int total = 0;
        if (!vpnblocker::policy::listGuids(page, entries, total, error)) {
            txt._ebuf << error; return PA_ERROR;
        }
        Buffer buf;
        buf << xheader("-VPN GUID WHITELIST") << " Page " << xvalue(page) << "/"
            << xvalue(total ? (total + 7) / 8 : 1) << "; " << xvalue(total) << " entries";
        for (const auto& entry : entries)
            buf << '\n' << xvalue(entry.guid) << (entry.legacy ? " [legacy/inactive] " : " ")
                << displayReason(entry.reason);
        print(txt._client, buf);
        return PA_NONE;
    }
    if (action == "add-nguid") {
        if (txt._args.size() < 4) return PA_USAGE;
        if (!vpnblocker::policy::addGuid(txt._args[2], reasonFrom(txt._args, 3), error)) {
            txt._ebuf << error; return PA_ERROR;
        }
        Buffer buf;
        buf << "nguidlist: saved Xmod GUID exemption.";
        printChat(txt._client, buf);
        return PA_NONE;
    }
    if (action == "remove-nguid") {
        if (txt._args.size() != 3) return PA_USAGE;
        if (!vpnblocker::policy::removeGuid(txt._args[2], error)) {
            txt._ebuf << error; return PA_ERROR;
        }
        Buffer buf;
        buf << "nguidlist: removed GUID exemption.";
        printChat(txt._client, buf);
        return PA_NONE;
    }
    return PA_USAGE;
}

VpnCheck::VpnCheck() : VpnListCommand("vpn-check")
{
    __usage << xvalue("!vpn-check IP");
    __descr << "Check one public IPv4 address against local rules and fresh background API results. "
        "Reports to the requesting admin; does not disconnect players.";
}

AbstractCommand::PostAction VpnCheck::doExecute(Context& txt)
{
    if (txt._args.size() != 2) return PA_USAGE;
    std::string error;
    if (!vpnblocker::requestIpCheck(txt._client ? txt._client->slot : -1, txt._args[1], error)) {
        txt._ebuf << error; return PA_ERROR;
    }
    return PA_NONE;
}

} // namespace cmd
