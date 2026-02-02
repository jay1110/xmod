#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

CustomCommand::CustomCommand( const string& name, const string& exec, const string& desc, const set<int>& levels )
    : AbstractCommand( Privilege::TYPE_CUSTOM, name.c_str(), false )
    , _exec   ( exec )
    , _levels ( levels )
{
    __usage << xvalue( "!" + _name ) << " [arguments]";
    __descr << desc;
}

///////////////////////////////////////////////////////////////////////////////

CustomCommand::~CustomCommand()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
CustomCommand::hasPermission( const Context& txt )
{
    // Check if user's auth level is in the allowed levels set
    return _levels.find( txt._user.authLevel ) != _levels.end();
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
CustomCommand::doExecute( Context& txt )
{
    // Substitute variables in the exec string
    string cmdStr = substituteVariables( _exec, txt );
    
    // Execute the command on the server console
    if ( !cmdStr.empty() ) {
        // Ensure the command ends with a newline
        if ( cmdStr[cmdStr.length()-1] != '\n' ) {
            cmdStr += '\n';
        }
        trap_SendConsoleCommand( EXEC_APPEND, cmdStr.c_str() );
    }
    
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

string
CustomCommand::substituteVariables( const string& str, Context& txt )
{
    string result = str;
    
    // [n] - Executing player's name (colored)
    if ( txt._client ) {
        const std::string& namex = getPlayerNamex( txt._client->slot );
        size_t pos = 0;
        while ( (pos = result.find( "[n]", pos )) != string::npos ) {
            result.replace( pos, 3, namex );
            pos += namex.length();
        }
    }
    else {
        // Replace [n] with "Server" for console commands
        size_t pos = 0;
        while ( (pos = result.find( "[n]", pos )) != string::npos ) {
            result.replace( pos, 3, "Server" );
            pos += 6;
        }
    }
    
    // [d] - Target player (from first argument if it's a player reference)
    if ( txt._args.size() > 1 ) {
        Client* target = NULL;
        Buffer dummy;
        if ( !matchClient( txt._args[1], target, dummy ) && target ) {
            const std::string& targetName = getPlayerNamex( target->slot );
            size_t pos = 0;
            while ( (pos = result.find( "[d]", pos )) != string::npos ) {
                result.replace( pos, 3, targetName );
                pos += targetName.length();
            }
        }
    }
    // If no valid target, replace [d] with empty string
    {
        size_t pos = 0;
        while ( (pos = result.find( "[d]", pos )) != string::npos ) {
            result.replace( pos, 3, "" );
        }
    }
    
    // [1], [2], [3] etc. - Command arguments
    for ( size_t i = 1; i < txt._args.size() && i <= 9; i++ ) {
        char placeholder[4];
        snprintf( placeholder, sizeof(placeholder), "[%d]", static_cast<int>(i) );
        size_t pos = 0;
        while ( (pos = result.find( placeholder, pos )) != string::npos ) {
            result.replace( pos, 3, txt._args[i] );
            pos += txt._args[i].length();
        }
    }
    
    // Remove any remaining unused argument placeholders
    for ( int i = 1; i <= 9; i++ ) {
        char placeholder[4];
        snprintf( placeholder, sizeof(placeholder), "[%d]", i );
        size_t pos = 0;
        while ( (pos = result.find( placeholder, pos )) != string::npos ) {
            result.replace( pos, 3, "" );
        }
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
