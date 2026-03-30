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
        // Don't return here - still need to initialize sessions even without database
    }
    
    // Initialize session slots (even if database failed to open)
    // Sessions can operate without database, they just won't persist data
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!g_sessions[i]) {
            g_sessions[i] = new Session(g_database);
        }
    }
    
    // Legacy userDB migration is no longer supported (removed)
    // All user data is now managed through SQLite xmod.db database
    if (g_database && g_database->isOpened()) {
        G_Printf("xmod SQLite database initialized successfully\n");
    } else {
        G_Printf("^3[SQLite] Database unavailable, sessions will operate without persistence\n");
    }
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
// Global helper functions for accessing session data
///////////////////////////////////////////////////////////////////////////////

bool isClientMuted(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return false;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->isMuted();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->muted;
    }
    
    return false;
}

void setClientMuted(int clientNum, bool muted) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    // Set on session
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setMuted(muted);
    }
    
    // Also set on User for backward compatibility
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        connectedUsers[clientNum]->muted = muted;
    }
}

bool isClientNospammed(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return false;
    }
    
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        if (!g_sessions[clientNum]->isNospammed()) {
            return false;
        }
        // Check if nospam has expired
        time_t expiry = g_sessions[clientNum]->getNospamExpiry();
        if (expiry > 0 && time(NULL) >= expiry) {
            // Expired - auto-clear
            g_sessions[clientNum]->setNospammed(false);
            g_sessions[clientNum]->setNospamExpiry(0);
            g_sessions[clientNum]->setNospamLastChat(0);
            return false;
        }
        return true;
    }
    
    return false;
}

void setClientNospammed(int clientNum, bool nospammed) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setNospammed(nospammed);
    }
}

void setClientNospamExpiry(int clientNum, time_t expiry) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setNospamExpiry(expiry);
    }
}

time_t getClientNospamExpiry(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->getNospamExpiry();
    }
    
    return 0;
}

time_t getClientNospamLastChat(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->getNospamLastChat();
    }
    
    return 0;
}

void setClientNospamLastChat(int clientNum, time_t lastChat) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setNospamLastChat(lastChat);
    }
}

bool isClientNospamAllowed(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return true;
    }
    
    if (!isClientNospammed(clientNum)) {
        return true;
    }
    
    time_t now = time(NULL);
    time_t lastChat = getClientNospamLastChat(clientNum);
    
    // Allow if 60 seconds have passed since last message
    if (now - lastChat >= 60) {
        setClientNospamLastChat(clientNum, now);
        return true;
    }
    
    return false;
}

const std::string& getClientGuid(int clientNum) {
    static const std::string empty = "";
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return empty;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized() && 
        !g_sessions[clientNum]->getGuid().empty()) {
        return g_sessions[clientNum]->getGuid();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->guid;
    }
    
    return empty;
}

const std::string& getClientName(int clientNum) {
    static const std::string empty = "";
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return empty;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized() && 
        !g_sessions[clientNum]->getName().empty()) {
        return g_sessions[clientNum]->getName();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->name;
    }
    
    return empty;
}

const std::string& getClientNamex(int clientNum) {
    static const std::string empty = "";
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return empty;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized() && 
        !g_sessions[clientNum]->getNamex().empty()) {
        return g_sessions[clientNum]->getNamex();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->namex;
    }
    
    return empty;
}

int getClientLevel(int clientNum) {
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isAuthenticated()) {
        return g_sessions[clientNum]->getUserLevel();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->authLevel;
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void setClientFakeGuid(int clientNum, bool fakeguid)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    // Set on session if available
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setFakeGuid(fakeguid);
    }
    
    // Also set on User for backward compatibility
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        connectedUsers[clientNum]->fakeguid = fakeguid;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool isClientFakeGuid(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return false;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->isFakeGuid();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->fakeguid;
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////

time_t getClientMuteExpiry(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->getMuteExpiry();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->muteExpiry;
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool hasClientPrivilege(int clientNum, const Privilege& privilege)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return false;
    }
    
    // Use User for privilege checks (User has privDenied/privGranted)
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->hasPrivilege(privilege);
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void setClientMuteData(int clientNum, time_t muteTime, const std::string& reason, 
                       const std::string& authority, const std::string& authorityx, time_t expiry)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    // Set on session if available
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setMuteTime(muteTime);
        g_sessions[clientNum]->setMuteReason(reason);
        g_sessions[clientNum]->setMuteAuthority(authority);
        g_sessions[clientNum]->setMuteAuthorityx(authorityx);
        g_sessions[clientNum]->setMuteExpiry(expiry);
    }
    
    // Also set on User for backward compatibility
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        connectedUsers[clientNum]->muteTime = muteTime;
        connectedUsers[clientNum]->muteReason = reason;
        connectedUsers[clientNum]->muteAuthority = authority;
        connectedUsers[clientNum]->muteAuthorityx = authorityx;
        connectedUsers[clientNum]->muteExpiry = expiry;
    }
}

