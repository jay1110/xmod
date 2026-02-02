#include <bgame/impl.h>
#include <game/cmd/CustomCommand.h>

///////////////////////////////////////////////////////////////////////////////

CustomCommandDB::CustomCommandDB()
{
}

///////////////////////////////////////////////////////////////////////////////

CustomCommandDB::~CustomCommandDB()
{
    clear();
}

///////////////////////////////////////////////////////////////////////////////

void
CustomCommandDB::clear()
{
    // Remove custom commands from registry and delete them
    for ( size_t i = 0; i < _commands.size(); i++ ) {
        cmd::CustomCommand* cmd = _commands[i];
        if ( cmd ) {
            // Remove from registry
            cmd::Registry::iterator it = cmd::registry.find( cmd->_name );
            if ( it != cmd::registry.end() ) {
                cmd::registry.erase( it );
            }
            delete cmd;
        }
    }
    _commands.clear();
}

///////////////////////////////////////////////////////////////////////////////

int
CustomCommandDB::count() const
{
    return static_cast<int>( _commands.size() );
}

///////////////////////////////////////////////////////////////////////////////

void
CustomCommandDB::parseLevels( const string& levelsStr, set<int>& levels )
{
    levels.clear();
    
    string str = levelsStr;
    size_t pos = 0;
    
    // Replace commas with spaces
    while ( (pos = str.find( ',', pos )) != string::npos ) {
        str[pos] = ' ';
    }
    
    // Parse space-separated integers
    istringstream iss( str );
    int level;
    while ( iss >> level ) {
        if ( level >= Level::NUM_MIN && level <= Level::NUM_MAX ) {
            levels.insert( level );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

int
CustomCommandDB::load()
{
    // Clear any existing custom commands
    clear();
    
    // Build file path: fs_homepath/fs_game/commands.db
    char buffer[MAX_CVAR_VALUE_STRING];
    string fname;
    
    trap_Cvar_VariableStringBuffer( "fs_homepath", buffer, sizeof(buffer) );
    fname = buffer;
    fname += "/";
    
    trap_Cvar_VariableStringBuffer( "fs_game", buffer, sizeof(buffer) );
    fname += buffer;
    fname += "/commands.db";
    
    // Open file
    ifstream file( fname.c_str() );
    if ( !file.is_open() ) {
        // Try alternate path with basepath
        trap_Cvar_VariableStringBuffer( "fs_basepath", buffer, sizeof(buffer) );
        fname = buffer;
        fname += "/";
        
        trap_Cvar_VariableStringBuffer( "fs_game", buffer, sizeof(buffer) );
        fname += buffer;
        fname += "/commands.db";
        
        file.open( fname.c_str() );
        if ( !file.is_open() ) {
            // No commands.db file found - this is not an error, just no custom commands
            return 0;
        }
    }
    
    ostringstream msg;
    msg << "Loading custom commands from: " << fname << endl;
    trap_Printf( msg.str().c_str() );
    
    string line;
    string name, exec, desc, levelsStr;
    bool inRecord = false;
    int loadedCount = 0;
    
    while ( getline( file, line ) ) {
        // Trim leading/trailing whitespace
        size_t start = line.find_first_not_of( " \t\r\n" );
        if ( start == string::npos ) {
            continue;  // Empty line
        }
        size_t end = line.find_last_not_of( " \t\r\n" );
        line = line.substr( start, end - start + 1 );
        
        // Skip empty lines and comments
        if ( line.empty() || line[0] == '#' ) {
            continue;
        }
        
        // Check for record delimiter (10 asterisks)
        if ( line.find( "**********" ) == 0 ) {
            // If we have a previous record, create the command
            if ( inRecord && !name.empty() && !exec.empty() ) {
                set<int> levels;
                parseLevels( levelsStr, levels );
                
                // Only create command if it has valid levels and doesn't conflict with builtin
                if ( !levels.empty() ) {
                    // Check if a command with this name already exists
                    cmd::AbstractCommand* existing = cmd::commandForName( name );
                    if ( existing ) {
                        ostringstream warn;
                        warn << "WARNING: Custom command '" << name << "' conflicts with existing command, skipping." << endl;
                        trap_Printf( warn.str().c_str() );
                    }
                    else {
                        // Create the custom command
                        cmd::CustomCommand* customCmd = new cmd::CustomCommand( name, exec, desc, levels );
                        _commands.push_back( customCmd );
                        loadedCount++;
                    }
                }
            }
            
            // Reset for new record
            name.clear();
            exec.clear();
            desc.clear();
            levelsStr.clear();
            inRecord = true;
            continue;
        }
        
        if ( !inRecord ) {
            continue;
        }
        
        // Parse key = value
        size_t eqPos = line.find( '=' );
        if ( eqPos == string::npos ) {
            continue;
        }
        
        string key = line.substr( 0, eqPos );
        string value = ( eqPos + 1 < line.length() ) ? line.substr( eqPos + 1 ) : "";
        
        // Trim key and value
        start = key.find_first_not_of( " \t" );
        if ( start != string::npos ) {
            end = key.find_last_not_of( " \t" );
            key = key.substr( start, end - start + 1 );
        }
        
        start = value.find_first_not_of( " \t" );
        if ( start != string::npos ) {
            end = value.find_last_not_of( " \t" );
            value = value.substr( start, end - start + 1 );
        }
        
        // Convert key to lowercase for comparison
        for ( size_t i = 0; i < key.length(); i++ ) {
            key[i] = tolower( key[i] );
        }
        
        if ( key == "name" ) {
            name = value;
            // Convert command name to lowercase
            for ( size_t i = 0; i < name.length(); i++ ) {
                name[i] = tolower( name[i] );
            }
        }
        else if ( key == "exec" ) {
            exec = value;
        }
        else if ( key == "desc" ) {
            desc = value;
        }
        else if ( key == "levels" ) {
            levelsStr = value;
        }
    }
    
    // Don't forget the last record
    if ( inRecord && !name.empty() && !exec.empty() ) {
        set<int> levels;
        parseLevels( levelsStr, levels );
        
        if ( !levels.empty() ) {
            cmd::AbstractCommand* existing = cmd::commandForName( name );
            if ( existing ) {
                ostringstream warn;
                warn << "WARNING: Custom command '" << name << "' conflicts with existing command, skipping." << endl;
                trap_Printf( warn.str().c_str() );
            }
            else {
                cmd::CustomCommand* customCmd = new cmd::CustomCommand( name, exec, desc, levels );
                _commands.push_back( customCmd );
                loadedCount++;
            }
        }
    }
    
    file.close();
    
    msg.str( "" );
    msg << "Loaded " << loadedCount << " custom command(s)." << endl;
    trap_Printf( msg.str().c_str() );
    
    return loadedCount;
}

///////////////////////////////////////////////////////////////////////////////

CustomCommandDB customCommandDB;
