#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Seen::Seen()
    : AbstractBuiltin( "seen" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "NAME" );
    __descr << "Find the last time a specific admin was seen on the server.";
}

///////////////////////////////////////////////////////////////////////////////

Seen::~Seen()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Seen::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    if (!::xmod::g_database || !::xmod::g_database->isOpened()) {
        txt._ebuf << "Database not available.";
        return PA_ERROR;
    }

    const time_t now = time( NULL );

    // now look for name matches
    const string name = SanitizeString( txt._args[1], false );

    if (name.empty()) {
        txt._ebuf << xvalue( "NAME" ) << " is empty.";
        return PA_ERROR;
    }

    std::vector<::xmod::UserData> users;
    if (!::xmod::g_database->searchUsersByName(name, users) || users.empty()) {
        Buffer buf;
        buf << _name << ": No match found.";
        printChat( txt._client, buf );
        return PA_NONE;
    }

    Buffer buf;
    static const int maxOutput = 4;
    int outputCount = 0;

    for (std::vector<::xmod::UserData>::const_iterator it = users.begin(); it != users.end(); ++it) {
        if (++outputCount > maxOutput)
            break;

        if (outputCount > 1)
            buf << '\n';
        buf << _name << ": ";

        const ::xmod::UserData& user = *it;
        
        // Check if user is currently online by matching GUID
        bool isOnline = false;
        for (int i = 0; i < level.numConnectedClients; i++) {
            int slot = level.sortedClients[i];
            if (::xmod::g_sessions[slot] && ::xmod::g_sessions[slot]->isAuthenticated()) {
                if (::xmod::g_sessions[slot]->getGuid() == user.guid) {
                    isOnline = true;
                    break;
                }
            }
        }
        
        if (isOnline) {
            buf << xvalue( user.name ) << " is currently online.";
            continue;
        }

        const time_t delta = now - user.lastSeen;
        const string stime = str::toStringSecondsRemaining( delta, true );
        buf << xvalue( user.name ) << ' ' << xvalue( stime ) << " ago.";
    }

    if (outputCount == 0)
        buf << _name << ": No match found.";
    else if (outputCount > maxOutput)
        buf << '\n' << xcheader << "--too many results. Please be more specific in your search.";

    printChat( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
