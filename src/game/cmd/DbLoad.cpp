#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

DbLoad::DbLoad()
    : AbstractBuiltin( "dbload" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Reload the Admin System database files.";
}

///////////////////////////////////////////////////////////////////////////////

DbLoad::~DbLoad()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
DbLoad::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    G_DbLoad();
    
    // Get counts from SQLite
    int userCount = 0;
    int banCount = 0;
    if (xmod::g_database && xmod::g_database->isOpened()) {
        userCount = xmod::g_database->getUserCount();
        banCount = xmod::g_database->getBanCount();
    }

    Buffer buf;
    buf << "loaded: " << xvalue( int(levelDB.mapLEVEL.size()) ) << " level records"
        << '\n' << "SQLite: " << xvalue( userCount ) << " users, " << xvalue( banCount ) << " bans"
        << '\n' << "loaded: " << xvalue( int(mapDB.mapNAME.size()) ) << " map records";

    if (g_censor.integer)
        buf << '\n' << "loaded: " << xvalue( int(censorDB.wordSet.size()) ) << " censor records";

    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
