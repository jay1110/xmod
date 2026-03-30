#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Nospam::Nospam()
    : AbstractBuiltin( "nospam" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "TIME" );
    __descr << "Limit a player to 1 message or voice command per minute. Time is optional (e.g. 5m, 1h). Without time it is permanent.";
}

///////////////////////////////////////////////////////////////////////////////

Nospam::~Nospam()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Nospam::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    const std::string& targetNamex = getPlayerNamex(target->slot);

    // Self-nospam check using GUID comparison
    if (txt._client) {
        const std::string& targetGuid = getPlayerGuid(target->slot);
        const std::string& userGuid = getPlayerGuid(txt._client->slot);
        if (!targetGuid.empty() && !userGuid.empty() && targetGuid == userGuid) {
            txt._ebuf << "You cannot nospam yourself.";
            return PA_ERROR;
        }
    }

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    // bail if already nospammed
    if (::xmod::isClientNospammed(target->slot)) {
        txt._ebuf << "Player is already nospammed.";
        return PA_ERROR;
    }

    // parse optional time parameter
    time_t nospamExpiry = 0;
    if (txt._args.size() > 2) {
        int seconds = str::toSeconds( txt._args[2] );
        if (seconds > 0)
            nospamExpiry = time(NULL) + seconds;
    }

    G_NospamPlayer( &target->gentity, nospamExpiry );
    trap_SendServerCommand( target->slot, "cp \"^xYou've been nospammed.\n^7You can only send 1 message per minute.\n\"" );

    Buffer buf;
    buf << _name << ": " << xvalue( targetNamex ) << " was nospammed.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
