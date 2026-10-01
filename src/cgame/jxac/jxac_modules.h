#ifndef CGAME_JXAC_MODULES_H
#define CGAME_JXAC_MODULES_H

namespace jxac {

// Scan loaded modules and queue for sending
void scanAndSendModules();

// Reset pending reports when reinitializing the client or disabling JXAC.
void clearModuleQueue();

// Send at most one reliable command, after the caller acquires bulk budget.
// The completion message uses a separate command slot.
bool processModuleQueue();

} // namespace jxac

#endif // CGAME_JXAC_MODULES_H
