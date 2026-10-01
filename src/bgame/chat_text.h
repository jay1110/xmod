#ifndef XMOD_CHAT_TEXT_H
#define XMOD_CHAT_TEXT_H

#include <string>

// Leave room for the command, closing quote, client/team fields and terminator.
// Unlike MAX_SAY_TEXT this is the complete wire envelope, including the prefix.
inline std::string XmodChatText(const std::string& input, size_t limit) {
    std::string text = input;
    for (char& c : text) {
        if (c == '"') c = '\'';
        else if (c == '\r' || c == '\n' || c == '\0') c = ' ';
    }
    if (text.size() > limit) {
        size_t cut = limit;
        // Do not split a UTF-8 character. Standalone ET high-byte glyphs remain
        // unchanged; only a complete, recognizable multibyte sequence counts.
        size_t start = cut;
        while (start > 0 && cut - start < 3 &&
               (static_cast<unsigned char>(text[start]) & 0xc0) == 0x80) --start;
        unsigned char lead = static_cast<unsigned char>(text[start]);
        size_t count = lead >= 0xc2 && lead <= 0xdf ? 2 :
                       lead >= 0xe0 && lead <= 0xef ? 3 :
                       lead >= 0xf0 && lead <= 0xf4 ? 4 : 1;
        if (start < cut && start + count > cut && start + count <= text.size()) {
            bool continuation = true;
            for (size_t i = start + 1; i < start + count; ++i)
                continuation &= (static_cast<unsigned char>(text[i]) & 0xc0) == 0x80;
            if (continuation) cut = start;
        }
        text.resize(cut);
    }
    return text;
}

inline std::string XmodChatCommand(const std::string& command, const std::string& prefix,
                                   const std::string& message, const std::string& suffix) {
    const std::string before = command + " \"";
    const std::string after = "\"" + suffix;
    if (before.size() + after.size() > 1022) return "";
    const size_t available = 1022 - before.size() - after.size();
    const std::string safePrefix = XmodChatText(prefix, available);
    return before + safePrefix + XmodChatText(message, available - safePrefix.size()) + after;
}

inline std::string XmodPrivateCommand(const std::string& from, const std::string& to,
                                      int recipients, const std::string& message, bool sound) {
    const std::string before = "pm \"" + XmodChatText(from, 63) + "\" \"" +
                              XmodChatText(to, 63) + "\" " + std::to_string(recipients) + " \"";
    const std::string after = sound ? "\" 1" : "\" 0";
    return before + XmodChatText(message, 1022 - before.size() - after.size()) + after;
}

#endif
