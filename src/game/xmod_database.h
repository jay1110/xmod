#ifndef GAME_XMOD_DATABASE_H
#define GAME_XMOD_DATABASE_H

#include <string>
#include <vector>
#include <ctime>
#include <sqlite3.h>

///////////////////////////////////////////////////////////////////////////////
// SQLite-based database for xmod authentication and user management
///////////////////////////////////////////////////////////////////////////////

namespace xmod {

struct UserData {
    int id;
    std::string guid;
    int level;
    time_t lastSeen;
    std::string name;
    std::string hwid;
    std::string title;
    std::string commands;
    std::string greeting;
    std::string xp_skills;
    bool muted;
};

struct BanData {
    int id;
    std::string name;
    std::string guid;
    std::string hwid;
    std::string ip;
    std::string banned_by;
    std::string ban_date;
    time_t expires;
    std::string reason;
};

class Database {
private:
    sqlite3* db;
    std::string dbPath;
    bool isOpen;

    bool executeSQL(const char* sql);
    bool createTables();

public:
    Database();
    ~Database();

    // Database operations
    bool open(const std::string& path);
    void close();
    bool isOpened() const { return isOpen; }

    // User operations
    bool addUser(const std::string& guid, const std::string& hwid, const std::string& name);
    bool userExists(const std::string& guid);
    bool userExistsById(int id);
    bool getUserData(const std::string& guid, UserData& data);
    bool getUserDataById(int id, UserData& data);
    bool setLevel(int id, int level);
    bool updateLastSeen(int id, time_t lastSeen);
    bool updateName(int id, const std::string& name);
    bool addHwid(int id, const std::string& hwid);
    bool setXpSkills(int id, const std::string& xpSkills);
    bool setMuted(int id, bool muted);

    // Ban operations
    bool banUser(const std::string& guid, const std::string& hwid, const std::string& ip,
                 const std::string& name, const std::string& banned_by, 
                 const std::string& reason, time_t expires);
    bool isBanned(const std::string& guid, const std::string& hwid, BanData& banData);
    bool isIpBanned(const std::string& ip, BanData& banData);
    bool unbanUser(const std::string& guid);
    bool unbanById(int banId);
    bool getBanList(std::vector<BanData>& bans);
    int getBanCount();

    // Name tracking
    bool addNameAlias(int userId, const std::string& cleanName, const std::string& name);
    
    // Level operations
    bool levelExists(int level);
    
    // Map operations (spree records)
    bool updateMapSpreeRecord(const std::string& mapName, int spreeRecord, 
                             const std::string& spreePlayer, time_t spreeDate);

    // User listing and searching
    bool getUserList(std::vector<UserData>& users);
    bool searchUsersByName(const std::string& name, std::vector<UserData>& users);
    bool deleteUser(int id);
    bool deleteUserByGuid(const std::string& guid);
    int getUserCount();
    
    // XP operations
    bool resetAllXp();
    
    // User/GUID fetch/create (like fetchByKey)
    bool getOrCreateUser(const std::string& guid, const std::string& name, UserData& data);
    
    // Level migration
    int migrateLevel(int fromLevel, int toLevel);
};

} // namespace xmod

#endif // GAME_XMOD_DATABASE_H
