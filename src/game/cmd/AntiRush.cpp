#include <bgame/impl.h>
#include <game/g_antirush.h>

namespace cmd {
AntiRush::AntiRush() : AbstractBuiltin("antirush") {
    __usage << xvalue("!antirush [status|on|off]");
    __descr << "Show native Antirush status or persist its on/off switch. On/off selects g_antirush 2. "
        "C/antirush also permits !aa GUID management when C/aa access is available.";
}
bool AntiRush::hasPermission(const Context& txt) {
    return antirush::canManage(txt._client ? txt._client->slot : -1);
}
AbstractCommand::PostAction AntiRush::doExecute(Context& txt) {
    antirush::runCommand(txt._client ? txt._client->slot : -1,
        Args(txt._args.begin() + 1, txt._args.end()), true);
    return PA_NONE;
}
}
