#include <bgame/impl.h>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {
bool replaceLevelFile(const string& temporary, const string& destination)
{
#ifdef _WIN32
    return MoveFileExA(temporary.c_str(), destination.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(temporary.c_str(), destination.c_str()) == 0;
#endif
}

bool writeLevelFile(const string& destination, const string& contents)
{
    const string temporary = destination + ".tmp";
    ofstream output(temporary.c_str(), ios::binary | ios::trunc);
    if (!output.is_open()) return false;
    output.write(contents.data(), contents.size());
    output.flush();
    const bool written = output.good();
    output.close();
    if (!written || output.fail()) return false;
    return replaceLevelFile(temporary, destination);
}
}


///////////////////////////////////////////////////////////////////////////////

LevelDB::LevelDB()
    : Database ( "level.db", "level" )
    , _canSave ( false )
    , mapLEVEL ( _mapLEVEL )
{
}

///////////////////////////////////////////////////////////////////////////////

LevelDB::~LevelDB()
{
}

///////////////////////////////////////////////////////////////////////////////

Level&
LevelDB::fetchByKey( const string& key, string& err, bool create )
{
    return fetchByKey( toKey( key ), err, create );
}

///////////////////////////////////////////////////////////////////////////////

Level&
LevelDB::fetchByKey( int key, string& err, bool create )
{
    if (key < Level::NUM_MIN) {
        ostringstream oss;
        oss << "minimum is " << Level::NUM_MIN;
        err = oss.str();
        return Level::BAD;
    }

    if (key > Level::NUM_MAX) {
        ostringstream oss;
        oss << "maximum is " << Level::NUM_MAX;
        err = oss.str();
        return Level::BAD;
    }

    const mapLEVEL_t::iterator found = _mapLEVEL.find( key );
    if (found != _mapLEVEL.end())
        return found->second;

    if (!create) {
        err = "not found";
        return Level::BAD;
    }

    // create
    Level& lev = _mapLEVEL[ key ];
    lev._level      = key;
    lev.privGranted = Level::DEFAULT.privGranted;
    lev.privDenied  = Level::DEFAULT.privDenied ;

    return lev;
}

///////////////////////////////////////////////////////////////////////////////

