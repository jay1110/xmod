#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <bgame/jxac_tcp.h>
#include <cgame/jxac/jxac_tcp_client.h>
#include <cstring>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////
// Static members
///////////////////////////////////////////////////////////////////////////////

static qboolean             tcpInitialized = qfalse;
static jxac_socket_t        clientSocket = JXAC_INVALID_SOCKET;
static jxacTcpState_t       clientState = JXAC_TCP_STATE_DISCONNECTED;
static char                 serverAddress[256] = {0};
static int                  serverPort = 0;
static int                  connectStartTime = 0;
static int                  lastActivityTime = 0;

// Send buffer for screenshot transfer
static const unsigned char* sendBuffer = NULL;
static int                  sendSize = 0;
static int                  sendOffset = 0;
static int                  sendQuality = 0;

// Receive buffer for responses
static unsigned char        recvBuffer[1024];
static int                  recvBufferLen = 0;

#define JXAC_TCP_CONNECT_TIMEOUT    5000    // 5 second connect timeout

///////////////////////////////////////////////////////////////////////////////

qboolean TcpClient::connect(const char* serverIP, int port) {
    if (clientSocket != JXAC_INVALID_SOCKET) {
        disconnect();
    }
    
    // Initialize socket library
    if (!tcpInitialized) {
        if (jxac_socket_init() != 0) {
            CG_Printf("JXAC TCP: Failed to initialize socket library\n");
            return qfalse;
        }
        tcpInitialized = qtrue;
    }
    
    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == JXAC_INVALID_SOCKET) {
        CG_Printf("JXAC TCP: Failed to create socket\n");
        return qfalse;
    }
    
    // Set non-blocking before connect
    jxac_socket_setnonblocking(clientSocket);
    
    // Build server address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    
    // Convert IP address
    if (inet_pton(AF_INET, serverIP, &addr.sin_addr) <= 0) {
        CG_Printf("JXAC TCP: Invalid server address: %s\n", serverIP);
        jxac_closesocket(clientSocket);
        clientSocket = JXAC_INVALID_SOCKET;
        return qfalse;
    }
    
    // Store connection info
    Q_strncpyz(serverAddress, serverIP, sizeof(serverAddress));
    serverPort = port;
    
    // Start async connect
    int result = ::connect(clientSocket, (struct sockaddr*)&addr, sizeof(addr));
    
    if (result == JXAC_SOCKET_ERROR) {
        if (!jxac_socket_wouldblock()) {
            CG_Printf("JXAC TCP: Connect failed to %s:%d (errno %d)\n", 
                     serverIP, port, jxac_socket_errno);
            jxac_closesocket(clientSocket);
            clientSocket = JXAC_INVALID_SOCKET;
            return qfalse;
        }
        // Connect in progress (expected for non-blocking)
    }
    
    clientState = JXAC_TCP_STATE_CONNECTING;
    connectStartTime = cg.time;
    lastActivityTime = cg.time;
    
    CG_Printf("JXAC TCP: Connecting to %s:%d...\n", serverIP, port);
    return qtrue;
}

///////////////////////////////////////////////////////////////////////////////

void TcpClient::disconnect() {
    if (clientSocket != JXAC_INVALID_SOCKET) {
        // Send disconnect message if connected
        if (clientState == JXAC_TCP_STATE_READY || clientState == JXAC_TCP_STATE_CONNECTED) {
            sendMessage(JXAC_TCP_MSG_DISCONNECT, NULL, 0);
        }
        
        jxac_closesocket(clientSocket);
        clientSocket = JXAC_INVALID_SOCKET;
    }
    
    clientState = JXAC_TCP_STATE_DISCONNECTED;
    sendBuffer = NULL;
    sendSize = 0;
    sendOffset = 0;
    recvBufferLen = 0;
    
    CG_Printf("JXAC TCP: Disconnected\n");
}

///////////////////////////////////////////////////////////////////////////////

