#ifndef GAME_CUSTOMCOMMANDDB_H
#define GAME_CUSTOMCOMMANDDB_H

///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <set>

namespace cmd {
    class CustomCommand;
}

///////////////////////////////////////////////////////////////////////////////

class CustomCommandDB {
public:
    CustomCommandDB();
    ~CustomCommandDB();

    /**************************************************************************
     * Load custom commands from commands.db file.
     * Returns number of commands loaded.
     */
    int load();

    /**************************************************************************
     * Clear all custom commands.
     */
    void clear();

    /**************************************************************************
     * Get number of loaded custom commands.
     */
    int count() const;

private:
    // Parse a levels string like "0, 1, 2, 3, 4, 5" into a set of integers
    void parseLevels( const string& levelsStr, set<int>& levels );

    vector<cmd::CustomCommand*> _commands;
};

///////////////////////////////////////////////////////////////////////////////

extern CustomCommandDB customCommandDB;

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CUSTOMCOMMANDDB_H
