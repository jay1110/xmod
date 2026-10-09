#include <bgame/impl.h>
#include <game/jxac/jxac_server.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

JxacStatus::JxacStatus()
    : AbstractBuiltin( "jxac_status" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "PLAYER" );
    __descr << "Show JXAC status for all players or a specific player.";
}

///////////////////////////////////////////////////////////////////////////////

JxacStatus::~JxacStatus()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
JxacStatus::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    if (txt._args.size() == 1) {
        // Show status for all players
        jxac::Server::printStatusAll(txt._client ? txt._client->slot : -1);
    } else {
        // Show status for specific player
        Client* target;
        if (lookupPLAYER( txt._args[1], txt, target ))
            return PA_ERROR;

        jxac::Server::printStatus(target->slot, txt._client ? txt._client->slot : -1);
    }

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
