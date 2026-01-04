#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Unban::Unban()
    : AbstractBuiltin( "unban" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" );
    __descr << "Unban a specific player.";
}

///////////////////////////////////////////////////////////////////////////////

Unban::~Unban()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Unban::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    const string& id = txt._args[1];
    
    if (xmod::g_database && xmod::g_database->isOpened()) {
        // First try as numeric ban ID (this is what !banlist shows)
        int banId = atoi(id.c_str());
        if (banId > 0 && xmod::g_database->unbanById(banId)) {
            Buffer buf;
            buf << _name << ": Ban ID " << xvalue( id ) << " removed.";
            printCpm( txt._client, buf, true );
            return PA_NONE;
        }
        
        // Then try to unban by GUID (for advanced users)
        if (xmod::g_database->unbanUser(id)) {
            Buffer buf;
            buf << _name << ": User with GUID " << xvalue( id ) << " unbanned.";
            printCpm( txt._client, buf, true );
            return PA_NONE;
        }
    }
    
    txt._ebuf << "Ban not found. Use !banlist to see valid ban IDs.";
    return PA_ERROR;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
