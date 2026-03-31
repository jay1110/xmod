#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

AddXp::AddXp()
    : AbstractBuiltin( "addxp" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << xvalue( "AMOUNT" );
    __descr << "Add XP to a player's existing XP, divided evenly across all skills.";
}

///////////////////////////////////////////////////////////////////////////////

AddXp::~AddXp()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
AddXp::doExecute( Context& txt )
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

    // Divide XP evenly across all skills and add to existing
    if (amount > 0) {
        const int base = amount / SK_NUM_SKILLS;
        const int remainder = amount % SK_NUM_SKILLS;
        const int threshold = SK_NUM_SKILLS - remainder;

        for (int i = 0; i < SK_NUM_SKILLS; i++) {
            const float xp = (i >= threshold)
                ? static_cast<float>(base + 1)
                : static_cast<float>(base);
            target->gclient.sess.skillpoints[i] += xp;
        }
    }

    // Recalculate skill levels and rank
    G_CalcRank( &target->gclient );
    ClientUserinfoChanged( target->slot );

    const std::string& targetNamex = getPlayerNamex(target->slot);
    Buffer buf;
    buf << _name << ": " << xvalue( amount ) << " XP added to " << xvalue( targetNamex ) << '.';
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
