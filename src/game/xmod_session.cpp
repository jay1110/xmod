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
      initialized(false), authenticated(false), db(database),
      muted(false), muteTime(0), muteExpiry(0),
      nospammed(false), nospamExpiry(0), nospamLastChat(0),
      fakeguid(false), timestamp(0) {
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
    
    // Reset additional attributes
    muted = false;
    muteTime = 0;
    muteExpiry = 0;
    muteReason.clear();
    muteAuthority.clear();
    muteAuthorityx.clear();
    fakeguid = false;
    nospammed = false;
    nospamExpiry = 0;
    nospamLastChat = 0;
    name.clear();
    namex.clear();
    mac.clear();
    timestamp = 0;
    greetingText.clear();
    greetingAudio.clear();
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

    // Check database
    if (!db || !db->isOpened()) {
        G_Printf("^1[SQLite] ERROR: Database not available\n");
        return false;
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
        fakeguid = false;

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
                fakeguid = false;
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
    // This maintains backward compatibility with legacy UserDB system
    if (authenticated && clientNum >= 0 && clientNum < MAX_CLIENTS) {
        if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
            // Get current client info to sync MAC address and name
            gclient_t* client = &level.clients[clientNum];
            char userinfo[MAX_INFO_STRING];
            trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
            
            // Get MAC address from userinfo
            mac = Info_ValueForKey(userinfo, "cl_mac");
            if (!mac.empty()) {
                // Convert to lowercase for consistency
                str::toLower(mac);
            }
            
            // Store session data locally for quick access
            muted = userData.muted;
            muteTime = userData.muteTime;
            muteExpiry = userData.muteExpiry;
            muteReason = userData.muteReason;
            muteAuthority = userData.muteAuthority;
            name = userData.name;
            
            // Get namex from gclient if available, otherwise use plain name
            if (client && client->pers.netname[0]) {
                namex = client->pers.netname;  // This includes color codes
            } else {
                namex = userData.name;
            }
            
            // Sync session data to User object
            connectedUsers[clientNum]->authLevel = userLevel;
            connectedUsers[clientNum]->muted = userData.muted;
            connectedUsers[clientNum]->muteTime = userData.muteTime;
            connectedUsers[clientNum]->muteExpiry = userData.muteExpiry;
            connectedUsers[clientNum]->muteReason = userData.muteReason;
            connectedUsers[clientNum]->muteAuthority = userData.muteAuthority;
            connectedUsers[clientNum]->fakeguid = fakeguid;
            
            // Sync critical fields that were previously missing
            connectedUsers[clientNum]->ip = ip;
            connectedUsers[clientNum]->mac = mac;
            connectedUsers[clientNum]->name = name;
            connectedUsers[clientNum]->namex = namex;
            
            G_Printf("^2[SQLite] Synced user %d: authLevel=%d, muted=%d, ip=%s, mac=%s for client %d\n", 
                     userId, userLevel, userData.muted ? 1 : 0, ip.c_str(), mac.substr(0, 8).c_str(), clientNum);
        } else {
            G_Printf("^3[SQLite] WARNING: connectedUsers[%d] is NULL or BAD, cannot sync data\n", clientNum);
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
