#include <bgame/impl.h>
#include <game/UserManager.h>

///////////////////////////////////////////////////////////////////////////////

UserManager::UserManager()
{
}

///////////////////////////////////////////////////////////////////////////////

UserManager::~UserManager()
{
}

///////////////////////////////////////////////////////////////////////////////

User&
UserManager::fetchByKey(const std::string& guid, std::string& err, bool create)
{
    if (guid.length() != 40) {
        err = "length must be 40 chars";
        return User::BAD;
    }

    std::string key = guid;
    str::toLower(key);

    mapGUID_t::iterator found = _mapGUID.find(key);
    if (found != _mapGUID.end())
        return found->second;

    if (!create) {
        err = "not found";
        return User::BAD;
    }

    // create
    User& user = _mapGUID[key];
    user = User::DEFAULT;  // inherit default values
    user._guid = key;      // assign correct key

    return user;
}

///////////////////////////////////////////////////////////////////////////////

void
UserManager::remove(User& obj)
{
    _mapGUID.erase(obj._guid);
}

///////////////////////////////////////////////////////////////////////////////

User&
UserManager::fetchByID(const std::string& id, std::string& err)
{
    const std::string::size_type idlen = id.length();
    if (idlen < 8) {
        err = "must be at least 8 chars long";
        return User::BAD;
    }
    if (idlen > 40) {
        err = "exceeds maximum of 40 chars";
        return User::BAD;
    }

    const std::string::size_type foundpos = 40 - idlen;

    std::string lid = id;
    str::toLower(lid);

    int count = 0;
    User* found = NULL;
    const mapGUID_t::iterator max = _mapGUID.end();
    for (mapGUID_t::iterator it = _mapGUID.begin(); it != max; it++) {
        User& user = it->second;
        if (user.guid.rfind(lid) == foundpos) {
            found = &user;
            count++;
        }
    }

    if (count == 0) {
        err = "not found";
        return User::BAD;
    }
    else if (count == 1) {
        return *found;
    }
    else {
        err = "ambiguous";
        return User::BAD;
    }
}

///////////////////////////////////////////////////////////////////////////////

UserManager userManager;
User* connectedUsers[MAX_CLIENTS];
