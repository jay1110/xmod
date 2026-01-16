#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Chicken::Chicken()
    : AbstractBuiltin( "chicken" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Call a player a chicken.";
}

///////////////////////////////////////////////////////////////////////////////

Chicken::~Chicken()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Chicken::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    G_globalSound( "sound/xmod/chicken.wav" );

    // Phase 5: Use session-aware helper for player name
    const std::string& namex = getPlayerNamex(target->slot);
    Buffer buf;
    buf << _name << ": "
        << xcvalue << xnone( namex ) << " is running around like a headless chicken!\n";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
