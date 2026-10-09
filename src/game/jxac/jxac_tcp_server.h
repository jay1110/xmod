#ifndef GAME_JXAC_TCP_SERVER_H
#define GAME_JXAC_TCP_SERVER_H

///////////////////////////////////////////////////////////////////////////////
// JXAC TCP Server
// Listens on same port as game server (net_port) using TCP protocol
// Handles screenshot data transfer from clients
///////////////////////////////////////////////////////////////////////////////

#include <bgame/jxac_tcp.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

// Per-client TCP connection state
typedef struct {
    jxac_socket_t       socket;
    jxacTcpState_t      state;
    int                 clientNum;          // Game client slot (-1 if unknown)
    char                clientIP[64];       // Client IP address
    unsigned short      clientPort;         // Client port
    
    // Screenshot receive buffer
    unsigned char*      ssBuffer;           // Screenshot data buffer
    unsigned int        ssSize;             // Total expected size
    unsigned int        ssReceived;         // Bytes received so far
    int                 ssQuality;          // JPEG quality
    int                 ssRequestTime;      // Pending game-channel request at start
    
    // Receive buffer for partial messages
    unsigned char       recvBuffer[JXAC_TCP_RECV_BUFFER];
    int                 recvBufferLen;      // Bytes in receive buffer
    
    // Timing
    int                 connectTime;        // Connection timestamp
    int                 lastActivityTime;   // Last activity timestamp
} jxacTcpClientConn_t;

#define JXAC_TCP_MAX_CLIENTS    64
#define JXAC_TCP_TIMEOUT        30000   // 30 second timeout

class TcpServer {
public:
    // Start the TCP server on specified port
    static qboolean start(int port);
    
    // Stop the TCP server
    static void stop();
    
    // Process TCP connections (call each frame)
    static void frame();
    
    // Check if server is running
    static qboolean isRunning();
    
    // Get server port
    static int getPort();
    
    // Disconnect a specific client (by game client number)
    static void disconnectClient(int clientNum);
    
private:
    // Accept new connections
    static void acceptConnections();
    
    // Process data from connected clients
    static void processClients();
    
    // Process received data for a client
    static void processClientData(jxacTcpClientConn_t* conn);
    
    // Handle specific message types
    static void handleHandshake(jxacTcpClientConn_t* conn, const jxacTcpHandshake_t* handshake);
    static void handleScreenshotStart(jxacTcpClientConn_t* conn, const jxacTcpSsStart_t* ssStart);
    static void handleScreenshotData(jxacTcpClientConn_t* conn, const unsigned char* data, int dataLen);
    static void handleScreenshotEnd(jxacTcpClientConn_t* conn);
    
    // Send acknowledgement
    static void sendAck(jxacTcpClientConn_t* conn);
    
    // Close a client connection
    static void closeClient(jxacTcpClientConn_t* conn);
    
    // Check for timed out connections
    static void checkTimeouts();
    
    // Find client connection by game client number
    static jxacTcpClientConn_t* findClientByNum(int clientNum);
};

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac

#endif // GAME_JXAC_TCP_SERVER_H
