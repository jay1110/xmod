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
    
    G_Printf("Session initialized for client %d (IP: %s)\n", clientNum, ip.c_str());
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
        G_Printf("Session::guidReceived - session not initialized\n");
        return false;
    }

    // Validate format
    if (!validateGuid(hashedGuid)) {
        G_Printf("Session::guidReceived - invalid GUID format (client %d)\n", clientNum);
        return false;
    }

    if (!validateHwid(hashedHwid)) {
        G_Printf("Session::guidReceived - invalid HWID format (client %d)\n", clientNum);
        return false;
    }

    guid = hashedGuid;
    hwid = hashedHwid;

    // Check database
    if (!db || !db->isOpened()) {
        G_Printf("Session::guidReceived - database not available\n");
        return false;
    }

    // Check for ban
    BanData banData;
    if (db->isBanned(guid, hwid, banData)) {
        G_Printf("Client %d is banned: %s\n", clientNum, banData.reason.c_str());
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

        G_Printf("Client %d authenticated as user %d (level %d)\n", clientNum, userId, userLevel);
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

        if (db->addUser(guid, hwid, clientName)) {
            // Get the newly created user
            if (db->getUserData(guid, userData)) {
                userId = userData.id;
                userLevel = userData.level;
                authenticated = true;
                G_Printf("New user created for client %d (user ID: %d)\n", clientNum, userId);
            }
        } else {
            G_Printf("Failed to create user for client %d\n", clientNum);
            return false;
        }
    }

    // Sync user data from SQLite to runtime User object
    if (authenticated && clientNum >= 0 && clientNum < MAX_CLIENTS) {
        if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
            connectedUsers[clientNum]->authLevel = userLevel;
            connectedUsers[clientNum]->muted = userData.muted;
            G_Printf("Synced authLevel %d, muted=%d for client %d from SQLite\n", 
                     userLevel, userData.muted ? 1 : 0, clientNum);
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
