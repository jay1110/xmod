#ifndef XMOD_RELIABLE_BUDGET_H
#define XMOD_RELIABLE_BUDGET_H

#include <cstdint>

// Bulk anti-cheat reports share the engine's reliable client command queue
// with chat and gameplay commands. Render FPS must not determine their rate.
class XmodReliableBudget {
    uint32_t lastTick = 0;
    unsigned remaining = 0;
    bool started = false;
public:
    enum { INTERVAL_MS = 125, COMMANDS_PER_INTERVAL = 2 };

    void reset() { lastTick = 0; remaining = 0; started = false; }

    bool take(uint32_t now) {
        const uint32_t elapsed = now - lastTick;
        if (!started || elapsed >= INTERVAL_MS) {
            // Unsigned subtraction handles clock wrap. A backwards clock jump
            // resets the window as well; neither case accumulates old credits.
            lastTick = now;
            remaining = COMMANDS_PER_INTERVAL;
            started = true;
        }
        if (!remaining) return false;
        --remaining;
        return true;
    }
};

#endif
