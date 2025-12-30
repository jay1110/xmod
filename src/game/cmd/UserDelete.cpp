#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

UserDelete::UserDelete()
    : AbstractBuiltin( "userdelete" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" );
    __descr << "Remove the user specified by " << xvalue( "USERID" );
}

///////////////////////////////////////////////////////////////////////////////

UserDelete::~UserDelete()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
UserDelete::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    if (!xmod::g_database || !xmod::g_database->isOpened()) {
        txt._ebuf << "Database not available.";
        return PA_ERROR;
    }

    // bail on invalid id
    const string& id = txt._args[1];
    
    // Try to parse as numeric ID first
    int userId = atoi(id.c_str());
    
    bool deleted = false;
    if (userId > 0) {
        // Get user data to display name
        xmod::UserData userData;
        if (xmod::g_database->getUserDataById(userId, userData)) {
            deleted = xmod::g_database->deleteUser(userId);
            if (deleted) {
                Buffer buf;
                buf << _name << ": User ID " << xvalue( id ) << " (" << xvalue( userData.name ) << ") removed.";
                printCpm( txt._client, buf, true );
                return PA_NONE;
            }
        }
    }
    
    // Try as GUID
    if (!deleted) {
        xmod::UserData userData;
        if (xmod::g_database->getUserData(id, userData)) {
            deleted = xmod::g_database->deleteUserByGuid(id);
            if (deleted) {
                Buffer buf;
                buf << _name << ": User " << xvalue( id ) << " (" << xvalue( userData.name ) << ") removed.";
                printCpm( txt._client, buf, true );
                return PA_NONE;
            }
        }
    }
    
    txt._ebuf << "User not found.";
    return PA_ERROR;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
