#include "xm_client_auth.h"
#include <bgame/xm_auth_shared.h>
#include <bgame/xm_sha1.h>
#include <cgame/cg_local.h>

#include <unistd.h>  // For getpid() on all platforms

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#endif

#include <sstream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <cstdlib>

namespace xm_client_auth {

namespace {
    std::string g_guid;
    std::string g_hwid;
    bool g_initialized = false;
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
        int bytesRead = trap_FS_Read(buffer, sizeof(buffer) - 1, f);
        trap_FS_FCloseFile(f);
        
        // Validate read was successful (non-negative) and GUID format (UUID with dashes or 32 hex chars)
        if (bytesRead >= 32) {
            std::string guid(buffer);
            if (guid.length() >= 32) {
                return guid.substr(0, 36); // UUID format with dashes
            }
        }
    }
    return "";
}

///////////////////////////////////////////////////////////////////////////////

void saveGuidToFile(const std::string& guid) {
    fileHandle_t f;
    if (trap_FS_FOpenFile("xmodguid.dat", &f, FS_WRITE) >= 0) {
        trap_FS_Write(guid.c_str(), guid.length(), f);
        trap_FS_FCloseFile(f);
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
std::string collectHwidLinux() {
    std::stringstream ss;
    
    // Try to get MAC address from common interface names
    const char* interfaces[] = {"eth0", "enp0s3", "ens33", "wlan0", "wlp2s0", NULL};
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    bool found = false;
    
    if (sock >= 0) {
        for (int idx = 0; interfaces[idx] != NULL && !found; idx++) {
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, interfaces[idx], IFNAMSIZ - 1);
            
            if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
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
    
    // Fallback to hostname if MAC address not found
    if (!found) {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            ss << hostname;
        } else {
            // Ultimate fallback - use a fixed identifier
            ss << "xmod-linux-client";
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
    g_hwid = collectHwidLinux();
#endif
    
    return g_hwid;
}

///////////////////////////////////////////////////////////////////////////////

void handleGuidRequest() {
    // Get GUID and HWID
    std::string guid = getGuid();
    std::string hwid = getHwid();
    
    // Hash both with SHA1
    std::string hashedGuid = xm_sha1::hashString(guid);
    std::string hashedHwid = xm_sha1::hashString(hwid);
    
    // Send authenticate command to server
    std::stringstream cmd;
    cmd << xm_auth::CMD_AUTHENTICATE << " " << hashedGuid << " " << hashedHwid;
    trap_SendClientCommand(cmd.str().c_str());
}

///////////////////////////////////////////////////////////////////////////////

void init() {
    if (g_initialized) {
        return;
    }
    
    g_initialized = true;
    g_guid.clear();
    g_hwid.clear();
}

///////////////////////////////////////////////////////////////////////////////

void shutdown() {
    g_initialized = false;
    g_guid.clear();
    g_hwid.clear();
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xm_client_auth
