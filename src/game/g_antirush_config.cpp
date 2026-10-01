#include <bgame/impl.h>
#include <bgame/numeric_text.h>
#include <game/g_antirush.h>
#undef min
#undef max
#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <locale>
#include <set>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

namespace antirush {
State state;
namespace {
const char* CONFIG_PATH = "antirush/antirush.cfg";
const char* GUID_PATH = "antirush/guids.cfg";
const char* MODE_PATH = "antirush/settings.cache";
const size_t MAX_FILE_SIZE = 2 * 1024 * 1024;
const size_t MAX_ENTRIES = 4096;
const double MAX_RADIUS = 100000;
const double MAX_COORDINATE = 10000000;
bool pointsWritable = true;
bool guidsWritable = true;
bool modeWritable = true;

std::string trim(const std::string& value) {
    const size_t begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return "";
    return value.substr(begin, value.find_last_not_of(" \t\r\n") - begin + 1);
}
std::string lower(std::string value) {
    for (char& c : value) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return value;
}
bool asciiLetter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
bool asciiDigit(char c) { return c >= '0' && c <= '9'; }
bool tokenCharacter(char c) { return asciiLetter(c) || asciiDigit(c) || c == '_' || c == '-'; }
bool validText(const std::string& text, size_t limit) {
    if (text.size() > limit) return false;
    for (unsigned char c : text) if (c < 32 || c == 127) return false;
    return true;
}
bool relativePath(const std::string& path) {
    if (path.empty() || path.size() > 200 || path.find("..") != std::string::npos
        || path.front() == '/' || path.back() == '/') return false;
    for (char c : path) if (!tokenCharacter(c) && c != '.' && c != '/' && c != '+') return false;
    return true;
}
std::string mapName(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (result.size() == 100) break;
        result += tokenCharacter(c) || c == '+' ? c : '_';
    }
    return result.empty() ? "unknownmap" : result;
}
bool number(const std::string& token, double& value, double minimum, double maximum) {
    if (token.empty() || token.size() > 100) return false;
    return XmodParseFiniteDecimal(token.c_str(), value) && value >= minimum && value <= maximum;
}
std::string homeDirectory() {
    char home[MAX_OSPATH] = {}, game[MAX_QPATH] = {};
    trap_Cvar_VariableStringBuffer("fs_homepath", home, sizeof(home));
    trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
    std::string name = game[0] ? game : "etmain";
    if (!home[0] || name == "." || name == "..") return "";
    for (char c : name) if (!tokenCharacter(c) && c != '.') return "";
    return std::string(home) + "/" + name;
}
std::string physicalPath(const std::string& path) {
    const std::string home = homeDirectory();
    return home.empty() || !relativePath(path) ? "" : home + "/" + path;
}
bool makeDirectory(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return (st.st_mode & S_IFDIR) != 0;
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}
bool parentDirectories(const std::string& relative) {
    std::string path = homeDirectory();
    if (path.empty() || !relativePath(relative) || !makeDirectory(path)) return false;
    size_t begin = 0, slash;
    while ((slash = relative.find('/', begin)) != std::string::npos) {
        path += "/" + relative.substr(begin, slash - begin);
        if (!makeDirectory(path)) return false;
        begin = slash + 1;
    }
    return true;
}
// 1: read, -1: absent, 0: unreadable. Never hide an unreadable local override
// by loading an older packaged version of it.
int readNative(const std::string& path, std::string& data) {
    errno = 0;
    FILE* file = std::fopen(path.c_str(), "rb");
    if (!file) return errno == ENOENT ? -1 : 0;
    bool ok = std::fseek(file, 0, SEEK_END) == 0;
    const long size = ok ? std::ftell(file) : -1;
    ok = size >= 0 && static_cast<unsigned long>(size) <= MAX_FILE_SIZE
        && std::fseek(file, 0, SEEK_SET) == 0;
    data.clear();
    if (ok) {
        data.resize(static_cast<size_t>(size));
        if (size) ok = std::fread(&data[0], 1, size, file) == static_cast<size_t>(size);
    }
    if (std::fclose(file) != 0) ok = false;
    return ok ? 1 : 0;
}
bool read(const std::string& path, std::string& data, bool* writable = nullptr) {
    const std::string physical = physicalPath(path);
    const int local = physical.empty() ? -1 : readNative(physical, data);
    if (local != -1) {
        if (!local && writable) *writable = false;
        if (!local) G_Printf("AntiRush: cannot read local %s; packaged data not substituted.\n", path.c_str());
        return local == 1;
    }
    fileHandle_t file = 0;
    const int length = trap_FS_FOpenFile(path.c_str(), &file, FS_READ);
    if (!file || length < 0) return false;
    if (static_cast<size_t>(length) > MAX_FILE_SIZE) {
        if (writable) *writable = false;
        trap_FS_FCloseFile(file);
        G_Printf("AntiRush: %s exceeds the configuration size limit.\n", path.c_str());
        return false;
    }
    data.resize(length);
    if (length) trap_FS_Read(&data[0], length, file);
    trap_FS_FCloseFile(file);
    return true;
}
bool replaceFile(const std::string& from, const std::string& to) {
#ifdef _WIN32
    return MoveFileExA(from.c_str(), to.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(from.c_str(), to.c_str()) == 0;
#endif
}
bool writeTemporary(const std::string& path, const std::string& data) {
    FILE* file = std::fopen(path.c_str(), "wb");
    if (!file) return false;
    bool ok = std::fwrite(data.data(), 1, data.size(), file) == data.size()
        && std::fflush(file) == 0;
#ifdef _WIN32
    if (ok) ok = FlushFileBuffers(reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)))) != 0;
