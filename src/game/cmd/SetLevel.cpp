#include <bgame/impl.h>
#include <game/xmod_database.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

SetLevel::SetLevel()
    : AbstractBuiltin( "setlevel" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << xvalue( "LEVEL" );
    __descr << "Change a specific player's admin level.";
}

///////////////////////////////////////////////////////////////////////////////

SetLevel::~SetLevel()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
SetLevel::doExecute( Context& txt )
{
    if (txt._args.size() != 3)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    Level& lev = lookupLEVEL( txt._args[2], txt );
    if (lev == Level::BAD)
        return PA_ERROR;

    // bail if raising level
    if (txt._user.authLevel < lev.level) {
        txt._ebuf << "You cannot set a level higher than your own.";
        return PA_ERROR;
    }

    // Use session-aware helpers for player data
    const std::string& targetNamex = getPlayerNamex(target->slot);
    
    // bail if fake GUID
    if (isPlayerFakeGuid(target->slot)) {
        txt._ebuf << xvalue( targetNamex ) << " has no GUID.";
        return PA_ERROR;
    }

    // Update runtime user level - still need connectedUsers for modifying authLevel
    // since Session doesn't have a direct setter that syncs to User
    if (connectedUsers[target->slot] && connectedUsers[target->slot] != &User::BAD) {
        connectedUsers[target->slot]->authLevel = lev.level;
    }

    // Persist level to SQLite database
    if (::xmod::g_database && ::xmod::g_database->isOpened() && 
        ::xmod::g_sessions[target->slot] && ::xmod::g_sessions[target->slot]->isAuthenticated()) {
        int userId = ::xmod::g_sessions[target->slot]->getUserId();
        if (userId > 0) {
            if (::xmod::g_database->setLevel(userId, lev.level)) {
                G_Printf("SetLevel: Updated user %d level to %d in SQLite\n", userId, lev.level);
                // Also update session level
                ::xmod::g_sessions[target->slot]->setUserLevel(lev.level);
            } else {
                G_Printf("^1SetLevel: Failed to update user %d level in SQLite\n", userId);
            }
        }
    }

    // Report success
    Buffer buf;
    buf << _name << ": " << xvalue( targetNamex ) << "'s level set to " << xvalue( lev.level );
    printCpm(txt._client, buf, true);

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
