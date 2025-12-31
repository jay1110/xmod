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
    
    // Construct full path using fs_homepath and fs_game (same as legacy Database class)
    // This ensures 32-bit and 64-bit use the same database file
    char buffer[MAX_CVAR_VALUE_STRING];
    std::string dbPath;
    
    trap_Cvar_VariableStringBuffer("fs_homepath", buffer, sizeof(buffer));
    dbPath = buffer;
    dbPath += "/";
    
    trap_Cvar_VariableStringBuffer("fs_game", buffer, sizeof(buffer));
    dbPath += buffer;
    dbPath += "/";
    dbPath += dbFilename;
    
    G_Printf("Opening SQLite database at: %s\n", dbPath.c_str());
    
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
