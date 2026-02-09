#include "xm_server_commands_handler.h"
#include <cgame/cg_local.h>

namespace xmod {

// Global instance
std::shared_ptr<ServerCommandsHandler> g_serverCommandsHandler;

///////////////////////////////////////////////////////////////////////////////

ServerCommandsHandler::ServerCommandsHandler() {
}

///////////////////////////////////////////////////////////////////////////////

ServerCommandsHandler::~ServerCommandsHandler() {
    callbacks_.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool ServerCommandsHandler::check(const std::string& command, 
                                   const std::vector<std::string>& args) {
    auto it = callbacks_.find(command);
    if (it != callbacks_.end()) {
        // Command found, execute callback
        it->second(args);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool ServerCommandsHandler::subscribe(const std::string& command, 
                                       std::function<void(const std::vector<std::string>&)> callback) {
    if (callbacks_.find(command) != callbacks_.end()) {
        CG_Printf("[ServerCommandsHandler] Warning: Command '%s' already has a subscription\n", 
                  command.c_str());
        return false;
    }
    
    callbacks_[command] = callback;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ServerCommandsHandler::unsubscribe(const std::string& command) {
    auto it = callbacks_.find(command);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod
