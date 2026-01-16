#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

CryBaby::CryBaby()
    : AbstractBuiltin( "crybaby" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Make a player cry like a baby.";
}

///////////////////////////////////////////////////////////////////////////////

CryBaby::~CryBaby()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
CryBaby::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    G_globalSound( "sound/xmod/crybaby.wav" );

    // Phase 5: Use session-aware helper for player name
    const std::string& namex = getPlayerNamex(target->slot);
    Buffer buf;
    buf << _name << ": "
        << xcvalue << xnone( namex ) << " is crying like a little baby!\n";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
