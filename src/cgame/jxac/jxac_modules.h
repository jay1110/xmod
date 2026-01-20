#ifndef CGAME_JXAC_MODULES_H
#define CGAME_JXAC_MODULES_H

namespace jxac {

// Scan loaded modules and queue for sending
void scanAndSendModules();

// Process module queue (call each frame)
void processModuleQueue();

} // namespace jxac

#endif // CGAME_JXAC_MODULES_H
