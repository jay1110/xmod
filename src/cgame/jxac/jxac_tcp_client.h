#ifndef CGAME_JXAC_TCP_CLIENT_H
#define CGAME_JXAC_TCP_CLIENT_H

///////////////////////////////////////////////////////////////////////////////
// JXAC TCP Client
// Connects to game server on same port (net_port) using TCP protocol
// Sends screenshot data to server
///////////////////////////////////////////////////////////////////////////////

#include <bgame/jxac_tcp.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

class TcpClient {
public:
    // Connect to server
    static qboolean connect(const char* serverIP, int port);
    
    // Disconnect from server
    static void disconnect();
    
    // Process TCP communication (call each frame)
    static void frame();
    
    // Check if connected and ready
    static qboolean isConnected();
    static qboolean isReady();
    
    // Send screenshot over TCP
    static qboolean sendScreenshot(const unsigned char* data, int size, int quality);
    
    // Check if screenshot transfer is in progress
    static qboolean isTransferring();
    
private:
    // Connection process
    static void processConnecting();
    static void processConnected();
    static void processHandshake();
    
    // Send handshake
    static void sendHandshake();
    
    // Process received data
    static void processReceive();
    
    // Send data helpers
    static qboolean sendMessage(jxacTcpMessageType_t type, const void* data, int dataLen);
    static qboolean sendRaw(const void* data, int dataLen);
};

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac

#endif // CGAME_JXAC_TCP_CLIENT_H
