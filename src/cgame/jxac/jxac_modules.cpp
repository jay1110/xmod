#include <bgame/impl.h>
#include <bgame/xm_md5.h>
#include <bgame/xm_sha1.h>
#include <cgame/jxac/jxac_modules.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <cstdio>
#include <cstring>
#include <deque>
#include <set>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#elif !defined(__EMSCRIPTEN__)
#include <sys/stat.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#include <link.h>
#endif
#endif

namespace jxac {
namespace {

// Bound memory and frame work independently of the reliable-command budget.
static const size_t MAX_MODULES = 4096;
static const size_t MAX_READY_REPORTS = 32;
static const size_t BYTES_PER_FRAME = 256 * 1024;
static const size_t READ_CHUNK_SIZE = 64 * 1024;
static const size_t MAX_BASENAME_BYTES = 127;

#ifdef _WIN32
typedef std::wstring ModulePath;
#else
typedef std::string ModulePath;
#endif

struct ModuleReport {
    char md5[33];
    char sha1[41];
    char basenameHex[MAX_BASENAME_BYTES * 2 + 1];
};

static std::vector<ModulePath> modulePaths;
static size_t nextModule = 0;
static std::deque<ModuleReport> readyReports;
static unsigned sentReports = 0;
static bool transferActive = false;
static xm_md5::MD5 md5;
static xm_sha1::SHA1 sha1;
static uint64_t expectedBytes = 0;
static uint64_t readBytes = 0;
static std::string activeBasename;

#ifdef _WIN32
static HANDLE activeFile = INVALID_HANDLE_VALUE;
static BY_HANDLE_FILE_INFORMATION initialFileInfo;
#else
static FILE* activeFile = NULL;
#if !defined(__EMSCRIPTEN__)
static struct stat initialFileInfo;
#endif
#endif

static bool fileOpen() {
#ifdef _WIN32
    return activeFile != INVALID_HANDLE_VALUE;
#else
    return activeFile != NULL;
#endif
}

static void closeFile() {
#ifdef _WIN32
    if (fileOpen()) CloseHandle(activeFile);
    activeFile = INVALID_HANDLE_VALUE;
#else
    if (fileOpen()) fclose(activeFile);
    activeFile = NULL;
#endif
    expectedBytes = readBytes = 0;
    activeBasename.clear();
}

static void addPath(const ModulePath& path, std::set<ModulePath>& seen) {
    if (!path.empty() && modulePaths.size() < MAX_MODULES && seen.insert(path).second) {
        modulePaths.push_back(path);
    }
}

#if defined(_WIN32)
static void discoverModules() {
    std::set<ModulePath> seen;
    const HANDLE process = GetCurrentProcess();
    std::vector<HMODULE> modules(128);
    DWORD needed = 0;
    size_t count = 0;
    // The module set can change between enumeration calls. Never trust the
    // returned required byte count as the number actually written to a buffer.
    for (unsigned attempt = 0; attempt < 4; ++attempt) {
        if (!EnumProcessModules(process, modules.data(),
                static_cast<DWORD>(modules.size() * sizeof(HMODULE)), &needed)) return;
        count = needed / sizeof(HMODULE);
        if (count <= modules.size()) break;
        if (modules.size() == MAX_MODULES || attempt == 3) {
            count = modules.size();
            break;
        }
        const size_t capacity = count < MAX_MODULES ? count : MAX_MODULES;
        modules.resize(capacity);
        count = 0; // A newly resized buffer has not been filled yet.
    }
    if (count > modules.size()) count = modules.size();
    std::vector<wchar_t> path(32768);
    for (size_t i = 0; i < count; ++i) {
        const DWORD length = GetModuleFileNameExW(process, modules[i], path.data(),
                                                 static_cast<DWORD>(path.size()));
        if (!length || length >= path.size()) continue;
        addPath(ModulePath(path.data(), length), seen);
    }
}

static bool openFile(const ModulePath& path) {
    // Engines without a long-path manifest can still load a DLL by an
    // extended path. Keep that file readable without converting to ANSI.
    ModulePath filePath = path;
    if (filePath.size() >= MAX_PATH && filePath.compare(0, 4, L"\\\\?\\") != 0) {
        if (filePath.compare(0, 2, L"\\\\") == 0) filePath = L"\\\\?\\UNC\\" + filePath.substr(2);
        else if (filePath.size() > 2 && filePath[1] == L':') filePath = L"\\\\?\\" + filePath;
    }
    activeFile = CreateFileW(filePath.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (!fileOpen()) return false;
    if (GetFileType(activeFile) != FILE_TYPE_DISK ||
        !GetFileInformationByHandle(activeFile, &initialFileInfo) ||
        (initialFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        closeFile();
        return false;
    }
    expectedBytes = (uint64_t(initialFileInfo.nFileSizeHigh) << 32) |
                   initialFileInfo.nFileSizeLow;
    const size_t separator = path.find_last_of(L"\\/");
    const std::wstring basename = path.substr(separator == ModulePath::npos ? 0 : separator + 1);
    const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        basename.c_str(), static_cast<int>(basename.size()), NULL, 0, NULL, NULL);
    if (length > 0) {
        activeBasename.resize(static_cast<size_t>(length));
        if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, basename.c_str(),
            static_cast<int>(basename.size()), &activeBasename[0], length, NULL, NULL)) {
            activeBasename.clear();
        }
    }
    return true;
}

static bool unchangedFile() {
    BY_HANDLE_FILE_INFORMATION current;
    return GetFileInformationByHandle(activeFile, &current) &&
        current.dwVolumeSerialNumber == initialFileInfo.dwVolumeSerialNumber &&
        current.nFileIndexHigh == initialFileInfo.nFileIndexHigh &&
        current.nFileIndexLow == initialFileInfo.nFileIndexLow &&
        current.nFileSizeHigh == initialFileInfo.nFileSizeHigh &&
        current.nFileSizeLow == initialFileInfo.nFileSizeLow &&
        CompareFileTime(&current.ftLastWriteTime, &initialFileInfo.ftLastWriteTime) == 0;
}

#elif !defined(__EMSCRIPTEN__)

#if defined(__FreeBSD__) || defined(__NetBSD__)
static int discoverImage(struct dl_phdr_info* info, size_t, void* opaque) {
    if (info && info->dlpi_name && info->dlpi_name[0]) {
        addPath(info->dlpi_name, *static_cast<std::set<ModulePath>*>(opaque));
    }
    return modulePaths.size() >= MAX_MODULES ? 1 : 0;
}
#endif

static void discoverModules() {
    std::set<ModulePath> seen;
#if defined(__APPLE__)
    const uint32_t total = _dyld_image_count();
    for (uint32_t i = 0; i < total && modulePaths.size() < MAX_MODULES; ++i) {
        const char* path = _dyld_get_image_name(i);
        if (path) addPath(path, seen);
    }
#elif defined(__linux__) || defined(__ANDROID__)
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return;
    char line[8192];
    bool skipRemainder = false;
    while (modulePaths.size() < MAX_MODULES && fgets(line, sizeof(line), maps)) {
        const size_t length = strlen(line);
        const bool complete = length && line[length - 1] == '\n';
        if (skipRemainder || !complete) {
            skipRemainder = !complete;
            continue;
        }
        char permissions[5] = {};
        int pathOffset = 0;
        // Consume the five fixed columns, then keep the ENTIRE path (spaces
        // are legal). Anonymous/special mappings do not have a module file.
        if (sscanf(line, "%*s %4s %*s %*s %*s %n", permissions, &pathOffset) != 1 ||
            pathOffset <= 0 || !strchr(permissions, 'x') || line[pathOffset] != '/') continue;
        line[length - 1] = '\0';
        std::string path(line + pathOffset);
        // A deleted/replaced mapping cannot safely be attributed to the file
        // now present at its old path. Never report that replacement's hash.
        const std::string deleted = " (deleted)";
        if (path.size() >= deleted.size() &&
            path.compare(path.size() - deleted.size(), deleted.size(), deleted) == 0) continue;
        addPath(path, seen);
    }
    fclose(maps);
#elif defined(__FreeBSD__) || defined(__NetBSD__)
    dl_iterate_phdr(discoverImage, &seen);
#endif
}

static bool openFile(const ModulePath& path) {
    activeFile = fopen(path.c_str(), "rb");
    if (!fileOpen()) return false;
    if (fstat(fileno(activeFile), &initialFileInfo) != 0 ||
        !S_ISREG(initialFileInfo.st_mode) || initialFileInfo.st_size < 0) {
        closeFile();
        return false;
    }
    expectedBytes = static_cast<uint64_t>(initialFileInfo.st_size);
    const size_t separator = path.find_last_of('/');
    activeBasename = path.substr(separator == ModulePath::npos ? 0 : separator + 1);
    return true;
}

static bool unchangedFile() {
    struct stat current;
    if (fstat(fileno(activeFile), &current) != 0 ||
        current.st_dev != initialFileInfo.st_dev || current.st_ino != initialFileInfo.st_ino ||
        current.st_size != initialFileInfo.st_size) return false;
#if defined(__APPLE__)
    return current.st_mtimespec.tv_sec == initialFileInfo.st_mtimespec.tv_sec &&
        current.st_mtimespec.tv_nsec == initialFileInfo.st_mtimespec.tv_nsec &&
        current.st_ctimespec.tv_sec == initialFileInfo.st_ctimespec.tv_sec &&
        current.st_ctimespec.tv_nsec == initialFileInfo.st_ctimespec.tv_nsec;
#else
    return current.st_mtim.tv_sec == initialFileInfo.st_mtim.tv_sec &&
        current.st_mtim.tv_nsec == initialFileInfo.st_mtim.tv_nsec &&
        current.st_ctim.tv_sec == initialFileInfo.st_ctim.tv_sec &&
        current.st_ctim.tv_nsec == initialFileInfo.st_ctim.tv_nsec;
#endif
}
#endif

static void finishFile() {
#if !defined(__EMSCRIPTEN__)
    if (readBytes == expectedBytes && unchangedFile() && !activeBasename.empty()) {
        ModuleReport report;
        uint8_t digest[20];
        md5.finalize(digest);
        xm_md5::toHex(digest, report.md5);
        // Keep the existing checksum algorithm for legacy server rules.
        sha1.finalize(digest);
        xm_sha1::toHex(digest, 20, report.sha1);
        size_t length = activeBasename.size();
        if (length > MAX_BASENAME_BYTES) {
            length = MAX_BASENAME_BYTES;
            // Avoid cutting a UTF-8 code point in half in the display name.
            while (length && (static_cast<unsigned char>(activeBasename[length]) & 0xc0) == 0x80) --length;
        }
        // POSIX allows names containing controls, ':' or '\\'. Keep these
        // informational names within the server's safe-basename protocol so
        // renaming a module cannot make its otherwise valid MD5 report fail.
        for (size_t i = 0; i < length; ++i) {
            const unsigned char c = static_cast<unsigned char>(activeBasename[i]);
            if (c < 32 || c == 127 || c == '/' || c == '\\' || c == ':') activeBasename[i] = '_';
        }
        xm_sha1::toHex(reinterpret_cast<const uint8_t*>(activeBasename.data()),
                       length, report.basenameHex);
        readyReports.push_back(report);
    }
#endif
    closeFile();
}

} // namespace

