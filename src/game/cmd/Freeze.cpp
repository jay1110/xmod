#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Freeze::Freeze()
    : AbstractBuiltin( "freeze" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "TIME" );
    __descr << "Freeze a player so they cannot move. Optionally specify a duration for auto-unfreeze.";
}

///////////////////////////////////////////////////////////////////////////////

Freeze::~Freeze()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Freeze::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    if (txt._args.size() > 3)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    const std::string& targetNamex = getPlayerNamex(target->slot);

    // Self-freeze check
    if (txt._client) {
        const std::string& targetGuid = getPlayerGuid(target->slot);
        const std::string& userGuid = getPlayerGuid(txt._client->slot);
        if (!targetGuid.empty() && !userGuid.empty() && targetGuid == userGuid) {
            txt._ebuf << "You cannot freeze yourself.";
            return PA_ERROR;
        }
    }

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    if (isNotOnTeamError( *target, txt ))
        return PA_ERROR;

    // Bail if already frozen
    if (target->frozen) {
        txt._ebuf << "Player is already frozen.";
        return PA_ERROR;
    }

    // Parse optional time
    int duration = 0;
    if (txt._args.size() > 2) {
        duration = str::toSeconds( txt._args[2] );
        if (duration < 1) {
            txt._ebuf << "Invalid duration.";
            return PA_ERROR;
        }
    }

    // Freeze the player
    target->frozen = true;
    if (duration > 0)
        target->frozenExpiry = time(NULL) + duration;
    else
        target->frozenExpiry = 0; // indefinite

    // Zero out velocity immediately
    VectorClear( target->gentity.client->ps.velocity );

    trap_SendServerCommand( target->slot, "cp \"^xYou've been frozen.\n\"" );

    Buffer buf;
    buf << _name << ": " << xvalue( targetNamex ) << " was frozen.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
