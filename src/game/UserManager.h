#ifndef GAME_USERMANAGER_H
#define GAME_USERMANAGER_H

#include <map>
#include <string>
#include <game/User.h>

///////////////////////////////////////////////////////////////////////////////

/*
 * UserManager: Simple replacement for UserDB that manages User objects
 * without the legacy file-based database.
 * 
 * This class provides basic GUID-to-User mapping functionality needed by
 * the game code. Actual persistence is handled by the SQLite xmod.db system.
 */
class UserManager {
private:
    typedef std::map<std::string, User> mapGUID_t;
    mapGUID_t _mapGUID;  // primary guid->user memory-map

public:
    UserManager();
    ~UserManager();

    /*
     * Fetch a user by GUID. If the GUID does not exist, one will be
     * created automatically.
     *
     * Param guid specifies which user GUID to fetch. GUID is case insensitive.
     * Returns a reference to a modifiable user record
     *         or User::BAD if invalid GUID.
     */
    User& fetchByKey(const std::string& guid, std::string& err, bool create = false);

    /*
     * Fetch a user by partial GUID ID (suffix match).
     * Used for admin commands like !setlevel
     *
     * Param id specifies a partial GUID (at least 8 chars).
     * Returns a reference to a user or User::BAD if not found/ambiguous.
     */
    User& fetchByID(const std::string& id, std::string& err);

    /*
     * Remove a user from the map
     */
    void remove(User& user);

    /*
     * Get the internal GUID map (read-only access)
     */
    const mapGUID_t& getMapGUID() const { return _mapGUID; }
};

///////////////////////////////////////////////////////////////////////////////

extern UserManager userManager;
extern User* connectedUsers[MAX_CLIENTS];

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_USERMANAGER_H
