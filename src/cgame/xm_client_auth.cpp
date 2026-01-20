#include "xm_client_auth.h"
#include "xm_server_commands_handler.h"
#include <bgame/xm_auth_shared.h>
#include <bgame/xm_sha1.h>
#include <cgame/cg_local.h>

#ifdef _WIN32
#include <process.h>  // For getpid() on Windows
#include <windows.h>
#include <intrin.h>
#else
#include <unistd.h>   // For getpid() on Linux/Unix
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#ifdef __APPLE__
#include <ifaddrs.h>
#include <net/if_dl.h>
#endif
#endif

#include <sstream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <cstdlib>

// Undefine min/max macros that conflict with C++ Standard Library
// These are defined in q_shared.h (included via cg_local.h) but conflict with std::min/std::max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace xm_client_auth {

namespace {
    std::string g_guid;
    std::string g_hwid;
    bool g_initialized = false;
    bool g_loginPending = false;  // Flag to defer login to first frame
}

///////////////////////////////////////////////////////////////////////////////

std::string generateUUID() {
    std::stringstream ss;
    // Use a combination of time and process ID for better randomness
    unsigned int seed = (unsigned int)(time(NULL) ^ (clock() << 16) ^ getpid());
    srand(seed);
    
    for (int i = 0; i < 32; i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << '-';
        }
        int r = rand() % 16;
        ss << "0123456789abcdef"[r];
    }
    
    return ss.str();
}

///////////////////////////////////////////////////////////////////////////////

std::string loadGuidFromFile() {
    fileHandle_t f = 0;
    int len = trap_FS_FOpenFile("xmodguid.dat", &f, FS_READ);
    if (len > 0 && f != 0) {
        char buffer[64];
        memset(buffer, 0, sizeof(buffer));
        trap_FS_Read(buffer, sizeof(buffer) - 1, f);
        trap_FS_FCloseFile(f);
        
        // Validate file size and GUID format (UUID with dashes or 32 hex chars)
        if (len >= 32) {
            std::string guid(buffer);
            if (guid.length() >= 32) {
                CG_Printf("[Auth] GUID loaded from file: %s\n", guid.substr(0, 36).c_str());
                return guid.substr(0, 36); // UUID format with dashes
            }
        }
    }
    CG_Printf("[Auth] No existing GUID file found\n");
    return "";
}

///////////////////////////////////////////////////////////////////////////////

void saveGuidToFile(const std::string& guid) {
    fileHandle_t f;
    if (trap_FS_FOpenFile("xmodguid.dat", &f, FS_WRITE) >= 0) {
        trap_FS_Write(guid.c_str(), guid.length(), f);
        trap_FS_FCloseFile(f);
        CG_Printf("[Auth] GUID saved to file\n");
    }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
std::string collectHwidWindows() {
    std::stringstream ss;
    
    // Get processor info
    int cpuInfo[4] = {0, 0, 0, 0};
    __cpuid(cpuInfo, 0);
    ss << cpuInfo[1] << cpuInfo[3] << cpuInfo[2];
    
    // Get volume serial number
    DWORD volumeSerial = 0;
    if (GetVolumeInformationA("C:\\", NULL, 0, &volumeSerial, NULL, NULL, NULL, 0)) {
        ss << volumeSerial;
    }
    
    return ss.str();
}
#else
std::string collectHwidUnix() {
    std::stringstream ss;
    bool found = false;
    
#ifdef __APPLE__
    // macOS implementation using getifaddrs and AF_LINK
    struct ifaddrs *ifap, *ifaptr;
    if (getifaddrs(&ifap) == 0) {
        for (ifaptr = ifap; ifaptr != NULL && !found; ifaptr = ifaptr->ifa_next) {
            if (ifaptr->ifa_addr != NULL && ifaptr->ifa_addr->sa_family == AF_LINK) {
                // Check common interface names
                const char* name = ifaptr->ifa_name;
                if (strcmp(name, "en0") == 0 || strcmp(name, "en1") == 0 || 
                    strcmp(name, "eth0") == 0 || strcmp(name, "wlan0") == 0) {
                    
                    struct sockaddr_dl* sdl = static_cast<struct sockaddr_dl*>(static_cast<void*>(ifaptr->ifa_addr));
                    unsigned char* mac = reinterpret_cast<unsigned char*>(LLADDR(sdl));
                    
                    // Check if MAC is not all zeros
                    bool allZeros = true;
                    for (int i = 0; i < 6; i++) {
                        if (mac[i] != 0) {
                            allZeros = false;
                            break;
                        }
                    }
                    if (!allZeros) {
                        for (int i = 0; i < 6; i++) {
                            ss << (int)mac[i];
                        }
                        found = true;
                    }
                }
            }
        }
        freeifaddrs(ifap);
    }
#else
    // Linux implementation using ioctl and SIOCGIFHWADDR
    const char* interfaces[] = {"eth0", "enp0s3", "ens33", "wlan0", "wlp2s0", NULL};
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock >= 0) {
        for (int idx = 0; interfaces[idx] != NULL && !found; idx++) {
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, interfaces[idx], IFNAMSIZ - 1);
            
            if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                unsigned char* mac = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
                // Check if MAC is not all zeros
                bool allZeros = true;
                for (int i = 0; i < 6; i++) {
                    if (mac[i] != 0) {
                        allZeros = false;
                        break;
                    }
                }
                if (!allZeros) {
                    for (int i = 0; i < 6; i++) {
                        ss << (int)mac[i];
                    }
                    found = true;
                }
            }
        }
        close(sock);
    }
