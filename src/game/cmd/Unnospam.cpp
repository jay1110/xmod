#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Unnospam::Unnospam()
    : AbstractBuiltin( "unnospam" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Remove nospam restriction from a player.";
}

///////////////////////////////////////////////////////////////////////////////

Unnospam::~Unnospam()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Unnospam::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    const std::string& targetNamex = getPlayerNamex(target->slot);

    // bail if not nospammed
    if (!::xmod::isClientNospammed(target->slot)) {
        txt._ebuf << "Player is not nospammed.";
        return PA_ERROR;
    }

    if (!G_UnnospamPlayer(&target->gentity)) {
        txt._ebuf << "Unable to remove nospam from SQLite.";
        return PA_ERROR;
    }
    trap_SendServerCommand( target->slot, "cp \"^xYour nospam restriction has been removed.\n\"" );

    Buffer buf;
    buf << _name << ": " << xvalue( targetNamex ) << " was unnospammed.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
