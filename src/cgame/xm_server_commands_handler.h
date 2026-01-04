#ifndef CGAME_XM_SERVER_COMMANDS_HANDLER_H
#define CGAME_XM_SERVER_COMMANDS_HANDLER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

///////////////////////////////////////////////////////////////////////////////
// Server commands handler - event-driven architecture for server commands
// Similar to ETJump's ClientCommandsHandler
///////////////////////////////////////////////////////////////////////////////

namespace xmod {

class ServerCommandsHandler {
public:
    ServerCommandsHandler();
    ~ServerCommandsHandler();

    // Check if a command has a subscription and execute it
    // Returns true if command was handled, false otherwise
    bool check(const std::string& command, const std::vector<std::string>& args);

    // Subscribe a callback to a server command
    bool subscribe(const std::string& command, 
                   std::function<void(const std::vector<std::string>&)> callback);

    // Unsubscribe from a server command
    bool unsubscribe(const std::string& command);

private:
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> callbacks_;
};

// Global instance
extern std::shared_ptr<ServerCommandsHandler> g_serverCommandsHandler;

} // namespace xmod

#endif // CGAME_XM_SERVER_COMMANDS_HANDLER_H
