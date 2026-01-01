#include <bgame/impl.h>
#include "xmod_session.h"
#include "xmod_database.h"
#include <cctype>

namespace xmod {

///////////////////////////////////////////////////////////////////////////////

static bool validateSha1Hash(const std::string& hash) {
    // SHA1 hash must be exactly 40 hexadecimal characters
    if (hash.length() != 40) {
        return false;
    }

    for (size_t i = 0; i < hash.length(); i++) {
        char c = hash[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

Session::Session(Database* database) 
    : clientNum(-1), userId(-1), userLevel(0), sessionStartTime(0),
      initialized(false), authenticated(false), db(database) {
}

Session::~Session() {
    reset();
}

///////////////////////////////////////////////////////////////////////////////

void Session::init(int client, const std::string& clientIp) {
    reset();
    
    clientNum = client;
    ip = clientIp;
    sessionStartTime = time(nullptr);
    initialized = true;
    authenticated = false;
    
    G_Printf("^2[SQLite] Session initialized for client %d (IP: %s)\n", clientNum, ip.c_str());
}

///////////////////////////////////////////////////////////////////////////////

void Session::reset() {
    clientNum = -1;
    guid.clear();
    hwid.clear();
    ip.clear();
    userId = -1;
    userLevel = 0;
    sessionStartTime = 0;
    initialized = false;
    authenticated = false;
}

///////////////////////////////////////////////////////////////////////////////

bool Session::validateGuid(const std::string& guidStr) {
    return validateSha1Hash(guidStr);
}

///////////////////////////////////////////////////////////////////////////////

bool Session::validateHwid(const std::string& hwidStr) {
    return validateSha1Hash(hwidStr);
}

///////////////////////////////////////////////////////////////////////////////

bool Session::guidReceived(const std::string& hashedGuid, const std::string& hashedHwid) {
    if (!initialized) {
        G_Printf("^1[SQLite] ERROR: Session::guidReceived - session not initialized\n");
        return false;
    }

    // Validate format
    if (!validateGuid(hashedGuid)) {
        G_Printf("^1[SQLite] ERROR: Invalid GUID format for client %d (length=%d)\n", clientNum, (int)hashedGuid.length());
        return false;
    }

    if (!validateHwid(hashedHwid)) {
        G_Printf("^1[SQLite] ERROR: Invalid HWID format for client %d (length=%d)\n", clientNum, (int)hashedHwid.length());
        return false;
    }

    guid = hashedGuid;
    hwid = hashedHwid;

    G_Printf("^2[SQLite] Client %d - GUID: %s, HWID: %s\n", clientNum, guid.c_str(), hwid.substr(0, 8).c_str());

    // Check database availability
    if (!db || !db->isOpened()) {
        G_Printf("^3[SQLite] WARNING: Database not available, using legacy authentication for client %d\n", clientNum);
        // Mark as authenticated without database - legacy system will handle authentication
        authenticated = true;
        return true;
    }

    // Check for ban
    BanData banData;
    if (db->isBanned(guid, hwid, banData)) {
        G_Printf("^1[SQLite] Client %d is BANNED: %s\n", clientNum, banData.reason.c_str());
        return false;
    }

    // Check if user exists
    UserData userData;
    if (db->getUserData(guid, userData)) {
        // Existing user
        userId = userData.id;
        userLevel = userData.level;
        authenticated = true;

        // Update last seen
        db->updateLastSeen(userId, time(nullptr));

        // Add new HWID if not already present (addHwid handles duplicate checking)
        if (!hwid.empty()) {
            db->addHwid(userId, hwid);
        }

        G_Printf("^2[SQLite] Client %d authenticated as EXISTING user (ID=%d, level=%d, name=%s)\n", 
                 clientNum, userId, userLevel, userData.name.c_str());
    } else {
        // New user - create entry
        std::string clientName = "UnknownPlayer";
        
        // Try to get name from gclient
        if (clientNum >= 0 && clientNum < MAX_CLIENTS) {
            gclient_t* client = &level.clients[clientNum];
            if (client && client->pers.netname[0]) {
                clientName = client->pers.netname;
            }
        }

        G_Printf("^2[SQLite] Creating NEW user for client %d (name=%s, GUID=%s)\n", 
                 clientNum, clientName.c_str(), guid.substr(0, 8).c_str());

        if (db->addUser(guid, hwid, clientName)) {
            // Get the newly created user
            if (db->getUserData(guid, userData)) {
                userId = userData.id;
                userLevel = userData.level;
                authenticated = true;
                G_Printf("^2[SQLite] SUCCESS: New user created (ID=%d, level=%d)\n", userId, userLevel);
            } else {
                G_Printf("^1[SQLite] ERROR: Failed to retrieve newly created user\n");
                return false;
            }
        } else {
            G_Printf("^1[SQLite] ERROR: Failed to create user in database\n");
            return false;
        }
    }

    // Sync user data from SQLite to runtime User object
    if (authenticated && clientNum >= 0 && clientNum < MAX_CLIENTS) {
        if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
            connectedUsers[clientNum]->authLevel = userLevel;
            connectedUsers[clientNum]->muted = userData.muted;
            G_Printf("^2[SQLite] Synced user %d: authLevel=%d, muted=%d for client %d\n", 
                     userId, userLevel, userData.muted ? 1 : 0, clientNum);
        } else {
            G_Printf("^3[SQLite] WARNING: connectedUsers[%d] is NULL or BAD, cannot sync authLevel\n", clientNum);
        }
    }

    return authenticated;
}

///////////////////////////////////////////////////////////////////////////////

void Session::onGuidReceived(const std::string& hashedGuid, const std::string& hashedHwid) {
    if (!guidReceived(hashedGuid, hashedHwid)) {
        // Authentication failed - disconnect client
        if (clientNum >= 0 && clientNum < MAX_CLIENTS) {
            trap_DropClient(clientNum, "Authentication failed", 0);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Session::getUserAndLevelData() {
    if (!authenticated || userId < 0) {
        return false;
    }

    if (!db || !db->isOpened()) {
        return false;
    }

    UserData userData;
    if (db->getUserDataById(userId, userData)) {
        userLevel = userData.level;
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool Session::writeSessionData() {
    if (!authenticated || userId < 0) {
        return false;
    }

    if (!db || !db->isOpened()) {
        return false;
    }

    // Update last seen time
    time_t now = time(nullptr);
    return db->updateLastSeen(userId, now);
}

///////////////////////////////////////////////////////////////////////////////

bool Session::readSessionData() {
    if (!initialized) {
        return false;
    }

    if (!db || !db->isOpened()) {
        return false;
    }

    // This would be called on reconnect to restore session
    // For now, we'll rely on the authenticate flow
    return true;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod
