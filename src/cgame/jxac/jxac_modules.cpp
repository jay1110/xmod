#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <bgame/xm_sha1.h>
#include <cgame/jxac/jxac_modules.h>
#include <vector>
#include <cstdio>
#include <cstring>

// Undefine min/max macros that conflict with C++ Standard Library
// These are defined in q_shared.h (included via impl.h) but conflict with std::min/std::max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

namespace jxac {

// Module scanning constants
#define JXAC_MAX_MODULE_FILE_SIZE (50 * 1024 * 1024)  // Max 50MB
#define JXAC_MAX_MODULES_WINDOWS 1024  // Max modules to scan on Windows

struct ModuleInfo {
    char name[256];
    char path[512];
    char checksum[41];  // SHA1 hex (40 chars + null)
};

// Calculate SHA1 of file
static bool calculateSHA1(const char* filepath, char* outHash) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return false;
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size <= 0 || size > JXAC_MAX_MODULE_FILE_SIZE) {
        fclose(f);
        return false;
    }
    
    unsigned char* buffer = (unsigned char*)malloc(size);
    if (!buffer) {
        fclose(f);
        return false;
    }
    
    fread(buffer, 1, size, f);
    fclose(f);
    
    // Calculate SHA1 using xm_sha1::SHA1 class
    xm_sha1::SHA1 sha1;
    sha1.update((const uint8_t*)buffer, size);
    uint8_t digest[20];
    sha1.finalize(digest);
    char hexHash[41];
    xm_sha1::toHex(digest, 20, hexHash);
    free(buffer);
    
    // Copy to output
    Q_strncpyz(outHash, hexHash, 41);
    
    return true;
}

#ifdef _WIN32
// Scan loaded modules (Windows)
static void scanLoadedModulesWindows(std::vector<ModuleInfo>& modules) {
    HMODULE hMods[JXAC_MAX_MODULES_WINDOWS];
    HANDLE hProcess = GetCurrentProcess();
    DWORD cbNeeded;
    
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        int count = cbNeeded / sizeof(HMODULE);
        
        for (int i = 0; i < count && i < JXAC_MAX_MODULES_WINDOWS; i++) {
            ModuleInfo info;
            memset(&info, 0, sizeof(info));
            
            // Get module filename
            GetModuleFileNameExA(hProcess, hMods[i], info.path, sizeof(info.path));
            
            // Extract just the filename
            char* slash = strrchr(info.path, '\\');
            if (slash) {
                Q_strncpyz(info.name, slash + 1, sizeof(info.name));
            } else {
                Q_strncpyz(info.name, info.path, sizeof(info.name));
            }
            
            // Calculate SHA1
            if (calculateSHA1(info.path, info.checksum)) {
                modules.push_back(info);
            }
        }
    }
}
#else
// Scan loaded modules (Linux)
static void scanLoadedModulesLinux(std::vector<ModuleInfo>& modules) {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return;
    
    char line[1024];
    char lastPath[512] = "";
    
    while (fgets(line, sizeof(line), f)) {
        // Parse: address perms offset dev inode pathname
        char perms[16], pathname[512];
        if (sscanf(line, "%*s %15s %*s %*s %*s %511s", perms, pathname) == 2) {
            // Skip non-executable mappings
            if (strchr(perms, 'x') == NULL) continue;
            
            // Skip special mappings
            if (pathname[0] == '[') continue;
            
            // Skip duplicates
            if (strcmp(pathname, lastPath) == 0) continue;
            strcpy(lastPath, pathname);
            
            ModuleInfo info;
            memset(&info, 0, sizeof(info));
            Q_strncpyz(info.path, pathname, sizeof(info.path));
            
            // Extract filename
            char* slash = strrchr(pathname, '/');
            if (slash) {
                Q_strncpyz(info.name, slash + 1, sizeof(info.name));
            } else {
                Q_strncpyz(info.name, pathname, sizeof(info.name));
            }
            
            // Calculate SHA1
            if (calculateSHA1(info.path, info.checksum)) {
                modules.push_back(info);
            }
        }
    }
    
    fclose(f);
}
#endif

// Public function to scan and send modules
void scanAndSendModules() {
    std::vector<ModuleInfo> modules;
    
#ifdef _WIN32
    scanLoadedModulesWindows(modules);
#else
    scanLoadedModulesLinux(modules);
#endif
    
    Com_Printf("JXAC: Scanned %d loaded modules\n", (int)modules.size());
    
    // Send each module to server
    for (const auto& mod : modules) {
        trap_SendClientCommand(va("jxac_module %s %s", mod.name, mod.checksum));
    }
    
    // Send completion message
    trap_SendClientCommand(va("jxac_module_complete %d", (int)modules.size()));
}

} // namespace jxac
