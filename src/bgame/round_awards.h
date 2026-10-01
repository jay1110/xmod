#ifndef XMOD_ROUND_AWARDS_H
#define XMOD_ROUND_AWARDS_H

// Wire order is independent of the legacy fourteen ET awards.
enum { XMOD_ROUND_AWARDS = 12 };
struct roundAwardCounters_t {
    int bestSpree, reviveSpree, bestReviveSpree;
    int heals, engineerObjectives, uniforms, ammo;
};
static const char* const roundAwardTitles[XMOD_ROUND_AWARDS] = {
    "Highest Fragger", "Highest XP", "Best Spree", "Most Headshots",
    "Highest damage", "Highest Accuracy", "Most Revives", "Most Heals",
    "Best Revive Spree", "Best Engineer", "Best CovertOps", "Best FieldOps"
};
static const int roundAwardIconIds[XMOD_ROUND_AWARDS] = {0,1,3,4,5,7,9,10,11,13,14,15};
#include <cstring>
#include <cstdlib>
struct roundAwardDisplay_t { char name[64]; int team; };
inline bool ParseRoundAwards(const char* text, roundAwardDisplay_t (&out)[XMOD_ROUND_AWARDS]) {
    std::memset(out, 0, sizeof(out));
    roundAwardDisplay_t parsed[XMOD_ROUND_AWARDS] = {};
    if (!text) return false;
    for (int i = 0; i < XMOD_ROUND_AWARDS; ++i) {
        while (*text == ' ') ++text;
        if (*text++ != ';') return false;
        const char* end = std::strchr(text, ';');
        if (!end || end - text >= (int)sizeof(parsed[i].name)) return false;
        std::memcpy(parsed[i].name, text, end - text);
        text = end + 1;
        while (*text == ' ') ++text;
        if (*text < '0' || *text > '2') return false;
        parsed[i].team = *text++ - '0';
        if (*text && *text != ' ') return false;
    }
    while (*text == ' ') ++text;
    if (*text) return false;
    std::memcpy(out, parsed, sizeof(out));
    return true;
}
#endif