///////////////////////////////////////////////////////////////////////////////

void clearClientMuteData(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    // Clear on session if available
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setMuteTime(0);
        g_sessions[clientNum]->setMuteReason("");
        g_sessions[clientNum]->setMuteAuthority("");
        g_sessions[clientNum]->setMuteAuthorityx("");
        g_sessions[clientNum]->setMuteExpiry(0);
    }
    
    // Also clear on User for backward compatibility
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        connectedUsers[clientNum]->muteTime = 0;
        connectedUsers[clientNum]->muteReason = "";
        connectedUsers[clientNum]->muteAuthority = "";
        connectedUsers[clientNum]->muteAuthorityx = "";
        connectedUsers[clientNum]->muteExpiry = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

static const std::string EMPTY_STRING;

const std::string& getClientIp(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return EMPTY_STRING;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->getIp();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->ip;
    }
    
    return EMPTY_STRING;
}

///////////////////////////////////////////////////////////////////////////////

void setClientLevel(int clientNum, int level)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    // Set on session if available
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setUserLevel(level);
    }
    
    // Also set on User for backward compatibility
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        connectedUsers[clientNum]->authLevel = level;
    }
}

///////////////////////////////////////////////////////////////////////////////

const std::string& getClientMac(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return EMPTY_STRING;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->getMac();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->mac;
    }
    
    return EMPTY_STRING;
}

///////////////////////////////////////////////////////////////////////////////

time_t getClientTimestamp(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->getTimestamp();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->timestamp;
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void setClientTimestamp(int clientNum, time_t ts)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    // Set on session if available
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setTimestamp(ts);
    }
    
    // Also set on User for backward compatibility
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        connectedUsers[clientNum]->timestamp = ts;
    }
}

///////////////////////////////////////////////////////////////////////////////

const std::string& getClientGreetingText(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return EMPTY_STRING;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized() &&
        !g_sessions[clientNum]->getGreetingText().empty()) {
        return g_sessions[clientNum]->getGreetingText();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->greetingText;
    }
    
    return EMPTY_STRING;
}

///////////////////////////////////////////////////////////////////////////////

void setClientGreetingText(int clientNum, const std::string& text)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    // Set on session if available
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setGreetingText(text);
    }
    
    // Also set on User for backward compatibility
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        connectedUsers[clientNum]->greetingText = text;
    }
}

///////////////////////////////////////////////////////////////////////////////

const std::string& getClientGreetingAudio(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return EMPTY_STRING;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized() &&
        !g_sessions[clientNum]->getGreetingAudio().empty()) {
        return g_sessions[clientNum]->getGreetingAudio();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->greetingAudio;
    }
    
    return EMPTY_STRING;
}

///////////////////////////////////////////////////////////////////////////////

void setClientGreetingAudio(int clientNum, const std::string& audio)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }
    
    // Set on session if available
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        g_sessions[clientNum]->setGreetingAudio(audio);
    }
    
    // Also set on User for backward compatibility
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        connectedUsers[clientNum]->greetingAudio = audio;
    }
}

///////////////////////////////////////////////////////////////////////////////

const std::string& getClientMuteAuthorityx(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return EMPTY_STRING;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized() &&
        !g_sessions[clientNum]->getMuteAuthorityx().empty()) {
        return g_sessions[clientNum]->getMuteAuthorityx();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->muteAuthorityx;
    }
    
    return EMPTY_STRING;
}

///////////////////////////////////////////////////////////////////////////////

time_t getClientBanExpiry(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    // Fallback to User (ban expiry not stored in Session)
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->banExpiry;
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

time_t getClientMuteTime(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized()) {
        return g_sessions[clientNum]->getMuteTime();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->muteTime;
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const std::string& getClientMuteReason(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return EMPTY_STRING;
    }
    
    // Try session first
    if (g_sessions[clientNum] && g_sessions[clientNum]->isInitialized() &&
        !g_sessions[clientNum]->getMuteReason().empty()) {
        return g_sessions[clientNum]->getMuteReason();
    }
    
    // Fallback to User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->muteReason;
    }
    
    return EMPTY_STRING;
}

///////////////////////////////////////////////////////////////////////////////

const PrivilegeSet* getClientPrivGranted(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return nullptr;
    }
    
    // Only available from User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->privGranted;
    }
    
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

const PrivilegeSet* getClientPrivDenied(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return nullptr;
    }
    
    // Only available from User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->privDenied;
    }
    
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

static const std::vector<std::string> EMPTY_NOTES;

const std::vector<std::string>& getClientNotes(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return EMPTY_NOTES;
    }
    
    // Only available from User
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return connectedUsers[clientNum]->notes;
    }
    
    return EMPTY_NOTES;
}

///////////////////////////////////////////////////////////////////////////////

const User& getClientUser(int clientNum)
{
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return User::BAD;
    }
    
    if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
        return *connectedUsers[clientNum];
    }
    
    return User::BAD;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod
