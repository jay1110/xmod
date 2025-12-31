#include <bgame/impl.h>
#include "xmod_globals.h"

// Reference to global CVAR
extern vmCvar_t g_userConfig;

namespace xmod {

///////////////////////////////////////////////////////////////////////////////

// Global database instance
Database* g_database = nullptr;

// Session instances (one per client slot)
Session* g_sessions[MAX_CLIENTS] = { nullptr };

///////////////////////////////////////////////////////////////////////////////

void initXmod() {
    // Check if already initialized
    if (g_database && g_database->isOpened()) {
        G_Printf("xmod already initialized, skipping re-initialization\n");
        return;
    }
    
    G_Printf("Initializing xmod SQLite database...\n");
    
    // Create database instance if needed
    if (!g_database) {
        g_database = new Database();
    }
    
    // Get database filename from CVAR
    const char* dbFilename = g_userConfig.string;
    
    if (!dbFilename || !dbFilename[0]) {
        dbFilename = "xmod.db";
    }
    
    // Ensure filename is just a filename, not a path (security: prevent directory traversal)
    // Strip any path separators to ensure database stays in mod folder
    std::string safeFilename = dbFilename;
    size_t lastSlash = safeFilename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        safeFilename = safeFilename.substr(lastSlash + 1);
        G_Printf("^3[SQLite] Stripped path from filename, using: %s\n", safeFilename.c_str());
    }
    
    // Construct full path: fs_homepath/fs_game/xmod.db
    // This places database in the mod directory (e.g., ~/.etwolf/xmod/xmod.db)
    // This ensures 32-bit and 64-bit use the same database file
    char buffer[MAX_CVAR_VALUE_STRING];
    std::string dbPath;
    
    trap_Cvar_VariableStringBuffer("fs_homepath", buffer, sizeof(buffer));
    dbPath = buffer;
    dbPath += "/";
    
    trap_Cvar_VariableStringBuffer("fs_game", buffer, sizeof(buffer));
    dbPath += buffer;
    dbPath += "/";
    dbPath += safeFilename;
    
    G_Printf("^2[SQLite] Database will be created in mod folder: %s\n", dbPath.c_str());
    
    // Open database with full path
    if (!g_database->open(dbPath)) {
        G_Printf("^1ERROR: Failed to open xmod database: %s\n", dbPath.c_str());
        delete g_database;
        g_database = nullptr;
        return;
    }
    
    // Initialize session slots
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!g_sessions[i]) {
            g_sessions[i] = new Session(g_database);
        }
    }
    
    // Check if we should migrate from legacy userDB
    // Only migrate if legacy userDB has users and SQLite is empty or has few users
    int sqliteUserCount = g_database->getUserCount();
    int legacyUserCount = (int)userDB.mapGUID.size();
    
    if (legacyUserCount > 0) {
        G_Printf("^3[SQLite] Found %d users in legacy userDB\n", legacyUserCount);
        
        if (sqliteUserCount == 0) {
            G_Printf("^3[SQLite] SQLite database is empty, performing migration...\n");
            g_database->importFromLegacyUserDB();
        } else if (sqliteUserCount < legacyUserCount) {
            G_Printf("^3[SQLite] SQLite has %d users, legacy has %d. Consider running !dbmigrate to sync.\n", 
                     sqliteUserCount, legacyUserCount);
        } else {
            G_Printf("^2[SQLite] Database already populated with %d users\n", sqliteUserCount);
        }
    } else {
        G_Printf("^2[SQLite] No legacy users to migrate\n");
    }
    
    G_Printf("xmod SQLite database initialized successfully\n");
}

///////////////////////////////////////////////////////////////////////////////

void shutdownXmod() {
    G_Printf("Shutting down xmod...\n");
    
    // Clean up sessions
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_sessions[i]) {
            delete g_sessions[i];
            g_sessions[i] = nullptr;
        }
    }
    
    // Close and cleanup database
    if (g_database) {
        g_database->close();
        delete g_database;
        g_database = nullptr;
    }
    
    G_Printf("xmod shutdown complete\n");
}

///////////////////////////////////////////////////////////////////////////////

void updateClientSession(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    if (!g_sessions[clientNum]) {
        return;
    }
    
    g_sessions[clientNum]->writeSessionData();
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod
