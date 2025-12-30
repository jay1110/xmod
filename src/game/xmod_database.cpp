#include <bgame/impl.h>
#include "xmod_database.h"
#include <cstring>
#include <sstream>

namespace xmod {

///////////////////////////////////////////////////////////////////////////////

Database::Database() : db(nullptr), isOpen(false) {
}

Database::~Database() {
    close();
}

///////////////////////////////////////////////////////////////////////////////

bool Database::executeSQL(const char* sql) {
    if (!isOpen || !db) {
        return false;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        if (errMsg) {
            G_Printf("SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
        }
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::createTables() {
    const char* sql_users = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "guid TEXT UNIQUE NOT NULL,"
        "level INTEGER DEFAULT 0,"
        "lastSeen INTEGER,"
        "name TEXT,"
        "hwid TEXT,"
        "title TEXT,"
        "commands TEXT,"
        "greeting TEXT,"
        "xp_skills TEXT,"
        "muted INTEGER DEFAULT 0"
        ");";

    const char* sql_bans = 
        "CREATE TABLE IF NOT EXISTS bans ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "guid TEXT NOT NULL,"
        "hwid TEXT,"
        "ip TEXT,"
        "banned_by TEXT,"
        "ban_date TEXT,"
        "expires INTEGER,"
        "reason TEXT"
        ");";

    const char* sql_levels = 
        "CREATE TABLE IF NOT EXISTS levels ("
        "level INTEGER PRIMARY KEY,"
        "name TEXT,"
        "namex TEXT,"
        "greeting_text TEXT,"
        "greeting_audio TEXT,"
        "priv_granted TEXT,"
        "priv_denied TEXT"
        ");";

    const char* sql_maps = 
        "CREATE TABLE IF NOT EXISTS maps ("
        "map_name TEXT PRIMARY KEY,"
        "spree_record INTEGER,"
        "spree_player TEXT,"
        "spree_date INTEGER"
        ");";

    const char* sql_names = 
        "CREATE TABLE IF NOT EXISTS names ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "clean_name TEXT,"
        "name TEXT UNIQUE,"
        "user_id INTEGER,"
        "FOREIGN KEY (user_id) REFERENCES users(id)"
        ");";

    // Create indexes for performance
    const char* sql_indexes = 
        "CREATE INDEX IF NOT EXISTS idx_users_guid ON users(guid);"
        "CREATE INDEX IF NOT EXISTS idx_users_hwid ON users(hwid);"
        "CREATE INDEX IF NOT EXISTS idx_bans_guid ON bans(guid);"
        "CREATE INDEX IF NOT EXISTS idx_bans_hwid ON bans(hwid);"
        "CREATE INDEX IF NOT EXISTS idx_bans_ip ON bans(ip);"
        "CREATE INDEX IF NOT EXISTS idx_names_user_id ON names(user_id);";

    return executeSQL(sql_users) &&
           executeSQL(sql_bans) &&
           executeSQL(sql_levels) &&
           executeSQL(sql_maps) &&
           executeSQL(sql_names) &&
           executeSQL(sql_indexes);
}

///////////////////////////////////////////////////////////////////////////////

bool Database::open(const std::string& path) {
    if (isOpen) {
        close();
    }

    dbPath = path;
    int rc = sqlite3_open(path.c_str(), &db);
    
    if (rc != SQLITE_OK) {
        G_Printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        return false;
    }

    isOpen = true;
    
    // Create tables if they don't exist
    if (!createTables()) {
        G_Printf("Failed to create database tables\n");
        close();
        return false;
    }

    G_Printf("SQLite database opened: %s\n", path.c_str());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    isOpen = false;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::addUser(const std::string& guid, const std::string& hwid, const std::string& name) {
    if (!isOpen || !db) return false;

    const char* sql = "INSERT INTO users (guid, hwid, name, lastSeen) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        G_Printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return false;
    }

    time_t now = time(nullptr);
    sqlite3_bind_text(stmt, 1, guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hwid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, (sqlite3_int64)now);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        G_Printf("Failed to insert user: %s\n", sqlite3_errmsg(db));
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::userExists(const std::string& guid) {
    if (!isOpen || !db) return false;

    const char* sql = "SELECT id FROM users WHERE guid = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, guid.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    
    return exists;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::userExistsById(int id) {
    if (!isOpen || !db) return false;

    const char* sql = "SELECT id FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    
    return exists;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::getUserData(const std::string& guid, UserData& data) {
    if (!isOpen || !db) return false;

    const char* sql = "SELECT id, guid, level, lastSeen, name, hwid, title, commands, greeting, xp_skills, muted "
                      "FROM users WHERE guid = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, guid.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        data.id = sqlite3_column_int(stmt, 0);
        data.guid = (const char*)sqlite3_column_text(stmt, 1);
        data.level = sqlite3_column_int(stmt, 2);
        data.lastSeen = (time_t)sqlite3_column_int64(stmt, 3);
        
        const char* name = (const char*)sqlite3_column_text(stmt, 4);
        data.name = name ? name : "";
        
        const char* hwid = (const char*)sqlite3_column_text(stmt, 5);
        data.hwid = hwid ? hwid : "";
        
        const char* title = (const char*)sqlite3_column_text(stmt, 6);
        data.title = title ? title : "";
        
        const char* commands = (const char*)sqlite3_column_text(stmt, 7);
        data.commands = commands ? commands : "";
        
        const char* greeting = (const char*)sqlite3_column_text(stmt, 8);
        data.greeting = greeting ? greeting : "";
        
        const char* xp_skills = (const char*)sqlite3_column_text(stmt, 9);
        data.xp_skills = xp_skills ? xp_skills : "";
        
        data.muted = sqlite3_column_int(stmt, 10) != 0;
        
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::getUserDataById(int id, UserData& data) {
    if (!isOpen || !db) return false;

    const char* sql = "SELECT id, guid, level, lastSeen, name, hwid, title, commands, greeting, xp_skills, muted "
                      "FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        data.id = sqlite3_column_int(stmt, 0);
        data.guid = (const char*)sqlite3_column_text(stmt, 1);
        data.level = sqlite3_column_int(stmt, 2);
        data.lastSeen = (time_t)sqlite3_column_int64(stmt, 3);
        
        const char* name = (const char*)sqlite3_column_text(stmt, 4);
        data.name = name ? name : "";
        
        const char* hwid = (const char*)sqlite3_column_text(stmt, 5);
        data.hwid = hwid ? hwid : "";
        
        const char* title = (const char*)sqlite3_column_text(stmt, 6);
        data.title = title ? title : "";
        
        const char* commands = (const char*)sqlite3_column_text(stmt, 7);
        data.commands = commands ? commands : "";
        
        const char* greeting = (const char*)sqlite3_column_text(stmt, 8);
        data.greeting = greeting ? greeting : "";
        
        const char* xp_skills = (const char*)sqlite3_column_text(stmt, 9);
        data.xp_skills = xp_skills ? xp_skills : "";
        
        data.muted = sqlite3_column_int(stmt, 10) != 0;
        
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::setLevel(int id, int level) {
    if (!isOpen || !db) return false;

    const char* sql = "UPDATE users SET level = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, level);
    sqlite3_bind_int(stmt, 2, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::updateLastSeen(int id, time_t lastSeen) {
    if (!isOpen || !db) return false;

    const char* sql = "UPDATE users SET lastSeen = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)lastSeen);
    sqlite3_bind_int(stmt, 2, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::updateName(int id, const std::string& name) {
    if (!isOpen || !db) return false;

    const char* sql = "UPDATE users SET name = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::addHwid(int id, const std::string& hwid) {
    if (!isOpen || !db) return false;

    // Get current HWID
    UserData data;
    if (!getUserDataById(id, data)) {
        return false;
    }

    // Check if HWID already exists (exact match or in space-separated list)
    if (!data.hwid.empty()) {
        // Split existing HWIDs by space and check each one
        std::stringstream ss(data.hwid);
        std::string existingHwid;
        while (ss >> existingHwid) {
            if (existingHwid == hwid) {
                // Already exists, no need to add
                return true;
            }
        }
        
        // Not found, append with space separator
        std::string newHwid = data.hwid + " " + hwid;
        
        const char* sql = "UPDATE users SET hwid = ? WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(stmt, 1, newHwid.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, id);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return rc == SQLITE_DONE;
    } else {
        // No existing HWID, just set it
        const char* sql = "UPDATE users SET hwid = ? WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(stmt, 1, hwid.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, id);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return rc == SQLITE_DONE;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Database::setXpSkills(int id, const std::string& xpSkills) {
    if (!isOpen || !db) return false;

    const char* sql = "UPDATE users SET xp_skills = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, xpSkills.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::setMuted(int id, bool muted) {
    if (!isOpen || !db) return false;

    const char* sql = "UPDATE users SET muted = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, muted ? 1 : 0);
    sqlite3_bind_int(stmt, 2, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::banUser(const std::string& guid, const std::string& hwid, const std::string& ip,
                       const std::string& name, const std::string& banned_by, 
                       const std::string& reason, time_t expires) {
    if (!isOpen || !db) return false;

    const char* sql = "INSERT INTO bans (guid, hwid, ip, name, banned_by, ban_date, expires, reason) "
                      "VALUES (?, ?, ?, ?, ?, datetime('now'), ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hwid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, ip.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, banned_by.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, (sqlite3_int64)expires);
    sqlite3_bind_text(stmt, 7, reason.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::isBanned(const std::string& guid, const std::string& hwid, BanData& banData) {
    if (!isOpen || !db) return false;

    // Check for permanent ban or unexpired ban by GUID or HWID
    const char* sql = "SELECT id, name, guid, hwid, ip, banned_by, ban_date, expires, reason "
                      "FROM bans "
                      "WHERE (guid = ? OR hwid = ?) AND (expires = 0 OR expires > ?) "
                      "ORDER BY id DESC LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    time_t now = time(nullptr);
    sqlite3_bind_text(stmt, 1, guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hwid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)now);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        banData.id = sqlite3_column_int(stmt, 0);
        
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        banData.name = name ? name : "";
        
        banData.guid = (const char*)sqlite3_column_text(stmt, 2);
        
        const char* hwid_val = (const char*)sqlite3_column_text(stmt, 3);
        banData.hwid = hwid_val ? hwid_val : "";
        
        const char* ip = (const char*)sqlite3_column_text(stmt, 4);
        banData.ip = ip ? ip : "";
        
        const char* banned_by = (const char*)sqlite3_column_text(stmt, 5);
        banData.banned_by = banned_by ? banned_by : "";
        
        const char* ban_date = (const char*)sqlite3_column_text(stmt, 6);
        banData.ban_date = ban_date ? ban_date : "";
        
        banData.expires = (time_t)sqlite3_column_int64(stmt, 7);
        
        const char* reason = (const char*)sqlite3_column_text(stmt, 8);
        banData.reason = reason ? reason : "";
        
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::isIpBanned(const std::string& ip, BanData& banData) {
    if (!isOpen || !db) return false;

    const char* sql = "SELECT id, name, guid, hwid, ip, banned_by, ban_date, expires, reason "
                      "FROM bans "
                      "WHERE ip = ? AND (expires = 0 OR expires > ?) "
                      "ORDER BY id DESC LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    time_t now = time(nullptr);
    sqlite3_bind_text(stmt, 1, ip.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, (sqlite3_int64)now);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        banData.id = sqlite3_column_int(stmt, 0);
        
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        banData.name = name ? name : "";
        
        const char* guid = (const char*)sqlite3_column_text(stmt, 2);
        banData.guid = guid ? guid : "";
        
        const char* hwid = (const char*)sqlite3_column_text(stmt, 3);
        banData.hwid = hwid ? hwid : "";
        
        banData.ip = (const char*)sqlite3_column_text(stmt, 4);
        
        const char* banned_by = (const char*)sqlite3_column_text(stmt, 5);
        banData.banned_by = banned_by ? banned_by : "";
        
        const char* ban_date = (const char*)sqlite3_column_text(stmt, 6);
        banData.ban_date = ban_date ? ban_date : "";
        
        banData.expires = (time_t)sqlite3_column_int64(stmt, 7);
        
        const char* reason = (const char*)sqlite3_column_text(stmt, 8);
        banData.reason = reason ? reason : "";
        
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::unbanUser(const std::string& guid) {
    if (!isOpen || !db) return false;

    const char* sql = "DELETE FROM bans WHERE guid = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, guid.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::addNameAlias(int userId, const std::string& cleanName, const std::string& name) {
    if (!isOpen || !db) return false;

    const char* sql = "INSERT OR IGNORE INTO names (user_id, clean_name, name) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, cleanName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, name.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::levelExists(int level) {
    if (!isOpen || !db) return false;

    const char* sql = "SELECT level FROM levels WHERE level = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, level);
    rc = sqlite3_step(stmt);
    
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    
    return exists;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::updateMapSpreeRecord(const std::string& mapName, int spreeRecord, 
                                   const std::string& spreePlayer, time_t spreeDate) {
    if (!isOpen || !db) return false;

    const char* sql = "INSERT OR REPLACE INTO maps (map_name, spree_record, spree_player, spree_date) "
                      "VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, mapName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, spreeRecord);
    sqlite3_bind_text(stmt, 3, spreePlayer.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, (sqlite3_int64)spreeDate);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::unbanById(int banId) {
    if (!isOpen || !db) return false;

    const char* sql = "DELETE FROM bans WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, banId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::getBanList(std::vector<BanData>& bans) {
    if (!isOpen || !db) return false;

    bans.clear();
    
    const char* sql = "SELECT id, name, guid, hwid, ip, banned_by, ban_date, expires, reason "
                      "FROM bans ORDER BY id DESC;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BanData banData;
        banData.id = sqlite3_column_int(stmt, 0);
        
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        banData.name = name ? name : "";
        
        const char* guid = (const char*)sqlite3_column_text(stmt, 2);
        banData.guid = guid ? guid : "";
        
        const char* hwid = (const char*)sqlite3_column_text(stmt, 3);
        banData.hwid = hwid ? hwid : "";
        
        const char* ip = (const char*)sqlite3_column_text(stmt, 4);
        banData.ip = ip ? ip : "";
        
        const char* banned_by = (const char*)sqlite3_column_text(stmt, 5);
        banData.banned_by = banned_by ? banned_by : "";
        
        const char* ban_date = (const char*)sqlite3_column_text(stmt, 6);
        banData.ban_date = ban_date ? ban_date : "";
        
        banData.expires = (time_t)sqlite3_column_int64(stmt, 7);
        
        const char* reason = (const char*)sqlite3_column_text(stmt, 8);
        banData.reason = reason ? reason : "";
        
        bans.push_back(banData);
    }

    sqlite3_finalize(stmt);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

int Database::getBanCount() {
    if (!isOpen || !db) return 0;

    const char* sql = "SELECT COUNT(*) FROM bans;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::getUserList(std::vector<UserData>& users) {
    if (!isOpen || !db) return false;

    users.clear();
    
    const char* sql = "SELECT id, guid, level, lastSeen, name, hwid, title, commands, greeting, xp_skills, muted "
                      "FROM users ORDER BY lastSeen DESC;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserData data;
        data.id = sqlite3_column_int(stmt, 0);
        
        const char* guid = (const char*)sqlite3_column_text(stmt, 1);
        data.guid = guid ? guid : "";
        
        data.level = sqlite3_column_int(stmt, 2);
        data.lastSeen = (time_t)sqlite3_column_int64(stmt, 3);
        
        const char* name = (const char*)sqlite3_column_text(stmt, 4);
        data.name = name ? name : "";
        
        const char* hwid = (const char*)sqlite3_column_text(stmt, 5);
        data.hwid = hwid ? hwid : "";
        
        const char* title = (const char*)sqlite3_column_text(stmt, 6);
        data.title = title ? title : "";
        
        const char* commands = (const char*)sqlite3_column_text(stmt, 7);
        data.commands = commands ? commands : "";
        
        const char* greeting = (const char*)sqlite3_column_text(stmt, 8);
        data.greeting = greeting ? greeting : "";
        
        const char* xp_skills = (const char*)sqlite3_column_text(stmt, 9);
        data.xp_skills = xp_skills ? xp_skills : "";
        
        data.muted = sqlite3_column_int(stmt, 10) != 0;
        
        users.push_back(data);
    }

    sqlite3_finalize(stmt);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::searchUsersByName(const std::string& name, std::vector<UserData>& users) {
    if (!isOpen || !db) return false;

    users.clear();
    
    const char* sql = "SELECT id, guid, level, lastSeen, name, hwid, title, commands, greeting, xp_skills, muted "
                      "FROM users WHERE name LIKE ? ESCAPE '\\' ORDER BY lastSeen DESC;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    // Escape SQL LIKE wildcards in the search pattern
    std::string escapedName;
    for (std::string::const_iterator it = name.begin(); it != name.end(); ++it) {
        if (*it == '%' || *it == '_') {
            escapedName += '\\';
        }
        escapedName += *it;
    }
    std::string pattern = "%" + escapedName + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserData data;
        data.id = sqlite3_column_int(stmt, 0);
        
        const char* guid = (const char*)sqlite3_column_text(stmt, 1);
        data.guid = guid ? guid : "";
        
        data.level = sqlite3_column_int(stmt, 2);
        data.lastSeen = (time_t)sqlite3_column_int64(stmt, 3);
        
        const char* name_col = (const char*)sqlite3_column_text(stmt, 4);
        data.name = name_col ? name_col : "";
        
        const char* hwid = (const char*)sqlite3_column_text(stmt, 5);
        data.hwid = hwid ? hwid : "";
        
        const char* title = (const char*)sqlite3_column_text(stmt, 6);
        data.title = title ? title : "";
        
        const char* commands = (const char*)sqlite3_column_text(stmt, 7);
        data.commands = commands ? commands : "";
        
        const char* greeting = (const char*)sqlite3_column_text(stmt, 8);
        data.greeting = greeting ? greeting : "";
        
        const char* xp_skills = (const char*)sqlite3_column_text(stmt, 9);
        data.xp_skills = xp_skills ? xp_skills : "";
        
        data.muted = sqlite3_column_int(stmt, 10) != 0;
        
        users.push_back(data);
    }

    sqlite3_finalize(stmt);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::deleteUser(int id) {
    if (!isOpen || !db) return false;

    const char* sql = "DELETE FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::deleteUserByGuid(const std::string& guid) {
    if (!isOpen || !db) return false;

    const char* sql = "DELETE FROM users WHERE guid = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, guid.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

int Database::getUserCount() {
    if (!isOpen || !db) return 0;

    const char* sql = "SELECT COUNT(*) FROM users;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::resetAllXp() {
    if (!isOpen || !db) return false;

    const char* sql = "UPDATE users SET xp_skills = NULL;";
    return executeSQL(sql);
}

///////////////////////////////////////////////////////////////////////////////

bool Database::getOrCreateUser(const std::string& guid, const std::string& name, UserData& data) {
    if (!isOpen || !db) return false;

    // First try to get existing user
    if (getUserData(guid, data)) {
        return true;
    }

    // User doesn't exist, create new one
    if (!addUser(guid, "", name)) {
        return false;
    }

    // Now fetch the newly created user
    return getUserData(guid, data);
}

///////////////////////////////////////////////////////////////////////////////

int Database::migrateLevel(int fromLevel, int toLevel) {
    if (!isOpen || !db) return 0;

    // First count how many will be affected
    const char* countSql = "SELECT COUNT(*) FROM users WHERE level = ?;";
    sqlite3_stmt* countStmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, countSql, -1, &countStmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(countStmt, 1, fromLevel);
    int count = 0;
    if (sqlite3_step(countStmt) == SQLITE_ROW) {
        count = sqlite3_column_int(countStmt, 0);
    }
    sqlite3_finalize(countStmt);

    if (count == 0) {
        return 0;
    }

    // Now update all users from oldLevel to newLevel
    const char* sql = "UPDATE users SET level = ? WHERE level = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, toLevel);
    sqlite3_bind_int(stmt, 2, fromLevel);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? count : 0;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod
