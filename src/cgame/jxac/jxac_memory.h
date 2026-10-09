#ifndef CGAME_JXAC_MEMORY_H
#define CGAME_JXAC_MEMORY_H

namespace jxac {

// Incremental inspection of this game process only. Other platforms report
// unsupported; an unknown allocation or hook is never itself a violation.
void processMemoryScan();
void clearMemoryScan();

// Send at most one status/detection after the shared reliable budget grants it.
bool processMemoryQueue();
// Confirmed detections take priority over screenshot/status uploads, still
// using the caller's shared reliable-command budget.
bool processMemoryHitQueue();

} // namespace jxac

#endif // CGAME_JXAC_MEMORY_H