Level&
LevelDB::fetchByName( const string& name, string& err )
{
    if (name.empty()) {
        err = "is empty";
        return Level::BAD;
    }

    string lname = name;
    str::toLower( lname );

    if (str::isIndex( lname ))
        return fetchByKey( name, err );

    Level* found = NULL;
    int count = 0;
    const mapLEVEL_t::iterator max = _mapLEVEL.end();
    for ( mapLEVEL_t::iterator it = _mapLEVEL.begin(); it != max; it++ ) {
        Level& lev = it->second;
        string tmp = lev.name;
        str::toLower( tmp );
        if (tmp.find( lname ) != string::npos) {
            found = &lev;
            count++;
        }
    }

    if (count == 0) {
        err = "not found";
        return Level::BAD;
    }
    else if (count == 1) {
        return *found;
    }
    else {
        err = "ambiguous";
        return Level::BAD;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
LevelDB::load()
{
    // Never permit a failed reload to overwrite the last valid file.
    _canSave = false;
    Level defaults;
    defaults._level = 0;
    defaults.name = "default";
    defaults.namex = defaults.name;
    defaults.privGranted.insert(cmd::builtins::listPlayers._privilege);
    defaults.privGranted.insert(cmd::builtins::resetmyXp._privilege);
    if (_mapLEVEL.empty()) {
        Level::DEFAULT = defaults;
        _mapLEVEL[0] = defaults;
    }

    string filename;
    if (open(false, filename)) {
        close();
        return;
    }

    // Keep the loaded text as the backup, including comments and formatting.
    ostringstream original;
    original << _stream.rdbuf();
    if (_stream.bad()) {
        close();
        trap_Printf("WARNING: level.db read failed; saving disabled.\n");
        return;
    }
    _stream.clear();
    _stream.seekg(0);
    mapLEVEL_t candidate;
    candidate[0] = defaults;
    set<int> seen;
    bool valid = _stream.good();
    map<string,string> data;
    while (valid && !_stream.rdstate()) {
        parseData(data);
        if (data.empty()) continue;
        map<string,string>::const_iterator it = data.find(_key);
        if (it == data.end()) { valid = false; break; }
        int key = -1;
        istringstream keyStream(it->second);
        keyStream >> key;
        if (keyStream.fail()) { valid = false; break; }
        keyStream >> ws;
        if (!keyStream.eof() || key < Level::NUM_MIN || key > Level::NUM_MAX ||
            !seen.insert(key).second) {
            valid = false;
            break;
        }
        Level record = defaults;
        record.decode(data);
        candidate[key] = record;
    }
    valid = valid && !_stream.bad() && !seen.empty();
    close();
    if (!valid) {
        trap_Printf("WARNING: invalid or empty level.db; existing levels retained, saving disabled.\n");
        return;
    }
    _mapLEVEL.swap(candidate);
    _lastGoodContents = original.str();
    _loadedPath = filename;
    _canSave = true;
    trap_Printf(va("Reading: %s, %d levels\n", filename.c_str(), int(_mapLEVEL.size())));

}

///////////////////////////////////////////////////////////////////////////////

uint32
LevelDB::remove( Level& obj, const Level& migrate )
{
    // Legacy userDB.migrateAuth removed - auth levels are managed in SQLite
    // This function now only removes the level from the level database
    _mapLEVEL.erase( obj.level );
    return 0;  // No users migrated (handled by SQLite)
}

///////////////////////////////////////////////////////////////////////////////

void
LevelDB::save()
{
    if (!_canSave) {
        trap_Printf("WARNING: level.db save skipped: no successful load. Restore the file and use !dbload.\n");
        return;
    }
    char home[MAX_CVAR_VALUE_STRING], game[MAX_CVAR_VALUE_STRING];
    trap_Cvar_VariableStringBuffer("fs_homepath", home, sizeof(home));
    trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
    const string filename = string(home) + "/" + game + "/" + _filename;
    if (filename != _loadedPath) {
        trap_Printf("WARNING: level.db path changed; save skipped.\n");
        return;
    }

    ostringstream output;
    time_t now = time(0);
    char fnow[32];
    strftime(fnow, sizeof(fnow), "%c", localtime(&now));
    output << "###############################################################################"
        << '\n' << "## " << XMOD_title << " -- " << _filename
        << '\n' << "## updated: " << fnow
        << '\n' << "## levels:  " << _mapLEVEL.size()
        << '\n' << "###############################################################################";
    int recnum = 0;
    for (mapLEVEL_t::iterator it = _mapLEVEL.begin(); it != _mapLEVEL.end(); ++it)
        it->second.encode(output, recnum++);
    output << '\n';
    if (!output.good()) {
        trap_Printf("WARNING: level.db serialization failed; file unchanged.\n");
        return;
    }

    // Both replacements stay on the same filesystem. Never truncate level.db.
    if (!writeLevelFile(filename + ".bak", _lastGoodContents)) {
        trap_Printf("WARNING: level.db backup failed; file unchanged.\n");
        return;
    }
    const string contents = output.str();
    if (!writeLevelFile(filename, contents)) {
        trap_Printf("WARNING: level.db replacement failed; previous file retained.\n");
        return;
    }
    _lastGoodContents = contents;
    trap_Printf(va("Writing: %s, %d levels (backup: level.db.bak)\n",
        filename.c_str(), int(_mapLEVEL.size())));

}

///////////////////////////////////////////////////////////////////////////////

int
LevelDB::toKey( const string& key )
{
    int ikey = -1;
    istringstream( key ) >> ikey;
    return ikey;
}

///////////////////////////////////////////////////////////////////////////////

LevelDB levelDB;
