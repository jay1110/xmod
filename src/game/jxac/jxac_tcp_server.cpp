#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <bgame/jxac_tcp.h>
#include <game/jxac/jxac_tcp_server.h>
#include <game/jxac/jxac_server.h>
#include <cstring>
#include <cstdlib>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////
// Static members
///////////////////////////////////////////////////////////////////////////////

static qboolean             tcpInitialized = qfalse;
static jxac_socket_t        listenSocket = JXAC_INVALID_SOCKET;
static int                  serverPort = 0;
static jxacTcpClientConn_t  clients[JXAC_TCP_MAX_CLIENTS];

///////////////////////////////////////////////////////////////////////////////

qboolean TcpServer::start(int port) {
    if (tcpInitialized) {
        // Already running, no need to log
        return qtrue;
    }
    
    // Initialize socket library
    if (jxac_socket_init() != 0) {
        Com_Printf("JXAC TCP: Failed to initialize socket library\n");
        return qfalse;
    }
    
    // Create listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == JXAC_INVALID_SOCKET) {
        Com_Printf("JXAC TCP: Failed to create socket\n");
        jxac_socket_cleanup();
        return qfalse;
    }
    
    // Set socket options
    jxac_socket_setreuseaddr(listenSocket);
    jxac_socket_setnonblocking(listenSocket);
    
    // Bind to port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((unsigned short)port);
    
    if (::bind(listenSocket, (struct sockaddr*)&addr, sizeof(addr)) == JXAC_SOCKET_ERROR) {
        Com_Printf("JXAC TCP: Failed to bind to port %d (errno %d)\n", port, jxac_socket_errno);
        jxac_closesocket(listenSocket);
        listenSocket = JXAC_INVALID_SOCKET;
        jxac_socket_cleanup();
        return qfalse;
    }
    
    // Start listening
    if (listen(listenSocket, 10) == JXAC_SOCKET_ERROR) {
        Com_Printf("JXAC TCP: Failed to listen on port %d\n", port);
        jxac_closesocket(listenSocket);
        listenSocket = JXAC_INVALID_SOCKET;
        jxac_socket_cleanup();
        return qfalse;
    }
    
    // Initialize client slots
    memset(clients, 0, sizeof(clients));
    for (int i = 0; i < JXAC_TCP_MAX_CLIENTS; i++) {
        clients[i].socket = JXAC_INVALID_SOCKET;
        clients[i].state = JXAC_TCP_STATE_DISCONNECTED;
        clients[i].clientNum = -1;
    }
    
    serverPort = port;
    tcpInitialized = qtrue;
    
    // Server started successfully
    return qtrue;
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::stop() {
    if (!tcpInitialized) {
        return;
    }
    
    // Close all client connections
    for (int i = 0; i < JXAC_TCP_MAX_CLIENTS; i++) {
        if (clients[i].socket != JXAC_INVALID_SOCKET) {
            closeClient(&clients[i]);
        }
    }
    
    // Close listening socket
    if (listenSocket != JXAC_INVALID_SOCKET) {
        jxac_closesocket(listenSocket);
        listenSocket = JXAC_INVALID_SOCKET;
    }
    
    jxac_socket_cleanup();
    tcpInitialized = qfalse;
    serverPort = 0;
    
    // Server stopped
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::frame() {
    if (!tcpInitialized) {
        return;
    }
    
    // Accept new connections
    acceptConnections();
    
    // Process existing clients
    processClients();
    
    // Check for timeouts
    checkTimeouts();
}

///////////////////////////////////////////////////////////////////////////////

qboolean TcpServer::isRunning() {
    return tcpInitialized;
}

///////////////////////////////////////////////////////////////////////////////

int TcpServer::getPort() {
    return serverPort;
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::disconnectClient(int clientNum) {
    for (int i = 0; i < JXAC_TCP_MAX_CLIENTS; i++) {
        if (clients[i].clientNum == clientNum && clients[i].socket != JXAC_INVALID_SOCKET) {
            closeClient(&clients[i]);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::acceptConnections() {
    struct sockaddr_in clientAddr;
#ifdef _WIN32
    int addrLen = sizeof(clientAddr);
#else
    socklen_t addrLen = sizeof(clientAddr);
#endif
    
    // Try to accept a new connection (non-blocking)
    jxac_socket_t clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
    
    if (clientSocket == JXAC_INVALID_SOCKET) {
        // No pending connection (or error)
        return;
    }
    
    // Find a free client slot
    jxacTcpClientConn_t* conn = NULL;
    for (int i = 0; i < JXAC_TCP_MAX_CLIENTS; i++) {
        if (clients[i].socket == JXAC_INVALID_SOCKET) {
            conn = &clients[i];
            break;
        }
    }
    
    if (!conn) {
        // No free slots
        Com_Printf("JXAC TCP: Max clients reached, rejecting connection\n");
        jxac_closesocket(clientSocket);
        return;
    }
    
    // Initialize connection
    memset(conn, 0, sizeof(*conn));
    conn->socket = clientSocket;
    conn->state = JXAC_TCP_STATE_CONNECTED;
    conn->clientNum = -1;  // Unknown until handshake
    conn->connectTime = level.time;
    conn->lastActivityTime = level.time;
    
    // Store client IP
    inet_ntop(AF_INET, &clientAddr.sin_addr, conn->clientIP, sizeof(conn->clientIP));
    conn->clientPort = ntohs(clientAddr.sin_port);
    
    // Set socket options
    jxac_socket_setnonblocking(clientSocket);
    jxac_socket_setnodelay(clientSocket);
    
    // Client connected successfully
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::processClients() {
    for (int i = 0; i < JXAC_TCP_MAX_CLIENTS; i++) {
        jxacTcpClientConn_t* conn = &clients[i];
        
        if (conn->socket == JXAC_INVALID_SOCKET) {
            continue;
        }
        
        // Try to receive data
        int recvLen = recv(conn->socket, 
                          (char*)(conn->recvBuffer + conn->recvBufferLen),
                          JXAC_TCP_RECV_BUFFER - conn->recvBufferLen,
                          0);
        
        if (recvLen > 0) {
            conn->recvBufferLen += recvLen;
            conn->lastActivityTime = level.time;
            
            // Process received data
            processClientData(conn);
        }
        else if (recvLen == 0) {
            // Connection closed by client
            // Client disconnected normally
            closeClient(conn);
        }
        else {
            // Error or would block
            if (!jxac_socket_wouldblock()) {
                Com_Printf("JXAC TCP: Error receiving from %s:%d (errno %d)\n", 
                          conn->clientIP, conn->clientPort, jxac_socket_errno);
                closeClient(conn);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::processClientData(jxacTcpClientConn_t* conn) {
    // Process complete messages in buffer
    while (conn->recvBufferLen >= (int)sizeof(jxacTcpHeader_t)) {
        jxacTcpHeader_t* header = (jxacTcpHeader_t*)conn->recvBuffer;
        int totalMsgLen = sizeof(jxacTcpHeader_t) + header->dataLen;
        
        // Check if we have the complete message
        if (conn->recvBufferLen < totalMsgLen) {
            break;  // Wait for more data
        }
        
        // Get data pointer
        unsigned char* data = conn->recvBuffer + sizeof(jxacTcpHeader_t);
        
        // Handle message based on type
        switch (header->type) {
            case JXAC_TCP_MSG_HANDSHAKE:
                if (header->dataLen >= sizeof(jxacTcpHandshake_t)) {
                    handleHandshake(conn, (const jxacTcpHandshake_t*)data);
                }
                break;
                
            case JXAC_TCP_MSG_SS_START:
                if (header->dataLen >= sizeof(jxacTcpSsStart_t)) {
                    handleScreenshotStart(conn, (const jxacTcpSsStart_t*)data);
                }
                break;
                
            case JXAC_TCP_MSG_SS_DATA:
                handleScreenshotData(conn, data, header->dataLen);
                break;
                
            case JXAC_TCP_MSG_SS_END:
                handleScreenshotEnd(conn);
                break;
                
            case JXAC_TCP_MSG_DISCONNECT:
                // Client sent disconnect
                closeClient(conn);
                return;  // Connection closed
                
            default:
                Com_Printf("JXAC TCP: Unknown message type %d from %s:%d\n", 
                          header->type, conn->clientIP, conn->clientPort);
                break;
        }
        
        // Remove processed message from buffer
        int remaining = conn->recvBufferLen - totalMsgLen;
        if (remaining > 0) {
            memmove(conn->recvBuffer, conn->recvBuffer + totalMsgLen, remaining);
        }
        conn->recvBufferLen = remaining;
    }
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::handleHandshake(jxacTcpClientConn_t* conn, const jxacTcpHandshake_t* handshake) {
    // Validate magic
    if (handshake->magic != JXAC_BINARY_MAGIC) {
        Com_Printf("JXAC TCP: Invalid handshake magic from %s:%d\n", conn->clientIP, conn->clientPort);
        closeClient(conn);
        return;
    }
    
    // Store client info
    conn->clientNum = handshake->clientNum;
    conn->state = JXAC_TCP_STATE_READY;
    
    // Handshake completed successfully
    
    // Send acknowledgement
    sendAck(conn);
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::handleScreenshotStart(jxacTcpClientConn_t* conn, const jxacTcpSsStart_t* ssStart) {
    if (conn->state != JXAC_TCP_STATE_READY) {
        Com_Printf("JXAC TCP: Screenshot start in wrong state from %s:%d\n", 
                  conn->clientIP, conn->clientPort);
        return;
    }
    
    // Validate size
    if (ssStart->totalSize > JXAC_TCP_MAX_SS_SIZE) {
        Com_Printf("JXAC TCP: Screenshot too large (%u bytes) from client %d\n", 
                  ssStart->totalSize, conn->clientNum);
        return;
    }
    
    // Allocate buffer
    if (conn->ssBuffer) {
        free(conn->ssBuffer);
    }
    conn->ssBuffer = (unsigned char*)malloc(ssStart->totalSize);
    if (!conn->ssBuffer) {
        Com_Printf("JXAC TCP: Failed to allocate screenshot buffer (%u bytes)\n", ssStart->totalSize);
        return;
    }
    
    conn->ssSize = ssStart->totalSize;
    conn->ssReceived = 0;
    conn->ssQuality = ssStart->quality;
    conn->state = JXAC_TCP_STATE_TRANSFERRING;
    
    // Screenshot transfer started
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::handleScreenshotData(jxacTcpClientConn_t* conn, const unsigned char* data, int dataLen) {
    if (conn->state != JXAC_TCP_STATE_TRANSFERRING || !conn->ssBuffer) {
        return;
    }
    
    // Check bounds
    if (conn->ssReceived + dataLen > conn->ssSize) {
        conn->state = JXAC_TCP_STATE_READY;
        free(conn->ssBuffer);
        conn->ssBuffer = NULL;
        return;
    }
    
    // Copy data to buffer
    memcpy(conn->ssBuffer + conn->ssReceived, data, dataLen);
    conn->ssReceived += dataLen;
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::handleScreenshotEnd(jxacTcpClientConn_t* conn) {
    if (conn->state != JXAC_TCP_STATE_TRANSFERRING || !conn->ssBuffer) {
        return;
    }
    
    // Verify we received all data
    if (conn->ssReceived != conn->ssSize) {
        Com_Printf("JXAC TCP: Screenshot incomplete from client %d (%u/%u bytes)\n",
                  conn->clientNum, conn->ssReceived, conn->ssSize);
        free(conn->ssBuffer);
        conn->ssBuffer = NULL;
        conn->state = JXAC_TCP_STATE_READY;
        return;
    }
    
    // Screenshot received successfully
    
    // Pass screenshot to JXAC server for processing
    if (conn->clientNum >= 0 && conn->clientNum < MAX_CLIENTS) {
        Server::handleScreenshotData(conn->clientNum, conn->ssBuffer, conn->ssSize);
        Server::handleScreenshotComplete(conn->clientNum);
    }
    
    // Send acknowledgement
    sendAck(conn);
    
    // Clean up
    free(conn->ssBuffer);
    conn->ssBuffer = NULL;
    conn->ssSize = 0;
    conn->ssReceived = 0;
    conn->state = JXAC_TCP_STATE_READY;
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::sendAck(jxacTcpClientConn_t* conn) {
    jxacTcpHeader_t header;
    header.type = JXAC_TCP_MSG_SS_ACK;
    header.flags = 0;
    header.dataLen = 0;
    
    int sent = send(conn->socket, (const char*)&header, sizeof(header), 0);
    if (sent != sizeof(header)) {
        Com_Printf("JXAC TCP: Failed to send ACK to %s:%d\n", conn->clientIP, conn->clientPort);
    }
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::closeClient(jxacTcpClientConn_t* conn) {
    if (conn->socket != JXAC_INVALID_SOCKET) {
        jxac_closesocket(conn->socket);
        conn->socket = JXAC_INVALID_SOCKET;
    }
    
    if (conn->ssBuffer) {
        free(conn->ssBuffer);
        conn->ssBuffer = NULL;
    }
    
    conn->state = JXAC_TCP_STATE_DISCONNECTED;
    conn->clientNum = -1;
    conn->recvBufferLen = 0;
}

///////////////////////////////////////////////////////////////////////////////

void TcpServer::checkTimeouts() {
    int currentTime = level.time;
    
    for (int i = 0; i < JXAC_TCP_MAX_CLIENTS; i++) {
        jxacTcpClientConn_t* conn = &clients[i];
        
        if (conn->socket == JXAC_INVALID_SOCKET) {
            continue;
        }
        
        // Check for timeout
        if (currentTime - conn->lastActivityTime > JXAC_TCP_TIMEOUT) {
            // Client timed out
            closeClient(conn);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

jxacTcpClientConn_t* TcpServer::findClientByNum(int clientNum) {
    for (int i = 0; i < JXAC_TCP_MAX_CLIENTS; i++) {
        if (clients[i].clientNum == clientNum && clients[i].socket != JXAC_INVALID_SOCKET) {
            return &clients[i];
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
