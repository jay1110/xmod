#ifndef GAME_JXAC_MD5_RULES_H
#define GAME_JXAC_MD5_RULES_H

#include <map>
#include <string>

namespace jxac {
namespace md5rules {

static const size_t MAX_FILE_BYTES = 1024 * 1024;
static const size_t MAX_RULES = 8192;
static const size_t MAX_MODULE_NAME = 127;

inline int hexValue(unsigned char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

inline bool normalizeHash(const std::string& source, size_t length, std::string& result) {
    if (source.size() != length) return false;
    result.clear();
    for (unsigned char c : source) {
        const int value = hexValue(c);
        if (value < 0) return false;
        result += "0123456789abcdef"[value];
    }
    return true;
}

inline std::string trim(const std::string& source) {
    const size_t first = source.find_first_not_of(" \t\r");
    if (first == std::string::npos) return "";
    return source.substr(first, source.find_last_not_of(" \t\r") - first + 1);
}

// Reports carry only a basename. Reject controls and path components before
// logging; replace command/color metacharacters only in human-readable output.
inline bool validBasename(const std::string& name) {
    if (name.empty() || name.size() > MAX_MODULE_NAME || name == "." || name == "..") return false;
    for (unsigned char c : name) if (c < 32 || c == 127 || c == '/' || c == '\\' || c == ':') return false;
    return true;
}

inline std::string safeText(const std::string& source, size_t limit = 255) {
    std::string result;
    for (unsigned char c : source) {
        if (result.size() == limit) break;
        result += c < 32 || c == 127 || c == '"' || c == '\\' || c == ';' || c == '^' ? '_' : c;
    }
    return result;
}

inline bool decodeBasename(const std::string& encoded, std::string& name) {
    if (encoded.empty() || encoded.size() > MAX_MODULE_NAME * 2 || encoded.size() % 2) return false;
    name.clear();
    for (size_t i = 0; i < encoded.size(); i += 2) {
        const int hi = hexValue(encoded[i]), lo = hexValue(encoded[i + 1]);
        if (hi < 0 || lo < 0) return false;
        name += static_cast<char>((hi << 4) | lo);
    }
    return validBasename(name);
}

typedef std::map<std::string, std::string> Rules;

// Parse transactionally: callers retain their active list on any failure.
inline bool parse(const std::string& contents, Rules& result, size_t& errorLine, std::string& error) {
    Rules parsed;
    errorLine = 0;
    if (contents.size() > MAX_FILE_BYTES) { error = "file exceeds 1 MiB"; return false; }
    size_t at = contents.compare(0, 3, "\xef\xbb\xbf") == 0 ? 3 : 0;
    size_t lineNumber = 0;
    while (at < contents.size()) {
        ++lineNumber;
        const size_t end = contents.find('\n', at);
        std::string line = contents.substr(at, end == std::string::npos ? end : end - at);
        at = end == std::string::npos ? contents.size() : end + 1;
        errorLine = lineNumber;
        if (line.size() > 512) { error = "line exceeds 512 bytes"; return false; }
        for (unsigned char c : line) {
            if ((c < 32 && c != '\r' && c != '\t') || c == 127) {
                error = "invalid control character or incomplete read"; return false;
            }
        }
        const size_t hashComment = line.find('#'), slashComment = line.find("//");
        const size_t comment = hashComment < slashComment ? hashComment : slashComment;
        if (comment != std::string::npos) line.resize(comment);
        line = trim(line);
        if (line.empty()) continue;
        const size_t space = line.find_first_of(" \t\r");
        std::string digest;
        if (!normalizeHash(line.substr(0, space), 32, digest)) {
            error = "expected a 32-digit hexadecimal MD5 followed by an optional label";
            return false;
        }
        const std::string label = space == std::string::npos ? "" : trim(line.substr(space));
        if (label.size() > 127) { error = "label exceeds 127 bytes"; return false; }
        parsed.insert(std::make_pair(digest, safeText(label, 127)));
        if (parsed.size() > MAX_RULES) { error = "more than 8192 distinct hashes"; return false; }
    }
    result.swap(parsed);
    errorLine = 0;
    error.clear();
    return true;
}

} // namespace md5rules
} // namespace jxac
#endif
