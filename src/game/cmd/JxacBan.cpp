#include <bgame/impl.h>
#include <game/jxac/jxac_server.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

JxacBan::JxacBan()
    : AbstractBuiltin( "jxac_ban" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "REASON" );
    __descr << "Ban a player detected by JXAC.";
}

///////////////////////////////////////////////////////////////////////////////

JxacBan::~JxacBan()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
JxacBan::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    // Compute reason text
    string banReason = "JXAC Violation - Banned";
    if (txt._args.size() > 2)
        str::concatArgs( txt._args, banReason, 2 );

    Buffer buf;
    buf << _name << ": Banning " << xvalue( target->gentity.client->pers.netname ) 
        << " (" << banReason << ")";
    printCpm( txt._client, buf, true );

    // Ban player
    jxac::Server::banPlayer( target->slot, banReason.c_str() );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
