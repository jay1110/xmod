#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Unfreeze::Unfreeze()
    : AbstractBuiltin( "unfreeze" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Unfreeze a player so they can move again.";
}

///////////////////////////////////////////////////////////////////////////////

Unfreeze::~Unfreeze()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Unfreeze::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    const std::string& targetNamex = getPlayerNamex(target->slot);

    // Bail if not frozen
    if (!target->frozen) {
        txt._ebuf << "Player is not frozen.";
        return PA_ERROR;
    }

    // Unfreeze the player
    target->frozen = false;
    target->frozenExpiry = 0;

    trap_SendServerCommand( target->slot, "cp \"^xYou've been unfrozen.\n\"" );

    Buffer buf;
    buf << _name << ": " << xvalue( targetNamex ) << " was unfrozen.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
