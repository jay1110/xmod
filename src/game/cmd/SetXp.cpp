#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

SetXp::SetXp()
    : AbstractBuiltin( "setxp" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << xvalue( "AMOUNT" );
    __descr << "Set a player's XP to the given amount, divided evenly across all skills.";
}

///////////////////////////////////////////////////////////////////////////////

SetXp::~SetXp()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
SetXp::doExecute( Context& txt )
{
    if (txt._args.size() != 3)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    const int amount = atoi( txt._args[2].c_str() );
    if (amount < 0) {
        txt._ebuf << "AMOUNT must not be negative.";
        return PA_ERROR;
    }

    // Reset all skillpoints and skill levels
    for (int i = 0; i < SK_NUM_SKILLS; i++) {
        target->gclient.sess.skillpoints[i] = 0.0f;
        target->gclient.sess.skill[i] = 0;
    }

    // Divide XP evenly across all skills
    if (amount > 0) {
        const int base = amount / SK_NUM_SKILLS;
        const int remainder = amount % SK_NUM_SKILLS;
        const int threshold = SK_NUM_SKILLS - remainder;

        for (int i = 0; i < SK_NUM_SKILLS; i++) {
            target->gclient.sess.skillpoints[i] = (i >= threshold)
                ? static_cast<float>(base + 1)
                : static_cast<float>(base);
        }
    }

    // Recalculate skill levels and rank
    G_CalcRank( &target->gclient );
    ClientUserinfoChanged( target->slot );

    const std::string& targetNamex = getPlayerNamex(target->slot);
    Buffer buf;
    buf << _name << ": " << xvalue( targetNamex ) << "'s XP has been set to " << xvalue( amount ) << '.';
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
