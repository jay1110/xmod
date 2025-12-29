#include <bgame/impl.h>
#include <game/jxac/jxac_server.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

JxacScreenshotAll::JxacScreenshotAll()
    : AbstractBuiltin( "jxac_screenshotall" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "QUALITY" );
    __descr << "Request screenshot from all connected players (JXAC).";
}

///////////////////////////////////////////////////////////////////////////////

JxacScreenshotAll::~JxacScreenshotAll()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
JxacScreenshotAll::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    // Get quality parameter (optional)
    int quality = JXAC_SS_QUALITY_DEFAULT;
    if (txt._args.size() > 1) {
        quality = atoi( txt._args[1].c_str() );
        if (quality < JXAC_SS_QUALITY_MIN || quality > JXAC_SS_QUALITY_MAX) {
            txt._ebuf << "Quality must be between " << JXAC_SS_QUALITY_MIN << " and " << JXAC_SS_QUALITY_MAX << ".";
            return PA_ERROR;
        }
    }

    // Request screenshots from all players
    jxac::Server::requestScreenshotAll( quality );

    Buffer buf;
    buf << _name << ": Screenshots requested from all players (quality: " << quality << ")";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
