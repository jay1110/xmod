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

    // User data operations
    bool getUserAndLevelData();
    bool writeSessionData();
    bool readSessionData();
};

} // namespace xmod

#endif // GAME_XMOD_SESSION_H
