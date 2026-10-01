#include <bgame/impl.h>
#include <game/vpn_globals.h>

namespace cmd {

Vpn::Vpn() : AbstractBuiltin("vpn")
{
    __usage << xvalue("!vpn") << ' ' << _ovalue("status|on|off|reload")
        << '\n' << xvalue("!vpn check") << ' ' << xvalue("PLAYER|all");
    __descr << "Manage the VPN blocker or queue fresh background checks. "
        "Status never displays API keys; bots, private addresses and exempt admins are skipped.";
}

Vpn::~Vpn() {}

bool Vpn::hasPermission(const Context& txt)
{
    if (!AbstractBuiltin::hasPermission(txt)) return false;
    if (!txt._client) return true;
    const int slot = txt._client->slot;
    if (slot < 0 || slot >= MAX_CLIENTS || !g_entities[slot].inuse ||
        !g_entities[slot].client || (g_entities[slot].r.svFlags & SVF_BOT) ||
        g_entities[slot].client->pers.connected != CON_CONNECTED) return false;
    const User* user = connectedUsers[slot];
    const std::string& guid = g_clientObjects[slot].authGuid;
    if (!g_clientObjects[slot].authenticated || !user || user == &User::BAD ||
        user->fakeguid || user->guid != guid || txt._user.guid != guid || guid.size() != 40) return false;
    for (char c : guid) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F'))) return false;
    }
    return true;
}

AbstractCommand::PostAction Vpn::doExecute(Context& txt)
{
    const std::string action = txt._args.size() > 1
        ? str::toLowerCopy(txt._args[1]) : "status";
    if (action == "status") {
        if (txt._args.size() > 2) return PA_USAGE;
        const vpnblocker::Status s = vpnblocker::status();
        Buffer buf;
        buf << xheader("-VPN BLOCKER")
            << '\n' << "Enabled: " << xvalue(s.enabled ? "yes" : "no")
            << "; platform available: " << xvalue(s.available ? "yes" : "no")
            << '\n' << "Providers configured: vpnapi.io=" << xvalue(s.provider1 ? "yes" : "no")
            << ", ipapi.is=" << xvalue(s.provider2 ? "yes" : "no")
            << '\n' << "Exempt authenticated admin levels: above " << xvalue(s.maxLevel)
            << '\n' << "Queued: " << xvalue(s.queued) << "; allowed: " << xvalue(s.allowed)
            << "; blocked awaiting connection/authentication: " << xvalue(s.blocked)
            << '\n' << "Unavailable results: " << xvalue(s.unavailable)
            << "; exempt: " << xvalue(s.exempt) << "; bots/local/private: " << xvalue(s.skipped)
            << '\n' << "Policy database: " << xvalue(s.database ? "loaded" : "unavailable/disabled")
            << "; IP whitelist: " << xvalue(s.ipWhitelist) << "; IP blacklist: " << xvalue(s.ipBlacklist)
            << '\n' << "Native GUID exceptions: " << xvalue(s.guidWhitelist)
            << "; retained inactive Nitmod GUIDs: " << xvalue(s.legacyGuids);
        if (!s.databaseError.empty()) buf << '\n' << xfail(s.databaseError);
        print(txt._client, buf);
        return PA_NONE;
    }
    if (action == "on" || action == "off") {
        if (txt._args.size() != 2) return PA_USAGE;
        vpnblocker::setEnabled(action == "on");
        const vpnblocker::Status s = vpnblocker::status();
        Buffer buf;
        buf << "vpn: " << xvalue(s.enabled ? "enabled" : "disabled") << '.';
        if (s.enabled && !s.available) buf << " Online lookups are unavailable on this platform.";
        else if (s.enabled && !s.provider1 && !s.provider2)
            buf << " No API provider is configured; only local policy rules are active.";
        printChat(txt._client, buf);
        return PA_NONE;
    }
    if (action == "reload") {
        if (txt._args.size() != 2) return PA_USAGE;
        std::string error;
        if (!vpnblocker::reloadPolicy(error)) { txt._ebuf << error; return PA_ERROR; }
        Buffer buf;
        buf << "vpn: policy database reloaded; connected players will be re-evaluated.";
        printChat(txt._client, buf);
        return PA_NONE;
    }
    if (action != "check" || txt._args.size() != 3) return PA_USAGE;
    const bool all = str::toLowerCopy(txt._args[2]) == "all";
    const vpnblocker::Status s = vpnblocker::status();
    if (!s.enabled) {
        txt._ebuf << "The VPN blocker is disabled. Use !vpn on.";
        return PA_ERROR;
    }
    if (all) {
        int queued = 0, skipped = 0, unavailable = 0;
        for (int n = 0; n < MAX_CLIENTS; ++n) {
            if (!g_entities[n].client || g_entities[n].client->pers.connected != CON_CONNECTED)
                continue;
            const vpnblocker::Recheck result = vpnblocker::recheck(n);
            if (result == vpnblocker::Recheck::Queued) ++queued;
            else if (result == vpnblocker::Recheck::NoProvider || result == vpnblocker::Recheck::Unavailable) ++unavailable;
            else ++skipped;
        }
        Buffer buf;
        buf << "vpn: " << xvalue(queued) << " fresh background checks queued; "
            << xvalue(skipped) << " exempt, bot or local/private clients skipped; "
            << xvalue(unavailable) << " clients without a matching local rule or online provider.";
        printChat(txt._client, buf);
        return PA_NONE;
    }
    Client* target;
    if (lookupPLAYER(txt._args[2], txt, target)) return PA_ERROR;
    const vpnblocker::Recheck result = vpnblocker::recheck(target->slot);
    if (result != vpnblocker::Recheck::Queued) {
        txt._ebuf << (result == vpnblocker::Recheck::Exempt
            ? "This authenticated player is exempt by admin level or GUID whitelist."
            : result == vpnblocker::Recheck::Skipped
            ? "Bots and local/private/reserved addresses are not checked."
            : result == vpnblocker::Recheck::NoProvider || result == vpnblocker::Recheck::Unavailable
            ? "No matching local policy rule or online provider is available."
            : "The player cannot currently be checked.");
        return PA_ERROR;
    }
    Buffer buf;
    buf << "vpn: fresh background check queued for " << xvalue(getPlayerNamex(target->slot))
        << ". A positive result disconnects this player under the usual exemption rules.";
    printChat(txt._client, buf);
    return PA_NONE;
}

} // namespace cmd
