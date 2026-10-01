#include <bgame/impl.h>
#include <game/g_maprecords.h>

namespace cmd {
Records::Records() : AbstractBuiltin("records", true) {
    __usage << xvalue("!records");
    __descr << "Show the current map's saved spree and frag records.";
}

AbstractCommand::PostAction Records::doExecute(Context& txt) {
    if (txt._args.size() != 1) return PA_USAGE;
    G_PrintMapRecords(txt._client ? txt._client->slot : -1);
    return PA_NONE;
}
}
