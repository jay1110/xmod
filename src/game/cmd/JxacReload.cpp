#include <bgame/impl.h>
#include <game/jxac/jxac_server.h>

namespace cmd {

JxacReload::JxacReload() : AbstractBuiltin("jxac_reload")
{
    __usage << xvalue("!jxac_reload");
    __descr << "Reload JXAC rules and recheck cached module hashes. "
        "Invalid MD5 files leave the previous list active.";
}

AbstractCommand::PostAction JxacReload::doExecute(Context& txt)
{
    if (txt._args.size() != 1) return PA_USAGE;
    if (!jxac::Server::reloadConfig()) {
        txt._ebuf << "JXAC MD5 rules could not be reloaded; previous list retained. "
            "Check the server console for details.";
        return PA_ERROR;
    }
    Buffer buf;
    buf << "JXAC configuration reload finished; active MD5 rules: "
        << xvalue(int(jxac::Server::getMd5RuleCount()))
        << ". Cached client modules rechecked. See the server console for other rule-file diagnostics.";
    printChat(txt._client, buf);
    return PA_NONE;
}

} // namespace cmd
