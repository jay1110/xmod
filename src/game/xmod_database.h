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
    time_t muteTime;
    time_t muteExpiry;
    std::string muteReason;
    std::string muteAuthority;
    time_t nospamExpiry = 0; // 0: disabled, -1: permanent, otherwise Unix expiry
    time_t nospamLastChat = 0;
};

struct MapRecords {
    int spreeRecord = 0;
    std::string spreePlayer;
    time_t spreeDate = 0;
    int fragRecord = 0;
    std::string fragPlayer;
    time_t fragDate = 0;
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
    bool executeSQLSilent(const char* sql);
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
    bool getUserByGuid(const std::string& guid, UserData& data); // Alias for getUserData
    bool setLevel(int id, int level);
    bool updateLastSeen(int id, time_t lastSeen);
    bool updateName(int id, const std::string& name);
    bool addHwid(int id, const std::string& hwid);
    bool setXpSkills(int id, const std::string& xpSkills);
    bool setMuted(int id, bool muted);
    bool setNospamExpiry(int userId, time_t expiry);
    bool setNospamLastChat(int userId, time_t lastChat);
    bool setMuteData(int userId, bool muted, time_t muteTime, time_t muteExpiry, 
                     const std::string& reason, const std::string& authority);

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
    
    // Map records: missing maps are returned as an empty record, not an error.
    bool getMapRecords(const std::string& mapName, MapRecords& records);
    // Atomically stores only improvements. changed: 1 = spree, 2 = frags.
    bool updateMapRecords(const std::string& mapName, const MapRecords& records, int& changed);
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
    
    // Migration from old userDB
    int importFromLegacyUserDB();
};

} // namespace xmod

#endif // GAME_XMOD_DATABASE_H