void TcpClient::frame() {
    if (clientSocket == JXAC_INVALID_SOCKET) {
        return;
    }
    
    switch (clientState) {
        case JXAC_TCP_STATE_CONNECTING:
            processConnecting();
            break;
            
        case JXAC_TCP_STATE_CONNECTED:
            sendHandshake();
            break;
            
        case JXAC_TCP_STATE_HANDSHAKING:
            processReceive();
            break;
            
        case JXAC_TCP_STATE_READY:
        case JXAC_TCP_STATE_TRANSFERRING:
            processConnected();
            break;
            
        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

qboolean TcpClient::isConnected() {
    if (clientSocket != JXAC_INVALID_SOCKET && 
        clientState != JXAC_TCP_STATE_DISCONNECTED &&
        clientState != JXAC_TCP_STATE_ERROR) {
        return qtrue;
    }
    return qfalse;
}

///////////////////////////////////////////////////////////////////////////////

qboolean TcpClient::isReady() {
    return (clientState == JXAC_TCP_STATE_READY) ? qtrue : qfalse;
}

///////////////////////////////////////////////////////////////////////////////

qboolean TcpClient::isTransferring() {
    return (clientState == JXAC_TCP_STATE_TRANSFERRING) ? qtrue : qfalse;
}

///////////////////////////////////////////////////////////////////////////////

void TcpClient::processConnecting() {
    // Check connect timeout
    if (cg.time - connectStartTime > JXAC_TCP_CONNECT_TIMEOUT) {
        CG_Printf("JXAC TCP: Connect timeout\n");
        disconnect();
        return;
    }
    
    // Check if connect completed using select()
    fd_set writeSet;
    fd_set errorSet;
    struct timeval tv = {0, 0};  // Immediate return
    
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    FD_SET(clientSocket, &writeSet);
    FD_SET(clientSocket, &errorSet);
    
    int result = select((int)clientSocket + 1, NULL, &writeSet, &errorSet, &tv);
    
    if (result > 0) {
        if (FD_ISSET(clientSocket, &errorSet)) {
            CG_Printf("JXAC TCP: Connect failed\n");
            disconnect();
            return;
        }
        
        if (FD_ISSET(clientSocket, &writeSet)) {
            // Connected!
            CG_Printf("JXAC TCP: Connected to %s:%d\n", serverAddress, serverPort);
            jxac_socket_setnodelay(clientSocket);
            clientState = JXAC_TCP_STATE_CONNECTED;
            lastActivityTime = cg.time;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void TcpClient::sendHandshake() {
    jxacTcpHandshake_t handshake;
    memset(&handshake, 0, sizeof(handshake));
    
    handshake.magic = JXAC_BINARY_MAGIC;
    handshake.clientNum = cg.clientNum;
    
    // Get GUID from userinfo
    char userinfo[MAX_INFO_STRING];
    const char* configStr = CG_ConfigString(CS_PLAYERS + cg.clientNum);
    Q_strncpyz(userinfo, configStr, sizeof(userinfo));
    const char* guid = Info_ValueForKey(userinfo, "guid");
    if (guid && *guid) {
        Q_strncpyz(handshake.guid, guid, sizeof(handshake.guid));
    }
    
    if (sendMessage(JXAC_TCP_MSG_HANDSHAKE, &handshake, sizeof(handshake))) {
        clientState = JXAC_TCP_STATE_HANDSHAKING;
        CG_Printf("JXAC TCP: Sent handshake\n");
    } else {
        CG_Printf("JXAC TCP: Failed to send handshake\n");
        disconnect();
    }
}

///////////////////////////////////////////////////////////////////////////////

void TcpClient::processConnected() {
    // Continue screenshot transfer if in progress
    if (clientState == JXAC_TCP_STATE_TRANSFERRING && sendBuffer && sendOffset < sendSize) {
        // Send next chunk
        int chunkSize = sendSize - sendOffset;
        if (chunkSize > JXAC_TCP_CHUNK_SIZE) {
            chunkSize = JXAC_TCP_CHUNK_SIZE;
        }
        
        // Log progress periodically (every 10%)
        int progressPercent = (sendOffset * 100) / sendSize;
        static int lastProgress = -1;
        if (progressPercent / 10 != lastProgress / 10) {
            CG_Printf("JXAC TCP DEBUG: Transfer progress: %d/%d bytes (%d%%)\n", 
                     sendOffset, sendSize, progressPercent);
            lastProgress = progressPercent;
        }
        
        if (sendMessage(JXAC_TCP_MSG_SS_DATA, sendBuffer + sendOffset, chunkSize)) {
            sendOffset += chunkSize;
            lastActivityTime = cg.time;
            
            // Check if transfer complete
            if (sendOffset >= sendSize) {
                // Send end marker
                CG_Printf("JXAC TCP DEBUG: Sending SS_END message\n");
                sendMessage(JXAC_TCP_MSG_SS_END, NULL, 0);
                
                CG_Printf("JXAC TCP DEBUG: Screenshot transfer complete (%d bytes sent successfully)\n", sendSize);
                sendBuffer = NULL;
                sendSize = 0;
                sendOffset = 0;
                clientState = JXAC_TCP_STATE_READY;
                lastProgress = -1;  // Reset for next transfer
            }
        } else {
            CG_Printf("JXAC TCP DEBUG: sendMessage(SS_DATA) FAILED at offset %d/%d\n", sendOffset, sendSize);
        }
    }
    
    // Process any incoming data
    processReceive();
}

///////////////////////////////////////////////////////////////////////////////

void TcpClient::processReceive() {
    // Try to receive data (non-blocking)
    int recvLen = recv(clientSocket, 
                       (char*)(recvBuffer + recvBufferLen),
                       sizeof(recvBuffer) - recvBufferLen,
                       0);
    
    if (recvLen > 0) {
        recvBufferLen += recvLen;
        lastActivityTime = cg.time;
        
        // Process complete messages
        while (recvBufferLen >= (int)sizeof(jxacTcpHeader_t)) {
            jxacTcpHeader_t* header = (jxacTcpHeader_t*)recvBuffer;
            int totalMsgLen = sizeof(jxacTcpHeader_t) + header->dataLen;
            
            if (recvBufferLen < totalMsgLen) {
                break;  // Wait for more data
            }
            
            // Handle message
            switch (header->type) {
                case JXAC_TCP_MSG_SS_ACK:
                    if (clientState == JXAC_TCP_STATE_HANDSHAKING) {
                        CG_Printf("JXAC TCP: Handshake acknowledged\n");
                        clientState = JXAC_TCP_STATE_READY;
                    }
                    break;
                    
                case JXAC_TCP_MSG_DISCONNECT:
                    CG_Printf("JXAC TCP: Server disconnected\n");
                    disconnect();
                    return;
                    
                default:
                    break;
            }
            
            // Remove processed message
            int remaining = recvBufferLen - totalMsgLen;
            if (remaining > 0) {
                memmove(recvBuffer, recvBuffer + totalMsgLen, remaining);
            }
            recvBufferLen = remaining;
        }
    }
    else if (recvLen == 0) {
        // Connection closed
        CG_Printf("JXAC TCP: Server closed connection\n");
        disconnect();
    }
    else {
        // Error or would block
        if (!jxac_socket_wouldblock()) {
            CG_Printf("JXAC TCP: Receive error (errno %d)\n", jxac_socket_errno);
            disconnect();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

qboolean TcpClient::sendScreenshot(const unsigned char* data, int size, int quality) {
    CG_Printf("JXAC TCP DEBUG: sendScreenshot() called - state=%d, data=%p, size=%d, quality=%d\n",
             clientState, (void*)data, size, quality);
    
    if (clientState != JXAC_TCP_STATE_READY) {
        CG_Printf("JXAC TCP DEBUG: sendScreenshot() FAILED - not ready (state=%d, expected=%d)\n", 
                 clientState, JXAC_TCP_STATE_READY);
        return qfalse;
    }
    
    if (!data || size <= 0 || size > JXAC_TCP_MAX_SS_SIZE) {
        CG_Printf("JXAC TCP DEBUG: sendScreenshot() FAILED - invalid data (data=%p, size=%d, max=%d)\n",
                 (void*)data, size, JXAC_TCP_MAX_SS_SIZE);
        return qfalse;
    }
    
    // Send start message
    jxacTcpSsStart_t ssStart;
    ssStart.totalSize = size;
    ssStart.quality = quality;
    ssStart.reserved = 0;
    
    CG_Printf("JXAC TCP DEBUG: Sending SS_START message (totalSize=%d, quality=%d)\n", size, quality);
    
    if (!sendMessage(JXAC_TCP_MSG_SS_START, &ssStart, sizeof(ssStart))) {
        CG_Printf("JXAC TCP DEBUG: sendScreenshot() FAILED - sendMessage(SS_START) failed\n");
        return qfalse;
    }
    
    // Store buffer for chunked transfer
    sendBuffer = data;
    sendSize = size;
    sendOffset = 0;
    sendQuality = quality;
    clientState = JXAC_TCP_STATE_TRANSFERRING;
    
    CG_Printf("JXAC TCP DEBUG: Screenshot transfer started (%d bytes, state now TRANSFERRING)\n", size);
    return qtrue;
}

///////////////////////////////////////////////////////////////////////////////

qboolean TcpClient::sendMessage(jxacTcpMessageType_t type, const void* data, int dataLen) {
    jxacTcpHeader_t header;
    header.type = type;
    header.flags = 0;
    header.dataLen = (unsigned short)dataLen;
    
    // Send header
    if (!sendRaw(&header, sizeof(header))) {
        return qfalse;
    }
    
    // Send data if present
    if (data && dataLen > 0) {
        if (!sendRaw(data, dataLen)) {
            return qfalse;
        }
    }
    
    return qtrue;
}

///////////////////////////////////////////////////////////////////////////////

qboolean TcpClient::sendRaw(const void* data, int dataLen) {
    const char* ptr = (const char*)data;
    int remaining = dataLen;
    int totalSent = 0;
    
    while (remaining > 0) {
        int sent = send(clientSocket, ptr, remaining, 0);
        
        if (sent > 0) {
            ptr += sent;
            remaining -= sent;
            totalSent += sent;
        }
        else if (sent == JXAC_SOCKET_ERROR) {
            if (jxac_socket_wouldblock()) {
                // Would block - try again next frame
                // For simplicity, we'll just fail here
                // A more robust implementation would buffer unsent data
                CG_Printf("JXAC TCP DEBUG: sendRaw() FAILED - would block (sent %d/%d bytes)\n",
                         totalSent, dataLen);
                return qfalse;
            }
            CG_Printf("JXAC TCP DEBUG: sendRaw() FAILED - socket error %d (sent %d/%d bytes)\n",
                     jxac_socket_errno, totalSent, dataLen);
            return qfalse;
        }
    }
    
    return qtrue;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
