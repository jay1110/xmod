#ifndef GAME_XMOD_SESSION_H
#define GAME_XMOD_SESSION_H

#include <string>
#include <ctime>

///////////////////////////////////////////////////////////////////////////////
// Session management for client authentication
///////////////////////////////////////////////////////////////////////////////

namespace xmod {

class Database;

class Session {
private:
    int clientNum;
    std::string guid;
    std::string hwid;
    std::string ip;
    int userId;
    int userLevel;
    time_t sessionStartTime;
    bool initialized;
    bool authenticated;

    // Additional attributes from User class for session tracking
    bool muted;
    time_t muteTime;
    time_t muteExpiry;
    std::string muteReason;
    std::string muteAuthority;
    bool fakeguid;
    std::string name;
    std::string namex;
    std::string mac;

    Database* db;

    bool validateGuid(const std::string& guid);
    bool validateHwid(const std::string& hwid);

public:
    Session(Database* database);
    ~Session();

    // Session lifecycle
    void init(int clientNum, const std::string& ip);
    void reset();
    bool isInitialized() const { return initialized; }
    bool isAuthenticated() const { return authenticated; }

    // GUID/HWID handling
    bool guidReceived(const std::string& hashedGuid, const std::string& hashedHwid);
    void onGuidReceived(const std::string& hashedGuid, const std::string& hashedHwid);

    // Getters
    int getClientNum() const { return clientNum; }
    const std::string& getGuid() const { return guid; }
    const std::string& getHwid() const { return hwid; }
    const std::string& getIp() const { return ip; }
    int getUserId() const { return userId; }
    int getUserLevel() const { return userLevel; }
    
    // Additional getters for session attributes
    bool isMuted() const { return muted; }
    time_t getMuteTime() const { return muteTime; }
    time_t getMuteExpiry() const { return muteExpiry; }
    const std::string& getMuteReason() const { return muteReason; }
    const std::string& getMuteAuthority() const { return muteAuthority; }
    bool isFakeGuid() const { return fakeguid; }
    const std::string& getName() const { return name; }
    const std::string& getNamex() const { return namex; }
    const std::string& getMac() const { return mac; }
    
    // Setters for session attributes
    void setMuted(bool value) { muted = value; }
    void setMuteTime(time_t value) { muteTime = value; }
    void setMuteExpiry(time_t value) { muteExpiry = value; }
    void setMuteReason(const std::string& value) { muteReason = value; }
    void setMuteAuthority(const std::string& value) { muteAuthority = value; }
    void setFakeGuid(bool value) { fakeguid = value; }
    void setName(const std::string& value) { name = value; }
    void setNamex(const std::string& value) { namex = value; }
    void setMac(const std::string& value) { mac = value; }
    void setUserLevel(int level) { userLevel = level; }

    // User data operations
    bool getUserAndLevelData();
    bool writeSessionData();
    bool readSessionData();
};

} // namespace xmod

#endif // GAME_XMOD_SESSION_H
