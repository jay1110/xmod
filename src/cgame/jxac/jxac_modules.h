#ifndef CGAME_JXAC_MODULES_H
#define CGAME_JXAC_MODULES_H

namespace jxac {

// Discover loaded module files. Hashing is deferred to rendered frames.
void scanAndSendModules();

// Hash at most 256 KiB per frame, yielding sooner after four milliseconds.
// Call once per frame while module scanning is enabled.
void processModuleScan();

// Reset pending reports when reinitializing the client or disabling JXAC.
void clearModuleQueue();

// Send at most one ready report after the caller acquires bulk budget.
// The completion message uses a separate command slot.
bool processModuleQueue();

} // namespace jxac

#endif // CGAME_JXAC_MODULES_H