#else
    if (ok) ok = fsync(fileno(file)) == 0;
#endif
    if (std::fclose(file) != 0) ok = false;
    if (!ok) std::remove(path.c_str());
    return ok;
}
bool write(const std::string& relative, const std::string& data) {
    const std::string path = physicalPath(relative);
    if (path.empty() || data.size() > MAX_FILE_SIZE || !parentDirectories(relative)) return false;
    std::string previous;
    const int old = readNative(path, previous);
    if (old == 0) return false;
    const std::string temporary = path + ".tmp";
    if (!writeTemporary(temporary, data)) return false;
    if (old == 1) {
        const std::string backupTemporary = path + ".bak.tmp";
        if (!writeTemporary(backupTemporary, previous) || !replaceFile(backupTemporary, path + ".bak")) {
            std::remove(temporary.c_str());
            std::remove(backupTemporary.c_str());
            return false;
        }
    }
    if (!replaceFile(temporary, path)) {
        std::remove(temporary.c_str());
        return false;
    }
    return true;
}
std::string withoutBom(std::string value) {
    if (value.compare(0, 3, "\xef\xbb\xbf") == 0) value.erase(0, 3);
    return value;
}
bool comment(const std::string& value) {
    return value.empty() || value[0] == '#' || value[0] == ';' || value.compare(0, 2, "//") == 0;
}
void messageDefaults(Settings& settings) {
    settings.messages["START"] = {"chat", "^3AUTOADMIN:^7 Rushing the following objectives is prohibited for the first ^3{duration}^7: ^3{objectives}"};
    settings.messages["END"] = {"chat", "^3AUTOADMIN:^7 The following objectives are no longer protected: ^3{objectives}"};
    settings.messages["COUNTDOWN"] = {"cpm", "^3Antirush expires in {remaining}^7"};
    settings.messages["RUSH"] = {"chat", "^3AUTOADMIN:^7 {player} ^7was killed for rushing protected objective ^3{objective}^7."};
    settings.messages["BOT"] = {"chat", "^3AUTOADMIN:^7 Omni-bot {player} ^7is restricted from moving with protected objective ^3{objective}^7 until anti-rush protection expires."};
    settings.messages["TRICKPLANT"] = {"chat", "^3AUTOADMIN:^7 Dynamite planted by {player} ^7was removed due to trickplant protection."};
    settings.messages["TRICKPLANT_UNKNOWN"] = {"chat", "^3AUTOADMIN:^7 An armed dynamite was removed due to trickplant protection."};
    settings.messages["OBJECTIVE_SAVED"] = {"chat", "^3AUTOADMIN:^7 Objective ^3{objective} ^7has been saved for antirush protection."};
}
bool applySetting(Settings& settings, const std::string& key, std::string value) {
    if (!value.empty() && value[0] == '"') {
        if (value.size() < 2 || value.back() != '"') return false;
        value = value.substr(1, value.size() - 2);
    }
    if (key == "ANTIRUSH_LOG_FILE") {
        for (char& c : value) if (c == '\\') c = '/';
        if (!value.empty() && (!relativePath(value) || value.size() < 4 || value.substr(value.size() - 4) != ".log")) return false;
        settings.logPath = value; return true;
    }
    if (key == "MESSAGES_ENABLED") {
        value = lower(value);
        if (value != "true" && value != "false") return false;
        settings.messagesEnabled = value == "true"; return true;
    }
    for (auto& message : settings.messages) {
        if (key == "MSG_" + message.first + "_TEXT") {
            if (!validText(value, 800)) return false;
            message.second.text = value; return true;
        }
        if (key == "MSG_" + message.first + "_CHANNEL") {
            value = lower(value);
            if (value != "chat" && value != "cpm" && value != "cp" && value != "print" && value != "off") return false;
            message.second.channel = value; return true;
        }
    }
    double n;
    if (key == "ADMIN_LEVEL" || key == "USER_LEVEL") {
        if (!number(value, n, key == "ADMIN_LEVEL" ? 1 : -1, 2147483647) || n != static_cast<int>(n)) return false;
        (key == "ADMIN_LEVEL" ? settings.adminLevel : settings.userLevel) = static_cast<int>(n); return true;
    }
    if (key == "PROTECT_PERCENT") {
        const size_t slash = value.find('/');
        if (slash != std::string::npos) {
            double numerator, denominator;
            if (!number(trim(value.substr(0, slash)), numerator, -1e100, 1e100)
                || !number(trim(value.substr(slash + 1)), denominator, -1e100, 1e100) || denominator == 0) return false;
            n = numerator / denominator;
            if (!(n >= 0 && n <= 1)) return false;
        } else if (!number(value, n, 0, 1)) return false;
        settings.protectFraction = n; return true;
    }
    if (key == "ANTIRUSH_SCAN_RANGE" || key == "TRICKPLANT_SCAN_RANGE") {
        if (!number(value, n, 1, MAX_RADIUS)) return false;
        (key == "ANTIRUSH_SCAN_RANGE" ? settings.antirushRange : settings.trickplantRange) = static_cast<float>(n); return true;
    }
    if (key == "COVERT_DRAIN_RANGE_MULTIPLIER") {
        if (!number(value, n, 0.1, 100)) return false;
        settings.legacyDrainMultiplier = static_cast<float>(n); return true;
    }
    if (key == "ANTIRUSH_COUNTDOWN_INTERVAL_SECONDS" || key == "START_MESSAGE_DELAY_SECONDS") {
        if (!number(value, n, 0, 3600) || (key == "ANTIRUSH_COUNTDOWN_INTERVAL_SECONDS" && n > 0 && n < 1)) return false;
        (key == "ANTIRUSH_COUNTDOWN_INTERVAL_SECONDS" ? settings.countdownMs : settings.startMessageDelayMs) = static_cast<int>(n * 1000.0); return true;
    }
    return false;
}
void loadSettings() {
    messageDefaults(state.settings);
    std::string data;
    if (!read(CONFIG_PATH, data)) return;
    std::istringstream stream(withoutBom(data));
    std::set<std::string> seen;
    std::string line;
    int lineNumber = 0;
    while (std::getline(stream, line)) {
        ++lineNumber; line = trim(line);
        if (comment(line)) continue;
        const size_t equals = line.find('=');
        const std::string key = equals == std::string::npos ? "" : trim(line.substr(0, equals));
        if (line.size() > 2048 || key.empty() || !seen.insert(key).second
            || !applySetting(state.settings, key, trim(line.substr(equals + 1)))) {
            G_Printf("AntiRush: invalid or duplicate setting at %s:%d; ignored.\n", CONFIG_PATH, lineNumber);
        }
    }
}
bool parsePoint(const std::string& original, bool trickplant, Point& point) {
    point = Point(); point.trickplant = trickplant;
    std::string line = original;
    if (!trickplant) {
        if (line.empty() || line[0] != '"') return false;
        const size_t end = line.find('"', 1);
        if (end == std::string::npos || end + 1 >= line.size()
            || (line[end + 1] != ' ' && line[end + 1] != '\t')
            || !validText(line.substr(1, end - 1), 256)) return false;
        point.name = sanitizeText(line.substr(1, end - 1));
        line = line.substr(end + 1);
    }
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) { tokens.push_back(token); if (tokens.size() > 6) return false; }
    if (tokens.size() < 4 || (trickplant && tokens.size() != 4)) return false;
    double value;
    for (int i = 0; i < 3; ++i) {
        if (!number(tokens[i], value, -MAX_COORDINATE, MAX_COORDINATE)) return false;
        point.origin[i] = static_cast<float>(value);
    }
    if (!number(tokens[3], value, 0.001, MAX_RADIUS)) return false;
    point.radius = static_cast<float>(value);
    if (tokens.size() == 5) {
        token = lower(tokens[4]);
        if (token != "true" && token != "false") return false;
        if (token == "true") {
            value = static_cast<double>(point.radius) * state.settings.legacyDrainMultiplier;
            if (value > MAX_RADIUS) return false;
            point.covertRadius = point.engineerRadius = static_cast<float>(value);
        }
    } else if (tokens.size() == 6) {
        if (!number(tokens[4], value, 0, MAX_RADIUS)) return false;
        point.covertRadius = static_cast<float>(value);
        if (!number(tokens[5], value, 0, MAX_RADIUS)) return false;
        point.engineerRadius = static_cast<float>(value);
    }
    return true;
}
void loadPoints() {
    const std::string path = "antirush/maps/" + state.mapName + ".cfg";
    std::string data;
    if (!read(path, data, &pointsWritable)) return;
    std::istringstream stream(withoutBom(data));
    std::string line, section;
    int lineNumber = 0;
    while (std::getline(stream, line)) {
        ++lineNumber; line = trim(line);
        if (comment(line)) continue;
        if (line[0] == '[' && line.back() == ']') { section = lower(trim(line.substr(1, line.size() - 2))); continue; }
        Point point;
        if (line.size() <= 2048 && state.points.size() < MAX_ENTRIES
            && (section == "antirush" || section == "trickplant")
            && parsePoint(line, section == "trickplant", point)) state.points.push_back(point);
        else {
            pointsWritable = false;
            G_Printf("AntiRush: invalid point at %s:%d; ignored. Editing disabled until the file is repaired and reloaded.\n", path.c_str(), lineNumber);
        }
    }
    std::stable_partition(state.points.begin(), state.points.end(), [](const Point& point) { return !point.trickplant; });
}
void loadGuids() {
    std::string data;
    if (!read(GUID_PATH, data, &guidsWritable)) return;
    std::istringstream stream(withoutBom(data));
    std::set<std::string> seen;
    std::string line;
    int rejected = 0;
    while (std::getline(stream, line)) {
        line = trim(line);
        if (comment(line)) continue;
        const size_t space = line.find_first_of(" \t");
        const std::string guid = normalizeGuid(line.substr(0, space));
        const std::string name = space == std::string::npos ? "" : trim(line.substr(space));
        if (guid.empty() || !validText(name, 256) || state.guids.size() >= MAX_ENTRIES) { ++rejected; guidsWritable = false; continue; }
        if (seen.insert(guid).second) state.guids.push_back({guid, sanitizeText(name)});
    }
    if (rejected) G_Printf("AntiRush: ignored %d invalid GUID entries; authenticated 40-character Xmod GUIDs are required. Editing disabled until the file is repaired and reloaded.\n", rejected);
}
}

