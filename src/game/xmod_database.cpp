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
        "muted INTEGER DEFAULT 0,"
        "muteTime INTEGER DEFAULT 0,"
        "muteExpiry INTEGER DEFAULT 0,"
        "muteReason TEXT,"
        "muteAuthority TEXT"
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

    bool result = executeSQL(sql_users) &&
           executeSQL(sql_bans) &&
           executeSQL(sql_levels) &&
           executeSQL(sql_maps) &&
           executeSQL(sql_names) &&
           executeSQL(sql_indexes);
    
    // Add mute columns if they don't exist (migration for existing databases)
    if (result) {
        // These will fail silently if columns already exist
        executeSQL("ALTER TABLE users ADD COLUMN muteTime INTEGER DEFAULT 0;");
        executeSQL("ALTER TABLE users ADD COLUMN muteExpiry INTEGER DEFAULT 0;");
        executeSQL("ALTER TABLE users ADD COLUMN muteReason TEXT;");
        executeSQL("ALTER TABLE users ADD COLUMN muteAuthority TEXT;");
    }
    
    return result;
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

    const char* sql = "SELECT id, guid, level, lastSeen, name, hwid, title, commands, greeting, xp_skills, "
                      "muted, muteTime, muteExpiry, muteReason, muteAuthority "
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
        data.muteTime = (time_t)sqlite3_column_int64(stmt, 11);
        data.muteExpiry = (time_t)sqlite3_column_int64(stmt, 12);
        
        const char* muteReason = (const char*)sqlite3_column_text(stmt, 13);
        data.muteReason = muteReason ? muteReason : "";
        
        const char* muteAuthority = (const char*)sqlite3_column_text(stmt, 14);
        data.muteAuthority = muteAuthority ? muteAuthority : "";
        
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::getUserDataById(int id, UserData& data) {
    if (!isOpen || !db) return false;

    const char* sql = "SELECT id, guid, level, lastSeen, name, hwid, title, commands, greeting, xp_skills, "
                      "muted, muteTime, muteExpiry, muteReason, muteAuthority "
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
        data.muteTime = (time_t)sqlite3_column_int64(stmt, 11);
        data.muteExpiry = (time_t)sqlite3_column_int64(stmt, 12);
        
        const char* muteReason = (const char*)sqlite3_column_text(stmt, 13);
        data.muteReason = muteReason ? muteReason : "";
        
        const char* muteAuthority = (const char*)sqlite3_column_text(stmt, 14);
        data.muteAuthority = muteAuthority ? muteAuthority : "";
        
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

bool Database::setMuteData(int userId, bool muted, time_t muteTime, time_t muteExpiry, 
                          const std::string& reason, const std::string& authority) {
    if (!isOpen || !db) return false;

    const char* sql = "UPDATE users SET muted = ?, muteTime = ?, muteExpiry = ?, "
                      "muteReason = ?, muteAuthority = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, muted ? 1 : 0);
    sqlite3_bind_int64(stmt, 2, (sqlite3_int64)muteTime);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)muteExpiry);
    sqlite3_bind_text(stmt, 4, reason.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, authority.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, userId);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

///////////////////////////////////////////////////////////////////////////////

bool Database::getUserByGuid(const std::string& guid, UserData& data) {
    // Alias for getUserData for compatibility
    return getUserData(guid, data);
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

    // Only return true if rows were actually deleted
    return rc == SQLITE_DONE && sqlite3_changes(db) > 0;
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

    // Only return true if rows were actually deleted
    return rc == SQLITE_DONE && sqlite3_changes(db) > 0;
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

int Database::importFromLegacyUserDB() {
    if (!isOpen || !db) return 0;

    G_Printf("^3[SQLite] Starting migration from legacy userDB...\n");
    
    const int EXPECTED_HWID_LENGTH = 40;  // SHA1 hash length
    int importedCount = 0;
    int updatedCount = 0;
    int skippedCount = 0;
    
    // Iterate through all users in the legacy userDB
    for (UserDB::mapGUID_t::const_iterator it = userDB.mapGUID.begin(); 
         it != userDB.mapGUID.end(); ++it) {
        const User& legacyUser = it->second;
        
        // Skip fake GUIDs
        if (legacyUser.fakeguid) {
            skippedCount++;
            continue;
        }
        
        // Skip null/bad users
        if (legacyUser.isNull()) {
            skippedCount++;
            continue;
        }
        
        std::string guid = legacyUser.guid;
        std::string name = legacyUser.name;
        
        // Check if user already exists in SQLite
        UserData existingData;
        if (getUserData(guid, existingData)) {
            // User exists, update their data if needed
            bool needsUpdate = false;
            
            // Update level if different
            if (existingData.level != legacyUser.authLevel) {
                setLevel(existingData.id, legacyUser.authLevel);
                needsUpdate = true;
            }
            
            // Update name if different
            if (existingData.name != name && !name.empty()) {
                updateName(existingData.id, name);
                needsUpdate = true;
            }
            
            // Update muted status
            if (existingData.muted != legacyUser.muted) {
                setMuted(existingData.id, legacyUser.muted);
                needsUpdate = true;
            }
            
            // Import HWIDs
            for (size_t i = 0; i < legacyUser.hwids.size(); i++) {
                const std::string& hwid = legacyUser.hwids[i];
                if (!hwid.empty() && hwid.length() == EXPECTED_HWID_LENGTH) {
                    addHwid(existingData.id, hwid);
                    needsUpdate = true;
                }
            }
            
            if (needsUpdate) {
                updatedCount++;
                G_Printf("^2[SQLite] Updated user: %s (ID=%d, level=%d)\n", 
                         name.c_str(), existingData.id, legacyUser.authLevel);
            } else {
                skippedCount++;
            }
        } else {
            // User doesn't exist, create new entry
            std::string hwid;
            if (!legacyUser.hwids.empty() && legacyUser.hwids[0].length() == EXPECTED_HWID_LENGTH) {
                hwid = legacyUser.hwids[0];
            }
            
            if (addUser(guid, hwid, name)) {
                // Get the newly created user to set additional properties
                UserData newData;
                if (getUserData(guid, newData)) {
                    // Set auth level
                    if (legacyUser.authLevel > 0) {
                        setLevel(newData.id, legacyUser.authLevel);
                    }
                    
                    // Set muted status
                    if (legacyUser.muted) {
                        setMuted(newData.id, true);
                    }
                    
                    // Add additional HWIDs (skip first one, already added)
                    for (size_t i = 1; i < legacyUser.hwids.size(); i++) {
                        const std::string& additionalHwid = legacyUser.hwids[i];
                        if (!additionalHwid.empty() && additionalHwid.length() == EXPECTED_HWID_LENGTH) {
                            addHwid(newData.id, additionalHwid);
                        }
                    }
                    
                    importedCount++;
                    G_Printf("^2[SQLite] Imported user: %s (ID=%d, level=%d)\n", 
                             name.c_str(), newData.id, legacyUser.authLevel);
                }
            } else {
                G_Printf("^1[SQLite] Failed to import user: %s (GUID=%s)\n", 
                         name.c_str(), guid.substr(0, 8).c_str());
            }
        }
    }
    
    G_Printf("^2[SQLite] Migration complete: %d imported, %d updated, %d skipped\n", 
             importedCount, updatedCount, skippedCount);
    
    return importedCount + updatedCount;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod
