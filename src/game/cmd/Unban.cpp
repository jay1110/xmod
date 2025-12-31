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

    // bail on invalid id
    const string& id = txt._args[1];
    
    // Try to parse as GUID or ban ID
    if (xmod::g_database && xmod::g_database->isOpened()) {
        // First try to unban by GUID
        if (xmod::g_database->unbanUser(id)) {
            Buffer buf;
            buf << _name << ": User " << xvalue( id ) << " unbanned.";
            printCpm( txt._client, buf, true );
            return PA_NONE;
        }
        
        // Try as numeric ban ID
        int banId = atoi(id.c_str());
        if (banId > 0 && xmod::g_database->unbanById(banId)) {
            Buffer buf;
            buf << _name << ": Ban ID " << xvalue( id ) << " removed.";
            printCpm( txt._client, buf, true );
            return PA_NONE;
        }
    }
    
    txt._ebuf << "User not found or unable to unban.";
    return PA_ERROR;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