std::string sanitizeText(const std::string& text) {
    std::string result;
    for (unsigned char c : text) {
        if (c < 32 || c == 127) result += ' ';
        else if (c == '"') result += '\'';
        else if (c == '\\') result += '/';
        else result += static_cast<char>(c);
    }
    return result;
}
std::string normalizeGuid(const std::string& guid) {
    if (guid.size() != 40) return "";
    const std::string result = lower(guid);
    for (char c : result) if (!asciiDigit(c) && !(c >= 'a' && c <= 'f')) return "";
    return result;
}
void load() {
    state = State();
    pointsWritable = guidsWritable = modeWritable = true;
    state.mapName = mapName(level.rawmapname);
    loadSettings();
    loadPoints();
    loadGuids();
    std::string data;
    if (read(MODE_PATH, data, &modeWritable)) {
        std::istringstream stream(withoutBom(data));
        std::string line;
        bool found = false;
        while (std::getline(stream, line)) {
            line = trim(line);
            if (comment(line)) continue;
            const size_t equals = line.find('=');
            if (!found && equals != std::string::npos && trim(line.substr(0, equals)) == "antirush_mode") {
                found = true;
                const std::string value = lower(trim(line.substr(equals + 1)));
                if (value == "on" || value == "off") state.mode = value == "on";
                else modeWritable = false;
            } else modeWritable = false;
        }
        if (!found) modeWritable = false;
        if (!modeWritable) G_Printf("AntiRush: invalid mode cache; editing disabled until the file is repaired and reloaded.\n");
    }
}
bool savePoints(const std::vector<Point>& replacement) {
    if (!pointsWritable || replacement.size() > MAX_ENTRIES) return false;
    std::vector<Point> canonical;
    std::ostringstream data;
    data.imbue(std::locale::classic());
    data << std::fixed << std::setprecision(3);
    for (int section = 0; section < 2; ++section) {
        data << (section ? "\n[trickplant]\n" : "[antirush]\n");
        for (const Point& point : replacement) {
            if (point.trickplant != (section != 0)) continue;
            std::ostringstream row;
            row.imbue(std::locale::classic());
            row << std::fixed << std::setprecision(3);
            if (!point.trickplant) row << '"' << sanitizeText(point.name) << "\" ";
            row << point.origin[0] << ' ' << point.origin[1] << ' ' << point.origin[2] << ' ' << point.radius;
            if (!point.trickplant) row << ' ' << point.covertRadius << ' ' << point.engineerRadius;
            Point checked;
            if (!parsePoint(row.str(), point.trickplant, checked)) return false;
            canonical.push_back(checked);
            data << row.str() << '\n';
        }
    }
    if (!write("antirush/maps/" + state.mapName + ".cfg", data.str())) return false;
    state.points = canonical;
    return true;
}
bool saveGuids(const std::vector<GuidEntry>& replacement) {
    if (!guidsWritable || replacement.size() > MAX_ENTRIES) return false;
    std::vector<GuidEntry> canonical;
    std::set<std::string> seen;
    std::string data = "# Authenticated Xmod GUIDs. N!tmod GUIDs are not interchangeable.\n";
    for (const GuidEntry& entry : replacement) {
        const std::string guid = normalizeGuid(entry.guid);
        if (guid.empty() || !validText(entry.name, 256) || !seen.insert(guid).second) return false;
        const std::string name = sanitizeText(entry.name);
        canonical.push_back({guid, name});
        data += guid + (name.empty() ? "" : " " + name) + "\n";
    }
    if (!write(GUID_PATH, data)) return false;
    state.guids = canonical;
    return true;
}
bool saveMode(bool enabled) {
    if (!modeWritable) return false;
    if (!write(MODE_PATH, std::string("# Managed by Xmod AntiRush. Use !antirush on/off.\nantirush_mode = ") + (enabled ? "on\n" : "off\n"))) return false;
    state.mode = enabled;
    return true;
}
std::vector<std::string> configuredMaps() {
    char list[65536] = {};
    const int count = trap_FS_GetFileList("antirush/maps", ".cfg", list, sizeof(list));
    std::set<std::string> seen;
    size_t offset = 0;
    for (int i = 0; i < count && offset < sizeof(list); ++i) {
        const char* end = static_cast<const char*>(std::memchr(list + offset, 0, sizeof(list) - offset));
        if (!end) break;
        const std::string file(list + offset, static_cast<size_t>(end - (list + offset)));
        offset += file.size() + 1;
        if (file.size() <= 4 || lower(file.substr(file.size() - 4)) != ".cfg") continue;
        const std::string name = file.substr(0, file.size() - 4);
        if (mapName(name) == name) seen.insert(lower(name));
    }
    return std::vector<std::string>(seen.begin(), seen.end());
}
void logChange(int clientNum, const std::string& command, const std::string& detail) {
    const std::string& relative = state.settings.logPath;
    if (relative.empty() || !relativePath(relative) || !parentDirectories(relative)) return;
    const std::string path = physicalPath(relative);
    if (path.empty()) return;
    std::string guid, name = "console";
    int adminLevel = 0;
    if (clientNum >= 0 && clientNum < MAX_CLIENTS) {
        authenticated(clientNum, &guid, &adminLevel);
        name = g_clients[clientNum].pers.netname;
    }
    char timestamp[32] = {};
    const std::time_t now = std::time(nullptr);
    const std::tm* utc = std::gmtime(&now);
    if (utc) std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", utc);
    std::ostringstream message;
    message << timestamp << " [map=" << state.mapName << "][levelTime=" << level.time << "] CHANGE slot=" << clientNum
        << " name=\"" << sanitizeText(name) << "\" guid=\"" << guid << "\" level=" << adminLevel
        << " command=\"" << sanitizeText(command.substr(0, 1024)) << "\" " << sanitizeText(detail.substr(0, 2048)) << '\n';
    const std::string data = message.str();
    FILE* file = std::fopen(path.c_str(), "ab");
    bool ok = file != nullptr;
    if (file) {
        ok = std::fwrite(data.data(), 1, data.size(), file) == data.size();
        if (std::fclose(file) != 0) ok = false;
    }
    if (!ok) G_Printf("AntiRush: cannot append admin change log %s.\n", relative.c_str());
}
}
