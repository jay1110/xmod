#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

DbSave::DbSave()
    : AbstractBuiltin( "dbsave" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Save the Admin System database (SQLite auto-saves).";
}

///////////////////////////////////////////////////////////////////////////////

DbSave::~DbSave()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
DbSave::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    // Save legacy level database
    levelDB.save();
    
    // SQLite database auto-saves, but we can report counts
    int userCount = 0;
    int banCount = 0;
    if (xmod::g_database && xmod::g_database->isOpened()) {
        userCount = xmod::g_database->getUserCount();
        banCount = xmod::g_database->getBanCount();
    }

    Buffer buf;
    buf << "saved: " << xvalue( int(levelDB.mapLEVEL.size()) ) << " levels\n"
        << "SQLite: " << xvalue( userCount ) << " users, " << xvalue( banCount ) << " bans\n";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
