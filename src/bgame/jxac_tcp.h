#ifndef BGAME_JXAC_TCP_H
#define BGAME_JXAC_TCP_H

///////////////////////////////////////////////////////////////////////////////
// JXAC TCP Socket Abstraction Layer
// Cross-platform TCP socket for screenshot transfer
// Uses same port as game server (net_port) but TCP protocol
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET jxac_socket_t;
#define JXAC_INVALID_SOCKET INVALID_SOCKET
#define JXAC_SOCKET_ERROR SOCKET_ERROR
#define jxac_closesocket closesocket
#define jxac_socket_errno WSAGetLastError()
#define JXAC_EWOULDBLOCK WSAEWOULDBLOCK
#define JXAC_EINPROGRESS WSAEINPROGRESS
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
typedef int jxac_socket_t;
#define JXAC_INVALID_SOCKET (-1)
#define JXAC_SOCKET_ERROR (-1)
#define jxac_closesocket close
#define jxac_socket_errno errno
#define JXAC_EWOULDBLOCK EWOULDBLOCK
#define JXAC_EINPROGRESS EINPROGRESS
#endif

// TCP Message Types (different from UDP binary channel)
typedef enum {
    JXAC_TCP_MSG_HANDSHAKE      = 0x01,   // Client hello / server ack
    JXAC_TCP_MSG_SS_START       = 0x02,   // Screenshot transfer start (includes size)
    JXAC_TCP_MSG_SS_DATA        = 0x03,   // Screenshot data chunk
    JXAC_TCP_MSG_SS_END         = 0x04,   // Screenshot transfer complete
    JXAC_TCP_MSG_SS_ACK         = 0x05,   // Screenshot received acknowledgement
    JXAC_TCP_MSG_DISCONNECT     = 0xFF,   // Clean disconnect
} jxacTcpMessageType_t;

// TCP Protocol Header (4 bytes)
#pragma pack(push, 1)
typedef struct jxacTcpHeader_s {
    unsigned char   type;       // jxacTcpMessageType_t
    unsigned char   flags;      // Reserved for future use
    unsigned short  dataLen;    // Length of data following header (max 65535)
} jxacTcpHeader_t;
#pragma pack(pop)

// TCP Handshake payload
#pragma pack(push, 1)
typedef struct jxacTcpHandshake_s {
    unsigned int    magic;          // JXAC_BINARY_MAGIC
    unsigned short  clientNum;      // Client slot number
    char            guid[33];       // Client GUID (null-terminated)
} jxacTcpHandshake_t;
#pragma pack(pop)

// Screenshot transfer start payload
#pragma pack(push, 1)
typedef struct jxacTcpSsStart_s {
    unsigned int    totalSize;      // Total screenshot size in bytes
    unsigned short  quality;        // JPEG quality
    unsigned short  reserved;       // Alignment padding
} jxacTcpSsStart_t;
#pragma pack(pop)

// TCP Buffer size for screenshot transfer
#define JXAC_TCP_CHUNK_SIZE     (32 * 1024)     // 32KB chunks
#define JXAC_TCP_MAX_SS_SIZE    (4 * 1024 * 1024)  // 4MB max screenshot
#define JXAC_TCP_RECV_BUFFER    (64 * 1024)     // 64KB receive buffer

// Connection state
typedef enum {
    JXAC_TCP_STATE_DISCONNECTED = 0,
    JXAC_TCP_STATE_CONNECTING,
    JXAC_TCP_STATE_CONNECTED,
    JXAC_TCP_STATE_HANDSHAKING,
    JXAC_TCP_STATE_READY,
    JXAC_TCP_STATE_TRANSFERRING,
    JXAC_TCP_STATE_ERROR
} jxacTcpState_t;

///////////////////////////////////////////////////////////////////////////////
// Platform-independent socket helpers
///////////////////////////////////////////////////////////////////////////////

// Initialize socket library (Windows needs WSAStartup)
static inline int jxac_socket_init(void) {
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
    return 0;
#endif
}

// Cleanup socket library
static inline void jxac_socket_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

// Set socket to non-blocking mode
static inline int jxac_socket_setnonblocking(jxac_socket_t sock) {
#ifdef _WIN32
    unsigned long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

// Set TCP_NODELAY (disable Nagle's algorithm for lower latency)
static inline int jxac_socket_setnodelay(jxac_socket_t sock) {
    int flag = 1;
    return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));
}

// Set SO_REUSEADDR (allow port reuse)
static inline int jxac_socket_setreuseaddr(jxac_socket_t sock) {
    int flag = 1;
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag));
}

// Check if error is "would block" (non-blocking socket)
static inline int jxac_socket_wouldblock(void) {
    int err = jxac_socket_errno;
    return (err == JXAC_EWOULDBLOCK || err == JXAC_EINPROGRESS);
}

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_JXAC_TCP_H
