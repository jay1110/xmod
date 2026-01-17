#include <bgame/impl.h>
#include <game/xmod_globals.h>
#include <cctype>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

DbLoad::DbLoad()
    : AbstractBuiltin( "dbload" )
{
    __usage << xvalue( "!" + _name ) << " [migrate]";
    __descr << "Reload the Admin System database files. Use 'migrate' to import legacy userDB to SQLite.";
}

///////////////////////////////////////////////////////////////////////////////

DbLoad::~DbLoad()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
DbLoad::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    bool doMigration = false;
    if (txt._args.size() == 2) {
        string arg = txt._args[1];
        // Convert to lowercase manually
        for (size_t i = 0; i < arg.length(); i++) {
            arg[i] = tolower(arg[i]);
        }
        if (arg == "migrate" || arg == "import") {
            doMigration = true;
        } else {
            return PA_USAGE;
        }
    }

    if (!doMigration) {
        G_DbLoad();
    }
    
    // Get counts from SQLite
    int userCount = 0;
    int banCount = 0;
    if (::xmod::g_database && ::xmod::g_database->isOpened()) {
        if (doMigration) {
            // Perform migration
            int migrated = ::xmod::g_database->importFromLegacyUserDB();
            Buffer buf;
            buf << "^2Migrated " << xvalue(migrated) << " users from legacy userDB to SQLite";
            printCpm( txt._client, buf, true );
            return PA_NONE;
        }
        
        userCount = ::xmod::g_database->getUserCount();
        banCount = ::xmod::g_database->getBanCount();
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
