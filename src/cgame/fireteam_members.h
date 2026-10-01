#ifndef CGAME_FIRETEAM_MEMBERS_H
#define CGAME_FIRETEAM_MEMBERS_H

#include <cstring>

// NCS may not have arrived yet after cgame restarts. Never read beyond a
// partial membership string or leave bits from an earlier parse behind.
inline bool CG_DecodeFireteamMembers(const char* text, int members[2]) {
    members[0] = members[1] = 0;
    if (!text || strlen(text) != 16) return false;
    unsigned int words[2] = {0, 0};
    for (int i = 0; i < 16; ++i) {
        unsigned int digit;
        const char c = text[i];
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else return false;
        const int word = i < 8 ? 1 : 0;
        words[word] = (words[word] << 4) | digit;
    }
    memcpy(members, words, sizeof(words));
    return true;
}
#endif
