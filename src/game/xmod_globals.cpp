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
    G_Printf("Initializing xmod SQLite database...\n");
    
    // Create database instance
    if (!g_database) {
        g_database = new Database();
    }
    
    // Get database path from CVAR
    const char* dbPath = g_userConfig.string;
    
    if (!dbPath || !dbPath[0]) {
        dbPath = "xmod.db";
    }
    
    // Open database
    if (!g_database->open(dbPath)) {
        G_Printf("^1ERROR: Failed to open xmod database: %s\n", dbPath);
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
