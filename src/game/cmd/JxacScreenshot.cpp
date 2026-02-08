#include <bgame/impl.h>
#include <game/jxac/jxac_server.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

JxacScreenshot::JxacScreenshot()
    : AbstractBuiltin( "ss" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "QUALITY" );
    __descr << "Request screenshot from player (JXAC).";
}

///////////////////////////////////////////////////////////////////////////////

JxacScreenshot::~JxacScreenshot()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
JxacScreenshot::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    // Get quality parameter (optional)
    int quality = JXAC_SS_QUALITY_DEFAULT;
    if (txt._args.size() > 2) {
        quality = atoi( txt._args[2].c_str() );
        if (quality < JXAC_SS_QUALITY_MIN || quality > JXAC_SS_QUALITY_MAX) {
            txt._ebuf << "Quality must be between " << JXAC_SS_QUALITY_MIN << " and " << JXAC_SS_QUALITY_MAX << ".";
            return PA_ERROR;
        }
    }

    // Request screenshot
    jxac::Server::requestScreenshot( target->slot, quality );

    Buffer buf;
    buf << _name << ": Screenshot requested from " << xvalue( target->gentity.client->pers.netname ) 
        << " (quality: " << quality << ")";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
