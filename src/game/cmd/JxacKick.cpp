#include <bgame/impl.h>
#include <game/jxac/jxac_server.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

JxacKick::JxacKick()
    : AbstractBuiltin( "jxac_kick" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "REASON" );
    __descr << "Kick a player detected by JXAC.";
}

///////////////////////////////////////////////////////////////////////////////

JxacKick::~JxacKick()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
JxacKick::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    // Compute reason text
    string kickReason = "JXAC Violation";
    if (txt._args.size() > 2)
        str::concatArgs( txt._args, kickReason, 2 );

    Buffer buf;
    buf << _name << ": Kicking " << xvalue( target->gentity.client->pers.netname ) 
        << " (" << kickReason << ")";
    printCpm( txt._client, buf, true );

    // Kick player
    jxac::Server::kickPlayer( target->slot, kickReason.c_str() );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
