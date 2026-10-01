#include <game/vpn_policy.h>
#include <sqlite3/sqlite3.h>
#undef min
#undef max
#include <algorithm>
#include <climits>
#include <cstring>
#include <set>
#include <utility>

namespace vpnblocker { namespace policy {
namespace {
constexpr size_t MAX_ENTRIES = 4096;
constexpr int POLICY_PAGE_SIZE = 8;
struct Rule { IpEntry entry; std::uint32_t first; std::uint32_t last; };
struct Snapshot { std::vector<Rule> ips; std::vector<GuidEntry> guids; };
sqlite3* database = nullptr;
std::string loadedPath;
Snapshot snapshot;
std::uint64_t generation = 0;

struct Statement {
    sqlite3_stmt* ptr = nullptr;
    Statement(sqlite3* db, const char* sql) { sqlite3_prepare_v2(db, sql, -1, &ptr, nullptr); }
    ~Statement() { if (ptr) sqlite3_finalize(ptr); }
};
bool fail(std::string& error, const char* text) { error = text; return false; }
const char* type(List list) { return list == List::Whitelist ? "WHITELIST" : "BLACKLIST"; }
bool validReason(const std::string& reason) {
    if (reason.empty() || reason.size() > 100) return false;
    bool nonSpace = false;
    for (unsigned char c : reason) {
        if (c < 32 || c == 127) return false;
        if (c != ' ') nonSpace = true;
    }
    return nonSpace;
}
bool parseV4(const std::string& text, std::uint32_t& output) {
    if (text.empty() || text.size() > 15) return false;
    std::uint32_t result = 0;
    size_t at = 0;
    for (int part = 0; part < 4; ++part) {
        const size_t begin = at;
        unsigned octet = 0;
        while (at < text.size() && text[at] >= '0' && text[at] <= '9') {
            octet = octet * 10 + static_cast<unsigned>(text[at++] - '0');
            if (octet > 255 || at - begin > 3) return false;
        }
        if (at == begin) return false;
        result = (result << 8) | octet;
        if (part < 3 && (at == text.size() || text[at++] != '.')) return false;
    }
    if (at != text.size()) return false;
    output = result;
    return true;
}
std::string formatV4(std::uint32_t ip) {
    return std::to_string(static_cast<int>(ip >> 24)) + "." +
        std::to_string(static_cast<int>((ip >> 16) & 255)) + "." +
        std::to_string(static_cast<int>((ip >> 8) & 255)) + "." +
        std::to_string(static_cast<int>(ip & 255));
}
sqlite3_int64 signedIp(std::uint32_t ip) {
    return ip <= 0x7fffffffU ? static_cast<sqlite3_int64>(ip)
        : static_cast<sqlite3_int64>(ip) - 0x100000000LL;
}
bool storedIp(sqlite3_stmt* row, int column, std::uint32_t& ip) {
    if (sqlite3_column_type(row, column) != SQLITE_INTEGER) return false;
    const sqlite3_int64 value = sqlite3_column_int64(row, column);
    if (value < -2147483648LL || value > 4294967295LL) return false;
    ip = static_cast<std::uint32_t>(value);
    return true;
}
std::string value(sqlite3_stmt* row, int column) {
    const auto* text = sqlite3_column_text(row, column);
    const int length = sqlite3_column_bytes(row, column);
    return text ? std::string(reinterpret_cast<const char*>(text), length) : std::string();
}
bool legacyGuid(const std::string& guid) {
    if (guid.size() != 32) return false;
    for (char c : guid)
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) return false;
    return true;
}
std::string upper(std::string text) {
    for (char& c : text) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    return text;
}
bool execute(sqlite3* db, const char* sql) { return sqlite3_exec(db, sql, nullptr, nullptr, nullptr) == SQLITE_OK; }
bool readSnapshot(sqlite3* db, Snapshot& output, std::string& error) {
    Snapshot next;
    Statement ips(db, "SELECT id,ip_1,ip_2,type,reason FROM ip_list ORDER BY id LIMIT 4097");
    if (!ips.ptr) return fail(error, "VPN database schema is invalid.");
    std::set<int> ids;
    std::set<std::uint32_t> starts;
    int result;
    while ((result = sqlite3_step(ips.ptr)) == SQLITE_ROW) {
        if (next.ips.size() >= MAX_ENTRIES) return fail(error, "VPN database exceeds the 4096 IP-rule limit.");
        Rule rule;
        const sqlite3_int64 id = sqlite3_column_int64(ips.ptr, 0);
        if (sqlite3_column_type(ips.ptr, 0) != SQLITE_INTEGER || id <= 0 || id > INT_MAX ||
            !storedIp(ips.ptr, 1, rule.first)) return fail(error, "VPN database contains an invalid IP rule.");
        rule.last = rule.first;
        if (sqlite3_column_type(ips.ptr, 2) != SQLITE_NULL && !storedIp(ips.ptr, 2, rule.last))
            return fail(error, "VPN database contains an invalid IP range.");
        if (rule.first > rule.last || !ids.insert(static_cast<int>(id)).second || !starts.insert(rule.first).second)
            return fail(error, "VPN database contains duplicate or reversed IP rules.");
        const std::string list = value(ips.ptr, 3);
        if (list != "WHITELIST" && list != "BLACKLIST") return fail(error, "VPN database contains an invalid list type.");
        rule.entry.id = static_cast<int>(id);
        rule.entry.list = list == "WHITELIST" ? List::Whitelist : List::Blacklist;
        rule.entry.first = formatV4(rule.first);
        rule.entry.last = formatV4(rule.last);
        rule.entry.reason = value(ips.ptr, 4);
        if (!validReason(rule.entry.reason)) return fail(error, "VPN database contains an invalid rule reason.");
        next.ips.push_back(rule);
    }
    if (result != SQLITE_DONE) return fail(error, "VPN database IP rules could not be read.");
    Statement guids(db, "SELECT nguid,reason FROM nguid_list ORDER BY nguid LIMIT 4097");
    if (!guids.ptr) return fail(error, "VPN database GUID schema is invalid.");
    while ((result = sqlite3_step(guids.ptr)) == SQLITE_ROW) {
        if (next.guids.size() >= MAX_ENTRIES) return fail(error, "VPN database exceeds the 4096 GUID-rule limit.");
        GuidEntry entry;
        entry.guid = value(guids.ptr, 0);
        entry.reason = value(guids.ptr, 1);
        entry.legacy = legacyGuid(entry.guid);
        if ((!entry.legacy && normalizeGuid(entry.guid).empty()) || !validReason(entry.reason))
            return fail(error, "VPN database contains an invalid GUID entry.");
        next.guids.push_back(entry);
    }
    if (result != SQLITE_DONE) return fail(error, "VPN database GUID rules could not be read.");
    output = std::move(next);
    return true;
}
bool tableExists(sqlite3* db, const char* name, bool& exists) {
    Statement row(db, "SELECT type FROM sqlite_master WHERE name=?1");
    if (!row.ptr) return false;
    sqlite3_bind_text(row.ptr, 1, name, -1, SQLITE_STATIC);
    const int result = sqlite3_step(row.ptr);
    exists = result == SQLITE_ROW;
    return result == SQLITE_DONE || (result == SQLITE_ROW && value(row.ptr, 0) == "table");
}
bool schema(sqlite3* db, std::string& error, bool create = false) {
    bool haveIps = false, haveGuids = false;
    if (!tableExists(db, "ip_list", haveIps) || !tableExists(db, "nguid_list", haveGuids))
        return fail(error, "VPN database table definitions are invalid.");
    if (!haveIps || !haveGuids) {
        Statement tables(db, "SELECT 1 FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' LIMIT 1");
        if (!create || haveIps || haveGuids || !tables.ptr || sqlite3_step(tables.ptr) != SQLITE_DONE)
            return fail(error, "VPN policy tables are missing or incomplete; existing data was retained.");
    }
    // Do not execute administrator-supplied triggers while editing rule tables.
    Statement triggers(db, "SELECT 1 FROM sqlite_master WHERE type='trigger' AND tbl_name IN ('ip_list','nguid_list') LIMIT 1");
    if (!triggers.ptr || sqlite3_step(triggers.ptr) != SQLITE_DONE)
        return fail(error, "VPN database rule tables must not have triggers.");
    if (!haveIps && !execute(db, "CREATE TABLE ip_list (id INTEGER NOT NULL PRIMARY KEY,ip_1 INTEGER NOT NULL UNIQUE,ip_2 INTEGER,type VARCHAR(9) CHECK(type IN ('WHITELIST','BLACKLIST')) NOT NULL,reason VARCHAR(100) NOT NULL)"))
        return fail(error, "VPN IP-rule table could not be created.");
    // SQLite VARCHAR is not length-limited: this retains old 32-character entries
    // and accepts native 40-hex GUIDs without rewriting the original table.
    if (!haveGuids && !execute(db, "CREATE TABLE nguid_list (nguid VARCHAR(40) PRIMARY KEY,reason VARCHAR(100) NOT NULL)"))
        return fail(error, "VPN GUID-rule table could not be created.");
    for (const char* query : {"PRAGMA table_info(ip_list)", "PRAGMA table_info(nguid_list)"}) {
        Statement columns(db, query);
        if (!columns.ptr) return fail(error, "VPN database schema cannot be inspected.");
        std::set<std::string> names;
        bool primaryKey = false;
        int result;
        while ((result = sqlite3_step(columns.ptr)) == SQLITE_ROW) {
            const std::string name = value(columns.ptr, 1);
            names.insert(name);
            if (sqlite3_column_int(columns.ptr, 5) == 1 &&
                ((!std::strcmp(query, "PRAGMA table_info(ip_list)") && name == "id") ||
                 (!std::strcmp(query, "PRAGMA table_info(nguid_list)") && name == "nguid"))) primaryKey = true;
        }
        const bool ipTable = !std::strcmp(query, "PRAGMA table_info(ip_list)");
        if (result != SQLITE_DONE || !primaryKey || !names.count("reason") ||
            (ipTable && (!names.count("id") || !names.count("ip_1") || !names.count("ip_2") || !names.count("type"))) ||
            (!ipTable && !names.count("nguid"))) return fail(error, "VPN database schema is incompatible; existing data was retained.");
    }
    return true;
}
bool writable(std::string& error) {
    error.clear();
    if (!database) return fail(error, "VPN policy database is unavailable or disabled.");
    return true;
}
bool begin(std::string& error) {
    if (!writable(error)) return false;
    if (!execute(database, "BEGIN IMMEDIATE")) return fail(error, "VPN database is busy or cannot be written; no changes saved.");
    if (!schema(database, error)) { execute(database, "ROLLBACK"); return false; }
    return true;
}
bool rollback(std::string& error, const char* message) {
    execute(database, "ROLLBACK");
    return fail(error, message);
}
bool commit(std::string& error) {
    Snapshot next;
    if (!schema(database, error) || !readSnapshot(database, next, error)) {
        execute(database, "ROLLBACK"); return false;
    }
    if (!execute(database, "COMMIT")) return rollback(error, "VPN database write failed; no changes saved.");
    snapshot = std::move(next); ++generation;
    return true;
}
bool pageValid(int page, int total, std::string& error) {
    return page >= 1 && page <= (total ? (total + POLICY_PAGE_SIZE - 1) / POLICY_PAGE_SIZE : 1)
        ? true : fail(error, "Invalid page number.");
}
}

std::string normalizeGuid(const std::string& guid) {
    if (guid.size() != 40) return "";
    std::string result = guid;
    for (char& c : result) {
        if (c >= 'A' && c <= 'F') c += 'a' - 'A';
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return "";
    }
    return result;
}
static bool openDatabase(const std::string& path, bool create, std::string& error) {
    error.clear();
    if (path.empty()) return fail(error, "VPN policy database path is empty.");
    sqlite3* candidate = nullptr;
    const int rc = sqlite3_open_v2(path.c_str(), &candidate,
        SQLITE_OPEN_READWRITE | (create ? SQLITE_OPEN_CREATE : 0) | SQLITE_OPEN_NOFOLLOW, nullptr);
    if (rc != SQLITE_OK) {
        if (candidate) sqlite3_close(candidate);
        return fail(error, "VPN policy database could not be opened; existing data was retained.");
    }
    sqlite3_busy_timeout(candidate, 25);
    sqlite3_db_config(candidate, SQLITE_DBCONFIG_DEFENSIVE, 1, nullptr);
    sqlite3_db_config(candidate, SQLITE_DBCONFIG_TRUSTED_SCHEMA, 0, nullptr);
    sqlite3_limit(candidate, SQLITE_LIMIT_LENGTH, 1024 * 1024);
    sqlite3_limit(candidate, SQLITE_LIMIT_SQL_LENGTH, 16384);
    Snapshot next;
    const bool ok = execute(candidate, "BEGIN IMMEDIATE") && schema(candidate, error, create) &&
        readSnapshot(candidate, next, error) && execute(candidate, "COMMIT");
    if (!ok) {
        execute(candidate, "ROLLBACK"); sqlite3_close(candidate);
        if (error.empty()) error = "VPN policy database is busy or cannot be loaded.";
        return false;
    }
    if (database) sqlite3_close(database);
    database = candidate; loadedPath = path; snapshot = std::move(next); ++generation;
    return true;
}
bool open(const std::string& path, std::string& error) { return openDatabase(path, true, error); }
void close() {
    if (database) sqlite3_close(database);
    database = nullptr; loadedPath.clear(); snapshot = Snapshot(); ++generation;
}
bool reload(std::string& error) {
    if (!writable(error)) return false;
    // Reopen the configured path so atomic file replacement on Unix is visible.
    // Validation succeeds before the previous connection and snapshot are retired.
    return openDatabase(loadedPath, false, error);
}
Counts counts() {
    Counts result; result.available = database != nullptr;
    for (const auto& rule : snapshot.ips) {
        if (rule.entry.list == List::Whitelist) ++result.whitelist;
        else ++result.blacklist;
    }
    for (const auto& entry : snapshot.guids) {
        if (entry.legacy) ++result.legacyGuids;
        else ++result.guids;
    }
    return result;
}
std::uint64_t revision() { return generation; }
Decision lookupIp(const std::string& ip) {
    std::uint32_t address;
    if (!parseV4(ip, address)) return Decision::None;
    Decision result = Decision::None;
    for (const auto& rule : snapshot.ips) {
        if (address < rule.first || address > rule.last) continue;
        if (rule.entry.list == List::Whitelist) return Decision::Whitelist;
        result = Decision::Blacklist;
    }
    return result;
}
bool containsGuid(const std::string& guid) {
    const std::string normalized = normalizeGuid(guid);
    if (normalized.empty()) return false;
    for (const auto& entry : snapshot.guids)
        if (!entry.legacy && normalizeGuid(entry.guid) == normalized) return true;
    return false;
}
bool listIps(List list, int page, std::vector<IpEntry>& entries, int& total, std::string& error) {
    entries.clear(); total = 0;
    if (!writable(error)) return false;
    for (const auto& rule : snapshot.ips) if (rule.entry.list == list) ++total;
    if (!pageValid(page, total, error)) return false;
    int index = 0;
    for (const auto& rule : snapshot.ips) {
        if (rule.entry.list != list) continue;
        if (index >= (page - 1) * POLICY_PAGE_SIZE && entries.size() < POLICY_PAGE_SIZE) entries.push_back(rule.entry);
        ++index;
    }
    return true;
}
bool addIp(List list, const std::string& first, const std::string& last,
    const std::string& reason, int& id, std::string& error) {
    id = 0;
    std::uint32_t start, end;
    if (!parseV4(first, start) || (!last.empty() && !parseV4(last, end))) return fail(error, "Use dotted IPv4 addresses without a port.");
    if (last.empty()) end = start;
    if (start > end) return fail(error, "The first IPv4 address must not exceed the last.");
    if (!validReason(reason)) return fail(error, "A reason of 1 to 100 printable bytes is required.");
    if (!begin(error)) return false;
    Statement insert(database, "INSERT INTO ip_list(ip_1,ip_2,type,reason) VALUES(?1,?2,?3,?4)");
    if (!insert.ptr) return rollback(error, "VPN IP rule could not be prepared.");
    sqlite3_bind_int64(insert.ptr, 1, signedIp(start));
    if (last.empty()) sqlite3_bind_null(insert.ptr, 2);
    else sqlite3_bind_int64(insert.ptr, 2, signedIp(end));
    sqlite3_bind_text(insert.ptr, 3, type(list), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert.ptr, 4, reason.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(insert.ptr) != SQLITE_DONE) return rollback(error, "VPN IP rule could not be saved; its starting address may already exist.");
    const sqlite3_int64 inserted = sqlite3_last_insert_rowid(database);
    if (inserted <= 0 || inserted > INT_MAX) return rollback(error, "VPN rule ID limit reached.");
    if (!commit(error)) return false;
    id = static_cast<int>(inserted); return true;
}
bool removeIp(List list, int id, std::string& error) {
    if (id <= 0) return fail(error, "A positive entry ID is required.");
    if (!begin(error)) return false;
    Statement remove(database, "DELETE FROM ip_list WHERE id=?1 AND type=?2");
    if (!remove.ptr) return rollback(error, "VPN IP rule could not be prepared.");
    sqlite3_bind_int(remove.ptr, 1, id);
    sqlite3_bind_text(remove.ptr, 2, type(list), -1, SQLITE_STATIC);
    if (sqlite3_step(remove.ptr) != SQLITE_DONE) return rollback(error, "VPN IP rule could not be removed.");
    if (sqlite3_changes(database) != 1) return rollback(error, "Entry ID was not found in this list.");
    return commit(error);
}
bool listGuids(int page, std::vector<GuidEntry>& entries, int& total, std::string& error) {
    entries.clear(); total = 0;
    if (!writable(error)) return false;
    total = static_cast<int>(snapshot.guids.size());
    if (!pageValid(page, total, error)) return false;
    for (int n = (page - 1) * POLICY_PAGE_SIZE; n < total && entries.size() < POLICY_PAGE_SIZE; ++n)
        entries.push_back(snapshot.guids[n]);
    return true;
}
bool addGuid(const std::string& guid, const std::string& reason, std::string& error) {
    const std::string normalized = normalizeGuid(guid);
    if (normalized.empty()) return fail(error, "Use an authenticated 40-hex Xmod GUID; Nitmod GUIDs cannot authorize Xmod players.");
    if (!validReason(reason)) return fail(error, "A reason of 1 to 100 printable bytes is required.");
    if (!begin(error)) return false;
    Statement insert(database, "INSERT INTO nguid_list(nguid,reason) SELECT ?1,?2 WHERE NOT EXISTS(SELECT 1 FROM nguid_list WHERE lower(nguid)=?1)");
    if (!insert.ptr) return rollback(error, "VPN GUID entry could not be prepared.");
    sqlite3_bind_text(insert.ptr, 1, normalized.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert.ptr, 2, reason.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(insert.ptr) != SQLITE_DONE || sqlite3_changes(database) != 1)
        return rollback(error, "VPN GUID entry could not be saved or already exists.");
    return commit(error);
}
bool removeGuid(const std::string& guid, std::string& error) {
    if (normalizeGuid(guid).empty() && !legacyGuid(guid)) return fail(error, "Use a complete native or retained legacy GUID.");
    if (!begin(error)) return false;
    Statement remove(database, "DELETE FROM nguid_list WHERE upper(nguid)=?1");
    if (!remove.ptr) return rollback(error, "VPN GUID entry could not be prepared.");
    const std::string normalized = upper(guid);
    sqlite3_bind_text(remove.ptr, 1, normalized.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(remove.ptr) != SQLITE_DONE || sqlite3_changes(database) < 1)
        return rollback(error, "VPN GUID entry was not found or could not be removed.");
    return commit(error);
}

} }
