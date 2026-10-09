#ifndef CGAME_JXAC_ANTITAMPER_H
#define CGAME_JXAC_ANTITAMPER_H

namespace jxac {

// Renderer integrity checks reported only to the connected game server.
class AntiTamper {
public:
    static void init();
    static void shutdown();
    static void check();
    static void observeRenderSubmission(int before, int after);
    static void observePlayerHead(int before, int after);

private:
    static void reportTamper(const char* details);
};

} // namespace jxac

#endif // CGAME_JXAC_ANTITAMPER_H
