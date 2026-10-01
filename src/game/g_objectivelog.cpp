#include <bgame/impl.h>

// Keep each event on one line and preserve unambiguous quoted objective names.
const char* G_LogSafeText(const char* text)
{
    static char buffers[4][MAX_STRING_CHARS];
    static unsigned index;
    char* result = buffers[index++ % 4];
    if (!text || !*text) text = "unknown";
    size_t used = 0;
    for (; *text && used + 1 < sizeof(buffers[0]); ++text) {
        unsigned char c = static_cast<unsigned char>(*text);
        result[used++] = c < 32 || c == 127 ? ' ' : c == '"' ? '\'' : c;
    }
    result[used] = '\0';
    return result;
}

const char* G_ObjectiveLogName(const gentity_t* ent)
{
    if (!ent) return G_LogSafeText(NULL);
    // track is the visible objective label in map scripts.
    if (ent->track && *ent->track) return G_LogSafeText(ent->track);
    if (ent->parent && ent->parent->track && *ent->parent->track)
        return G_LogSafeText(ent->parent->track);
    if (ent->message && *ent->message) return G_LogSafeText(ent->message);
    if (ent->parent && ent->parent->message && *ent->parent->message)
        return G_LogSafeText(ent->parent->message);
    if (ent->scriptName && *ent->scriptName) return G_LogSafeText(ent->scriptName);
    return G_LogSafeText(ent->targetname);
}
