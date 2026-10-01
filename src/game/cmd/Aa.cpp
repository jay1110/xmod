#include <bgame/impl.h>
#include <game/g_antirush.h>

namespace cmd {
Aa::Aa() : AbstractBuiltin("aa") {
    __usage << xvalue("!aa help|info|list|distance")
        << '\n' << xvalue("!aa antirush|trickplant [ENTITY_ID]")
        << '\n' << xvalue("!aa edit POINT_ID RADIUS|NAME|c RADIUS|e RADIUS")
        << '\n' << xvalue("!aa remove POINT_ID")
        << '\n' << xvalue("!aa addguid PLAYER_SLOT | removeguid [LIST_ID]");
    __descr << "Edit native Antirush points. /aa is a console alias. GUID changes also require C/antirush. "
        "Authenticated legacy level/GUID editor grants remain valid unless explicitly denied in the admin system.";
}
bool Aa::hasPermission(const Context& txt) {
    const int client = txt._client ? txt._client->slot : -1;
    if (!antirush::canEdit(client)) return false;
    if (txt._args.size() > 1 && (!Q_stricmp(txt._args[1].c_str(), "addguid") ||
        !Q_stricmp(txt._args[1].c_str(), "removeguid"))) return antirush::canManage(client);
    return true;
}
AbstractCommand::PostAction Aa::doExecute(Context& txt) {
    antirush::runCommand(txt._client ? txt._client->slot : -1,
        Args(txt._args.begin() + 1, txt._args.end()), false);
    return PA_NONE;
}
}
