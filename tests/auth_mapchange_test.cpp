#include <bgame/impl.h>
#include <game/xmod_globals.h>
#include <game/xm_main_ext.h>
#include <cstdio>
#include <cstdlib>

// Exercise the production command handler, session, UserManager and SQLite.
// Only the engine boundary is mocked; no map assets or running ET server needed.
static std::string guidArg, hwidArg;
static int drops;

static void check(bool condition, const char *message)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

static intptr_t QDECL syscallStub(intptr_t command, ...)
{
    va_list args;
    va_start(args, command);
    switch (command) {
    case G_ARGV: {
        int index = va_arg(args, int);
        char *buffer = va_arg(args, char *);
        int size = va_arg(args, int);
        Q_strncpyz(buffer, index == 1 ? guidArg.c_str() : hwidArg.c_str(), size);
        break;
    }
    case G_GET_USERINFO: {
        (void)va_arg(args, int);
        char *buffer = va_arg(args, char *);
        int size = va_arg(args, int);
        Q_strncpyz(buffer, "\\ip\\127.0.0.1\\cl_mac\\AA:BB:CC:DD:EE:FF", size);
        break;
    }
    case G_DROP_CLIENT:
        ++drops;
        break;
    case G_PRINT:
        break;
    default:
        std::fprintf(stderr, "Unexpected engine call: %lld\n", (long long)command);
        std::exit(2);
    }
    va_end(args);
    return 0;
}

int main()
{
    Engine::ptr = syscallStub;
    level.clients = g_clients;
    level.numConnectedClients = 1;
    level.sortedClients[0] = 0;
    g_entities[0].client = &g_clients[0];
    Q_strncpyz(g_clients[0].pers.netname, "^2Admin", sizeof(g_clients[0].pers.netname));
    g_xpSave.integer = 0;

    xmod::Database database;
    check(database.open(":memory:"), "open isolated SQLite database");
    xmod::g_database = &database;
    xmod::Session session(&database);
    xmod::g_sessions[0] = &session;
    std::string err;
    Level& admin = levelDB.fetchByKey(5, err, true);
    admin.privGranted.insert(priv::base::adminChat);
    hwidArg = std::string(40, 'b');

    // Repeat fresh-map authentication in both connection states. Distinct GUIDs
    // ensure no previously authenticated User cache can conceal the regression.
    for (int pass = 0; pass < 8; ++pass) {
        guidArg = std::string(40, char('1' + pass));
        check(database.addUser(guidArg, hwidArg, "Admin"), "seed account");
        xmod::UserData data;
        check(database.getUserData(guidArg, data), "read account");
        check(database.setLevel(data.id, 5), "seed admin level");
        check(database.setMuteData(data.id, true, 10, time(nullptr) + 3600, "test", "console"), "seed mute");
        User& pending = userManager.fetchByKey("PENDING00" + std::string(31, '0'), err, true);
        pending.authLevel = 0;
        pending.fakeguid = true;
        connectedUsers[0] = &pending;
        g_clientObjects[0].authenticated = false;
        g_clientObjects[0].authGuid.clear();
        g_clients[0].pers.connected = pass % 2 ? CON_CONNECTED : CON_CONNECTING;
        session.init(0, "127.0.0.1");
        check(xmod::OnClientCommand(0, "authenticate") == qtrue, "handle authenticate");
        check(drops == 0, "successful auth must not drop client");
        check(session.isAuthenticated() && g_clientObjects[0].authenticated, "both auth flags");
        check(connectedUsers[0]->guid == guidArg, "bind confirmed GUID");
        check(connectedUsers[0]->authLevel == 5 && xmod::getClientLevel(0) == 5, "level immediately restored");
        check(connectedUsers[0]->hasPrivilege(priv::base::adminChat), "admin rights immediately restored");
        check(connectedUsers[0]->muted && session.isMuted(), "mute state belongs to confirmed user");
        check(pending.authLevel == 0 && pending.fakeguid, "never authenticate the pending user");
        check(database.getUserData(guidArg, data) && data.level == 5, "database level unchanged");

        // Repeated auth and the session-restoration path used by ClientBegin.
        xmod::OnClientCommand(0, "authenticate");
        connectedUsers[0] = &User::BAD;
        session.init(0, "127.0.0.1");
        session.onGuidReceived(guidArg, hwidArg);
        check(connectedUsers[0]->authLevel == 5, "session restore binds even without a pending user");
    }

    session.init(0, "127.0.0.1");
    g_clientObjects[0].authenticated = false;
    connectedUsers[0] = &User::BAD;
    guidArg = "invalid";
    xmod::OnClientCommand(0, "authenticate");
    check(!session.isAuthenticated() && !g_clientObjects[0].authenticated && drops == 0,
          "invalid GUID does not authenticate");

    guidArg = std::string(40, '8');
    level.numConnectedClients = 2;
    level.sortedClients[1] = 1;
    g_clientObjects[1].authenticated = true;
    g_clientObjects[1].authGuid = guidArg;
    xmod::OnClientCommand(0, "authenticate");
    check(drops == 1 && !session.isAuthenticated(), "duplicate GUID is rejected");
    level.numConnectedClients = 1;
    g_clientObjects[1].authenticated = false;

    g_entities[0].r.svFlags |= SVF_BOT;
    xmod::OnClientCommand(0, "authenticate");
    check(drops == 1 && !session.isAuthenticated(), "bots cannot submit authenticate commands");
    g_entities[0].r.svFlags &= ~SVF_BOT;

    check(database.banUser(guidArg, hwidArg, "", "Admin", "console", "test", 0), "seed ban");
    xmod::OnClientCommand(0, "authenticate");
    check(drops == 2 && !session.isAuthenticated() && !g_clientObjects[0].authenticated,
          "banned account never gets authenticated flags");
    check(connectedUsers[0] == &User::BAD, "ban does not change runtime identity");

    // Failed database authentication must never be announced as successful.
    session.init(0, "127.0.0.1");
    g_clientObjects[0].authenticated = false;
    connectedUsers[0] = &User::BAD;
    database.close();
    xmod::OnClientCommand(0, "authenticate");
    check(drops == 3 && !session.isAuthenticated() && !g_clientObjects[0].authenticated,
          "database failure stops authentication");
    check(connectedUsers[0] == &User::BAD, "failed auth leaves runtime identity unchanged");
    xmod::g_sessions[0] = nullptr;
    xmod::g_database = nullptr;
    std::puts("PASS: early/late authentication, repeated auth, session restore, rights, mute, invalid/duplicate GUID, bots, bans and DB failure");
    return 0;
}
