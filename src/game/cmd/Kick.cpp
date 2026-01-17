#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Kick::Kick()
    : AbstractBuiltin( "kick" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "REASON" );
    __descr << "Kick a player off the server.";
}

///////////////////////////////////////////////////////////////////////////////

Kick::~Kick()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Kick::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    // Use session-aware helpers for player data
    const std::string& targetNamex = getPlayerNamex(target->slot);
    const std::string& targetGuid = getPlayerGuid(target->slot);
    const std::string& userGuid = getPlayerGuid(txt._client ? txt._client->slot : -1);
    
    // Self-kick check using GUID comparison
    if (!targetGuid.empty() && !userGuid.empty() && targetGuid == userGuid) {
        txt._ebuf << "You cannot kick yourself.";
        return PA_ERROR;
    }

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    // compute reason text
    string kickReason;
    if (txt._args.size() > 2)
        str::concatArgs( txt._args, kickReason, 2 );

    if (kickReason.empty()) {
        if (!txt._user.hasPrivilege( priv::base::reasonNone )) {
            txt._ebuf << "You must specify a kick reason.";
            return PA_ERROR;
        }

        kickReason = "none";
    }

    Buffer buf;
    buf << _name << ": " << xvalue( targetNamex ) << " kicked for " << xvalue( cache::kickDuration ) << '.';
    printCpm( txt._client, buf, true );

    buf.reset();
    buf << '\n' << "player: " << xvalue( targetNamex )
        << '\n'
        << '\n' << "duration:"
        << '\n' << xvalue( cache::kickDuration )
        << '\n'
        << '\n' << "reason:"
        << '\n' << xvalue( kickReason );
    SEngine::dropClient( target->slot, buf, "You have been kicked.", cache::kickTime );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
