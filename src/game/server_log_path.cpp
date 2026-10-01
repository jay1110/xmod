#include <bgame/impl.h>
#include <game/server_log_path.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

namespace serverlog {
namespace {
std::string slashes(std::string path) {
    for (char& c : path) if (c == '\\') c = '/';
    return path;
}
#ifdef _WIN32
bool letter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
#endif
bool absolute(const std::string& path) {
    if (!path.empty() && path[0] == '/') return true;
#ifdef _WIN32
    return path.size() >= 3 && letter(path[0]) && path[1] == ':' && path[2] == '/';
#else
    return false;
#endif
}
bool directory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) == 0) return (info.st_mode & S_IFDIR) != 0;
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}
bool parents(const std::string& path) {
    size_t start = path[0] == '/' ? 1 : 0;
#ifdef _WIN32
    if (path.size() >= 3 && path[1] == ':') start = 3;
    else if (path.compare(0, 2, "//") == 0) {
        const size_t server = path.find('/', 2);
        const size_t share = server == std::string::npos ? server : path.find('/', server + 1);
        if (share == std::string::npos) return false;
        start = share + 1; // UNC server/share already exists; create children only.
    }
#endif
    for (size_t slash = path.find('/', start); slash != std::string::npos; slash = path.find('/', slash + 1)) {
        if (slash && !directory(path.substr(0, slash))) return false;
    }
    return true;
}
bool component(const std::string& value) {
    if (value.empty() || value == "." || value == "..") return false;
    for (unsigned char c : value) if (c < 32 || c == 127 || c == '/' || c == '\\' || c == ':') return false;
    return true;
}
bool sameMod(const std::string& a, const std::string& b) {
#ifdef _WIN32
    return Q_stricmp(a.c_str(), b.c_str()) == 0;
#else
    return a == b;
#endif
}
}

bool resolve(const std::string& configured, std::string& physical) {
    physical.clear();
    if (configured.empty() || configured.size() > 4096) return false;
    std::string path = slashes(configured);
    for (unsigned char c : path) if (c < 32 || c == 127) return false;
    if (path.back() == '/') return false;
    if (!absolute(path)) {
        char homeBuffer[MAX_OSPATH] = {}, gameBuffer[MAX_QPATH] = {};
        trap_Cvar_VariableStringBuffer("fs_homepath", homeBuffer, sizeof(homeBuffer));
        trap_Cvar_VariableStringBuffer("fs_game", gameBuffer, sizeof(gameBuffer));
        const std::string game = gameBuffer[0] ? gameBuffer : "etmain";
        std::string home = slashes(homeBuffer);
        if (home.empty() || !component(game)) return false;
        while (home.size() > 1 && home.back() == '/') home.pop_back();
        std::vector<std::string> parts;
        for (size_t at = 0; at < path.size();) {
            const size_t slash = path.find('/', at);
            const std::string part = path.substr(at, slash == std::string::npos ? slash : slash - at);
            if (part != "." && !part.empty()) {
                if (!component(part)) return false;
                parts.push_back(part);
            }
            if (slash == std::string::npos) break;
            at = slash + 1;
        }
        // Older configs sometimes already include "xmod/". Do not append that
        // mod prefix twice when moving logs away from the process directory.
        if (parts.size() > 1 && sameMod(parts[0], game)) parts.erase(parts.begin());
        if (parts.empty()) return false;
        path = home + (home.back() == '/' ? "" : "/") + game;
        for (const auto& part : parts) path += "/" + part;
    }
    if (!parents(path)) return false;
    physical = path;
    return true;
}
}