#endif
    
    // Fallback to hostname if MAC address not found
    if (!found) {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            ss << hostname;
        } else {
            // Ultimate fallback - use a fixed identifier
            ss << "xmod-unix-client";
        }
    }
    
    return ss.str();
}
#endif

///////////////////////////////////////////////////////////////////////////////

std::string getGuid() {
    if (!g_guid.empty()) {
        return g_guid;
    }
    
    // Try to load from file
    g_guid = loadGuidFromFile();
    
    // Generate new GUID if not found
    if (g_guid.empty()) {
        g_guid = generateUUID();
        CG_Printf("[Auth] Generated new GUID: %s\n", g_guid.c_str());
        saveGuidToFile(g_guid);
    }
    
    return g_guid;
}

///////////////////////////////////////////////////////////////////////////////

std::string getHwid() {
    if (!g_hwid.empty()) {
        return g_hwid;
    }
    
#ifdef _WIN32
    g_hwid = collectHwidWindows();
#else
    g_hwid = collectHwidUnix();
#endif
    
    CG_Printf("[Auth] HWID collected: %s\n", g_hwid.c_str());
    
    return g_hwid;
}

///////////////////////////////////////////////////////////////////////////////

void login() {
    // Get GUID and HWID
    std::string guid = getGuid();
    std::string hwid = getHwid();

    // Hash both with SHA1
    std::string hashedGuid = xm_sha1::hashString(guid);
    std::string hashedHwid = xm_sha1::hashString(hwid);
    
    // Build command string
    std::stringstream cmd;
    cmd << xm_auth::CMD_AUTHENTICATE << " " << hashedGuid << " " << hashedHwid;
    std::string cmdStr = cmd.str();

    CG_Printf("[Auth] Sending: %s\n", cmdStr.c_str());

    // Send authenticate command to server
    trap_SendClientCommand(cmdStr.c_str());
}

///////////////////////////////////////////////////////////////////////////////

void handleGuidRequest() {
    CG_Printf("[Auth] Received guid_request from server\n");
    login();
}

///////////////////////////////////////////////////////////////////////////////

void init() {
    if (g_initialized) {
        return;
    }
    
    CG_Printf("[Auth] Initializing authentication system\n");
    
    g_initialized = true;
    g_guid.clear();
    g_hwid.clear();
    g_loginPending = true;  // Defer login to first frame

    // Subscribe to guid_request command for future requests (e.g., reconnect)
    if (xmod::g_serverCommandsHandler) {
        xmod::g_serverCommandsHandler->subscribe("guid_request",
            [](const std::vector<std::string>& args) {
                CG_Printf("[Auth] Received guid_request from server\n");
                login();
            }
        );
    }

    // Don't send authentication immediately during init
    // Wait for first frame to ensure client is fully ready
    CG_Printf("[Auth] Authentication deferred to first frame\n");
}

///////////////////////////////////////////////////////////////////////////////

void frame() {
    if (!g_initialized) {
        return;
    }

    // Send deferred login on first frame
    if (g_loginPending) {
        g_loginPending = false;
        CG_Printf("[Auth] Sending deferred authentication\n");
        login();
    }
}

///////////////////////////////////////////////////////////////////////////////

void shutdown() {
    g_initialized = false;
    g_guid.clear();
    g_hwid.clear();
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xm_client_auth