void scanAndSendModules() {
    if (transferActive) return;
    clearModuleQueue();
#if !defined(__EMSCRIPTEN__)
    discoverModules();
    transferActive = true;
#endif
    // Browser clients cannot inspect native DLLs. Do not claim a native scan
    // completed successfully or fabricate an empty native module report.
}

void clearModuleQueue() {
    closeFile();
    modulePaths.clear();
    nextModule = 0;
    readyReports.clear();
    sentReports = 0;
    transferActive = false;
    md5.reset();
    sha1.reset();
}

void processModuleScan() {
#if !defined(__EMSCRIPTEN__)
    if (!transferActive || readyReports.size() >= MAX_READY_REPORTS) return;
    size_t budget = BYTES_PER_FRAME;
    unsigned filesOpened = 0;
    const uint32_t start = static_cast<uint32_t>(trap_Milliseconds());
    uint8_t buffer[READ_CHUNK_SIZE];
    while (budget && readyReports.size() < MAX_READY_REPORTS) {
        if (!fileOpen()) {
            // Limit failures/empty files as well as bytes: many unreadable
            // entries must not all be opened in a single rendered frame.
            if (nextModule >= modulePaths.size() || filesOpened++ >= 4) break;
            const ModulePath& path = modulePaths[nextModule++];
            if (!openFile(path)) continue;
            md5.reset();
            sha1.reset();
            readBytes = 0;
        }
        if (readBytes == expectedBytes) {
            finishFile();
            continue;
        }
        const uint64_t remaining = expectedBytes - readBytes;
        size_t count = remaining < sizeof(buffer) ? static_cast<size_t>(remaining) : sizeof(buffer);
        if (count > budget) count = budget;
#ifdef _WIN32
        DWORD received = 0;
        const bool valid = ReadFile(activeFile, buffer, static_cast<DWORD>(count), &received, NULL) &&
                           received == count;
#else
        const bool valid = fread(buffer, 1, count, activeFile) == count && !ferror(activeFile);
#endif
        budget -= count;
        if (!valid) {
            closeFile();
        } else {
            md5.update(buffer, count);
            sha1.update(buffer, count);
            readBytes += count;
            if (readBytes == expectedBytes) finishFile();
        }
        if (static_cast<uint32_t>(trap_Milliseconds()) - start >= 4) break;
    }
#endif
}

bool processModuleQueue() {
    if (!transferActive) return false;
    if (!readyReports.empty()) {
        const ModuleReport& report = readyReports.front();
        trap_SendClientCommand(va("jxac_module_md5 %s %s %s",
            report.md5, report.sha1, report.basenameHex));
        readyReports.pop_front();
        ++sentReports;
        return true;
    }
    if (nextModule == modulePaths.size() && !fileOpen()) {
        trap_SendClientCommand(va("jxac_module_complete %u", sentReports));
        clearModuleQueue();
        return true;
    }
    return false;
}

} // namespace jxac
