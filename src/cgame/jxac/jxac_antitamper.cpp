#include <bgame/impl.h>
#include <cgame/jxac/jxac_antitamper.h>

namespace jxac {
namespace {

// Allow scene setup to settle, as in Nitmod's renderer integrity check.
const unsigned RENDER_WARMUP_SUBMISSIONS = 501;
bool initialized = false;
bool reported = false;
unsigned renderSubmissions = 0;
const char* pendingViolation = NULL;

bool checksEnabled() {
    return initialized && cvars::bg_jxacEnabled.ivalue &&
        cvars::bg_jxacAntiTamper.ivalue && !cgs.initing && cg.snap &&
        !cg.demoPlayback && !cg.demoWallHack;
}

void clearDetections() {
    reported = false;
    renderSubmissions = 0;
    pendingViolation = NULL;
}

} // namespace

void AntiTamper::init() {
    if (!initialized) {
        clearDetections();
        initialized = true;
    }
}

void AntiTamper::shutdown() {
    initialized = false;
    clearDetections();
}

void AntiTamper::observeRenderSubmission(int before, int after) {
    if (!checksEnabled() || reported) {
        return;
    }
    if (renderSubmissions < RENDER_WARMUP_SUBMISSIONS) {
        ++renderSubmissions;
        return;
    }

    // Engine submission takes a const refEntity. Effects (including view
    // weapons with RF_DEPTHHACK) are already applied before this call.
    if (before != after) {
        reportTamper("render_flags_modified");
    }
}

void AntiTamper::observePlayerHead(int before, int after) {
    if (!checksEnabled() || reported) {
        return;
    }

    // Only CG_Player's world-space head uses this check, never view weapons,
    // HUD portraits or limbo previews. Fire/powerup drawing restores the
    // original entity; demo wallhack is excluded by checksEnabled().
    if ((before | after) & RF_DEPTHHACK) {
        reportTamper("player_head_depthhack");
    } else if (before != after) {
        reportTamper("player_head_flags_modified");
    }
}

void AntiTamper::reportTamper(const char* details) {
    if (checksEnabled() && !reported && !pendingViolation) {
        // Callers supply fixed internal reason strings. Do not send commands
        // from render callbacks: queue one report for the next client frame.
        pendingViolation = details;
    }
}

void AntiTamper::check() {
    if (!checksEnabled()) {
        clearDetections();
        return;
    }
    if (pendingViolation && !reported) {
        reported = true;
        trap_SendClientCommand(va("jxac_violation gamehack %s", pendingViolation));
        pendingViolation = NULL;
    }
}

} // namespace jxac
