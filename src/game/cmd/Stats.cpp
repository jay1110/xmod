#include <bgame/impl.h>

namespace cmd {
Stats::Stats() : AbstractBuiltin("stats", true) {
    __usage << xvalue("!stats") << " [" << xvalue("PLAYER") << ']';
    __descr << "Show current round statistics for yourself or a player (name/slot).";
}

AbstractCommand::PostAction Stats::doExecute(Context& txt) {
    if (txt._args.size() > 2) return PA_USAGE;
    Client* target = txt._client;
    if (txt._args.size() == 2 && lookupPLAYER(txt._args[1], txt, target)) return PA_ERROR;
    if (!target) return PA_USAGE;

    const gclient_t& cl = target->gclient;
    int hits = 0, shots = 0;
    for (int i = 0; i < WS_MAX; ++i) {
        hits += cl.sess.aWeaponStats[i].hits;
        shots += cl.sess.aWeaponStats[i].subshots ? cl.sess.aWeaponStats[i].subshots : cl.sess.aWeaponStats[i].atts;
    }
    const int accuracy = shots > 0 ? (int)(100.0 * hits / shots) : 0;
    Buffer buf;
    buf << xheader("-ROUND STATISTICS") << '\n' << xvalue(cl.pers.netname)
        << '\n' << "Kills: " << xvalue(cl.sess.kills) << "  Deaths: " << xvalue(cl.sess.deaths)
        << "  Headshots: " << xvalue(cl.sess.headshots)
        << '\n' << "Spree: " << xvalue(cl.pers.killspreekills)
        << "  Best spree: " << xvalue(cl.pers.roundAwards.bestSpree)
        << '\n' << "Damage given: " << xvalue(cl.sess.damage_given)
        << "  Received: " << xvalue(cl.sess.damage_received)
        << '\n' << "Accuracy: " << xvalue(accuracy) << "% (" << xvalue(hits) << '/' << xvalue(shots) << ')'
        << '\n' << "Revives: " << xvalue(cl.sess.aWeaponStats[WS_SYRINGE].hits)
        << "  Best revive spree: " << xvalue(cl.pers.roundAwards.bestReviveSpree);
    print(txt._client, buf);
    return PA_NONE;
}
}
