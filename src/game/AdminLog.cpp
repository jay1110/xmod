#include <bgame/impl.h>
#include <game/xmod_globals.h>
#include <game/server_log_path.h>

///////////////////////////////////////////////////////////////////////////////

AdminLog::AdminLog()
{
}

///////////////////////////////////////////////////////////////////////////////

AdminLog::~AdminLog()
{
    if (_out.is_open())
        _out.close();
}

///////////////////////////////////////////////////////////////////////////////

void
AdminLog::cvarCallback( Cvar& cvar )
{
    adminLog.recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
AdminLog::init()
{
    recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
AdminLog::log( Client* actor, const vector<string>& args, bool denied )
{
    if (!_out.is_open())
        return;

    if (args.empty())
        return;

    struct Entry {
        const char* guid;
        const char* name;
        int         slot;
    };

    Entry entry;
    if (!actor) {
        entry.guid = "";
        entry.name = "console";
        entry.slot = -1;
    }
    else {
        // Use session helpers for GUID and name
        entry.guid = ::xmod::getClientGuid(actor->slot).c_str();
        entry.name = ::xmod::getClientName(actor->slot).c_str();
        entry.slot = actor->slot;
    }

    // compute timestamp
    time_t now = time( NULL );
    tm* lt = localtime( &now );
    char stime[32];
    strftime( stime, sizeof(stime), "%c", lt);

    string cline;
    str::concatArgs( args, cline );

    ostringstream oss;
    oss << (denied ? '-' : '+')
        << '[' << stime << ']'
        << " [" << setw(2) << entry.slot << ']'
        << " [" << setfill('-') << setw(32) << entry.guid << '/' << entry.name << ']'
        << ' ' << cline;
    _out << oss.str() << endl;
}

///////////////////////////////////////////////////////////////////////////////

void
AdminLog::recompute()
{
    const string configured = cvars::g_adminLog.svalue;
    string filename;
    const bool resolved = !configured.empty() && serverlog::resolve(configured, filename);
    if (resolved && _out.is_open() && filename == _filename) return;

    if (_out.is_open()) _out.close();
    _out.clear();
    _filename.clear();
    if (configured.empty()) return;

    if (resolved) _out.open(filename.c_str(), ios::app);
    if (!resolved || !_out.is_open() || _out.fail()) {
        if (_out.is_open()) _out.close();
        _out.clear();
        trap_Printf(va("WARNING: unable to open admin log '%s'. Check its path and write permissions.\n",
            (filename.empty() ? configured : filename).c_str()));
        return;
    }
    _filename = filename;
}

///////////////////////////////////////////////////////////////////////////////

AdminLog adminLog;
