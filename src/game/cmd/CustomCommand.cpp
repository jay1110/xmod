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
    
    if ( cmdStr.empty() ) {
        return PA_NONE;
    }
    
    // Split the command string by semicolons and process each part
    // This allows mixing server console commands with admin ! commands
    size_t start = 0;
    
    while ( start < cmdStr.length() ) {
        // Find next semicolon or end of string
        size_t pos = cmdStr.find( ';', start );
        if ( pos == string::npos ) {
            pos = cmdStr.length();
        }
        
        // Extract this command segment
        string segment = cmdStr.substr( start, pos - start );
        
        // Trim leading/trailing whitespace
        size_t first = segment.find_first_not_of( " \t\r\n" );
        if ( first != string::npos ) {
            size_t last = segment.find_last_not_of( " \t\r\n" );
            segment = segment.substr( first, last - first + 1 );
        }
        else {
            segment.clear();
        }
        
        if ( !segment.empty() ) {
            // Check if this is an admin command (starts with !)
            if ( segment[0] == '!' ) {
                // Process as an admin command through cmd::process
                // Pass the command as the actor's command (simulates them typing it)
                process( txt._client, false, &segment );
            }
            else {
                // Send to server console
                segment += '\n';
                trap_SendConsoleCommand( EXEC_APPEND, segment.c_str() );
            }
        }
        
        // Move past the semicolon (or end of string)
        start = pos + 1;
    }
    
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

string
CustomCommand::substituteVariables( const string& str, Context& txt )
{
    string result = str;
    
    // [n] - Executing player's name (colored)
    // Use netname directly for reliability - lookupPLAYER sanitizes color codes
    if ( txt._client ) {
        string namex = txt._client->gclient.pers.netname;
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
            string targetName = target->gclient.pers.netname;
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
            result.erase( pos, 3 );
            // Don't increment pos - next search starts at same position after string shrinks
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
            result.erase( pos, 3 );
            // Don't increment pos - next search starts at same position after string shrinks
        }
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
