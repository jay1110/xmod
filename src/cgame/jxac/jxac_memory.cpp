#include <bgame/impl.h>
#include <cgame/jxac/jxac_memory.h>
#define XMOD_JXAC_MEMORY_MATCHER
#include <bgame/jxac_memory_rules.h>
#undef XMOD_JXAC_MEMORY_MATCHER

#include <cstddef>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace jxac {
namespace {

const uint32_t SCAN_INTERVAL_MS = 30000;
const size_t BYTES_PER_FRAME = 256 * 1024;
const size_t READ_CHUNK = 16 * 1024;
const unsigned QUERIES_PER_FRAME = 64;
const unsigned MAX_RANGES = 256;
const unsigned MAX_PATTERNS = 8;
const char* pendingStatus = NULL;
const char* pendingHit = NULL;
bool hitSent = false;
bool started = false;
bool running = false;
uint32_t finishedAt = 0;
size_t frameBytes = 0;
unsigned frameQueries = 0;

#ifdef _WIN32
struct Range {
    uintptr_t start;
    uintptr_t end;
    bool code;
};

uintptr_t address = 0;
uintptr_t addressLimit = 0;
uintptr_t ownAllocation = 0;
uintptr_t allocation = 0;
uintptr_t executableAddress = 0;
uintptr_t readOnlyCodeAddress = 0;
uintptr_t writableDataAddress = 0;
bool depDisabled = false;
Range ranges[MAX_RANGES];
unsigned rangeCount = 0;
unsigned rangeIndex = 0;
uintptr_t readAddress = 0;
bool scanningAllocation = false;
bool allocationOverflow = false;
bool incomplete = false;
size_t carry = 0;
unsigned char* scratch = NULL;
uintptr_t matches[memoryrules::RULE_COUNT][MAX_PATTERNS];
int verifyRule = -1;

bool readable(DWORD protect) {
    if (protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    switch (protect & 0xff) {
        case PAGE_READONLY: case PAGE_READWRITE: case PAGE_WRITECOPY:
        case PAGE_EXECUTE_READ: case PAGE_EXECUTE_READWRITE: case PAGE_EXECUTE_WRITECOPY:
            return true;
        default: return false;
    }
}

bool executable(DWORD protect) {
    if (protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    switch (protect & 0xff) {
        case PAGE_EXECUTE: case PAGE_EXECUTE_READ: case PAGE_EXECUTE_READWRITE:
        case PAGE_EXECUTE_WRITECOPY: return true;
        default: return false;
    }
}

bool processDepDisabled() {
#if defined(_M_IX86) || defined(__i386__)
    DWORD flags = 0;
    BOOL permanent = FALSE;
    return GetProcessDEPPolicy(GetCurrentProcess(), &flags, &permanent) &&
        !(flags & PROCESS_DEP_ENABLE);
#else
    return false; // DEP cannot be disabled for a native 64-bit process.
#endif
}

bool privateReadOnlyCode(const MEMORY_BASIC_INFORMATION& info) {
    return depDisabled && info.Type == MEM_PRIVATE && info.Protect == PAGE_READONLY;
}

bool privateWritableData(const MEMORY_BASIC_INFORMATION& info) {
    return info.Type == MEM_PRIVATE && info.Protect == PAGE_READWRITE;
}

bool eligibleAllocation() {
    return executableAddress || (depDisabled && readOnlyCodeAddress && writableDataAddress);
}

bool query(uintptr_t at, MEMORY_BASIC_INFORMATION& result) {
    ++frameQueries;
    return VirtualQuery(reinterpret_cast<const void*>(at), &result, sizeof(result)) == sizeof(result);
}

bool readMemory(uintptr_t at, unsigned char* output, size_t bytes) {
    SIZE_T received = 0;
    frameBytes += bytes;
    return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(at),
        output, bytes, &received) && received == bytes;
}

void resetAllocation() {
    allocation = executableAddress = readAddress = 0;
    readOnlyCodeAddress = writableDataAddress = 0;
    rangeCount = rangeIndex = 0;
    carry = 0;
    allocationOverflow = scanningAllocation = false;
    verifyRule = -1;
    std::memset(matches, 0, sizeof(matches));
}

void finishPass(uint32_t now) {
    pendingStatus = incomplete ? "partial" : "complete";
    running = false;
    finishedAt = now;
    resetAllocation();
}

bool beginPass() {
    SYSTEM_INFO system;
    GetSystemInfo(&system);
    address = reinterpret_cast<uintptr_t>(system.lpMinimumApplicationAddress);
    const uintptr_t maximum = reinterpret_cast<uintptr_t>(system.lpMaximumApplicationAddress);
    addressLimit = maximum == UINTPTR_MAX ? maximum : maximum + 1;
    HMODULE module = NULL;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&processMemoryScan), &module)) return false;
    ownAllocation = reinterpret_cast<uintptr_t>(module);
    if (!scratch) {
        scratch = static_cast<unsigned char*>(VirtualAlloc(NULL,
            READ_CHUNK + memoryrules::MAX_PATTERN_BYTES, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if (!scratch) return false;
    }
    resetAllocation();
    depDisabled = processDepDisabled();
    incomplete = false;
    running = true;
    pendingStatus = "scanning";
    return true;
}

// Confirm all evidence is still present in the same allocation immediately
// before reporting. Enumeration and reads can race DLL unload/protection changes.
bool verifyFingerprint(unsigned ruleIndex) {
    const memoryrules::Rule& rule = memoryrules::rules[ruleIndex];
    if (frameQueries + rule.count + 2 > QUERIES_PER_FRAME ||
        frameBytes + (rule.count + 1) * memoryrules::MAX_PATTERN_BYTES > BYTES_PER_FRAME) return false;
    // A DEP-disabled x86 manual map can execute read-only private pages.
    // Require a separate writable data range as well; never treat an ordinary
    // writable heap/file buffer as code. Recheck the OS policy and both ranges.
    depDisabled = processDepDisabled();
    MEMORY_BASIC_INFORMATION info;
    const bool privateDataValid = depDisabled && writableDataAddress &&
        query(writableDataAddress, info) && info.State == MEM_COMMIT &&
        reinterpret_cast<uintptr_t>(info.AllocationBase) == allocation && privateWritableData(info);
    const uintptr_t codeAddress = executableAddress ? executableAddress : readOnlyCodeAddress;
    if (!codeAddress || !query(codeAddress, info) || info.State != MEM_COMMIT ||
        reinterpret_cast<uintptr_t>(info.AllocationBase) != allocation ||
        (!executable(info.Protect) && !(privateDataValid && privateReadOnlyCode(info)))) {
        incomplete = true;
        verifyRule = -1;
        carry = 0;
        return false;
    }
    for (size_t p = 0; p < rule.count; ++p) {
        const memoryrules::Pattern& pattern = rule.patterns[p];
        const uintptr_t at = matches[ruleIndex][p];
        if (!at || !query(at, info) || info.State != MEM_COMMIT ||
            reinterpret_cast<uintptr_t>(info.AllocationBase) != allocation || !readable(info.Protect) ||
            (pattern.executable && !executable(info.Protect) &&
                !(privateDataValid && privateReadOnlyCode(info))) ||
            info.RegionSize > UINTPTR_MAX - reinterpret_cast<uintptr_t>(info.BaseAddress) ||
            at > reinterpret_cast<uintptr_t>(info.BaseAddress) + info.RegionSize ||
            pattern.size > reinterpret_cast<uintptr_t>(info.BaseAddress) + info.RegionSize - at ||
            !readMemory(at, scratch, pattern.size) ||
            std::memcmp(scratch, pattern.bytes, pattern.size) != 0) {
            matches[ruleIndex][p] = 0;
            verifyRule = -1;
            incomplete = true;
            carry = 0;
            return false;
        }
    }
    pendingHit = rule.id;
    pendingStatus = NULL;
    running = false;
    return true;
}

void inspectChunk(uintptr_t start, size_t bytes, bool hasCode) {
    for (size_t r = 0; r < memoryrules::RULE_COUNT; ++r) {
        const memoryrules::Rule& rule = memoryrules::rules[r];
        if (!rule.count || rule.count > MAX_PATTERNS) continue;
        bool complete = true;
        for (size_t p = 0; p < rule.count; ++p) {
            const memoryrules::Pattern& pattern = rule.patterns[p];
            if (!pattern.size || pattern.size > memoryrules::MAX_PATTERN_BYTES ||
                (pattern.executable && !hasCode)) {
                if (!matches[r][p]) complete = false;
                continue;
            }
            if (!matches[r][p] && bytes >= pattern.size) {
                const unsigned char* candidate = scratch;
                const unsigned char* last = scratch + bytes - pattern.size;
                while (candidate <= last) {
                    const void* found = std::memchr(candidate, pattern.bytes[0], size_t(last - candidate) + 1);
                    if (!found) break;
                    candidate = static_cast<const unsigned char*>(found);
                    if (std::memcmp(candidate, pattern.bytes, pattern.size) == 0) {
                        matches[r][p] = start + size_t(candidate - scratch);
                        break;
                    }
                    ++candidate;
                }
            }
            if (!matches[r][p]) complete = false;
        }
        if (complete) {
            verifyRule = static_cast<int>(r);
            return;
        }
    }
}
#endif // _WIN32

} // namespace

void clearMemoryScan() {
#ifdef _WIN32
    if (scratch) VirtualFree(scratch, 0, MEM_RELEASE);
    scratch = NULL;
    resetAllocation();
    address = addressLimit = ownAllocation = 0;
    depDisabled = false;
    incomplete = false;
#endif
    pendingStatus = pendingHit = NULL;
    hitSent = started = running = false;
    finishedAt = 0;
    frameBytes = 0;
    frameQueries = 0;
}

void processMemoryScan() {
    if (pendingHit || hitSent) return;
#ifndef _WIN32
    if (!started) {
        started = true;
        pendingStatus = "unsupported";
    }
#else
    const uint32_t now = static_cast<uint32_t>(trap_Milliseconds());
    frameBytes = 0;
    frameQueries = 0;
    if (!running) {
        if (started && now - finishedAt < SCAN_INTERVAL_MS) return;
        started = true;
        if (!beginPass()) {
            incomplete = true;
            finishPass(now);
            return;
        }
    }
    while (frameBytes < BYTES_PER_FRAME && frameQueries < QUERIES_PER_FRAME &&
           static_cast<uint32_t>(trap_Milliseconds()) - now < 2) {
        if (verifyRule >= 0) {
            if (verifyFingerprint(static_cast<unsigned>(verifyRule))) return;
            if (verifyRule >= 0) break; // Budget exhausted; revalidate next frame.
            continue; // Recheck time/byte limits after failed revalidation.
        }
        if (scanningAllocation) {
            if (rangeIndex == rangeCount) {
                resetAllocation();
                continue;
            }
            const Range& range = ranges[rangeIndex];
            if (!readAddress) readAddress = range.start;
            const uintptr_t remaining = range.end - readAddress;
            size_t count = remaining < READ_CHUNK ? static_cast<size_t>(remaining) : READ_CHUNK;
            if (count > BYTES_PER_FRAME - frameBytes) count = BYTES_PER_FRAME - frameBytes;
            if (readMemory(readAddress, scratch + carry, count)) {
                const size_t bytes = carry + count;
                inspectChunk(readAddress - carry, bytes, range.code);
                if (verifyRule < 0) {
                    carry = bytes < memoryrules::MAX_PATTERN_BYTES - 1 ? bytes : memoryrules::MAX_PATTERN_BYTES - 1;
                    std::memmove(scratch, scratch + bytes - carry, carry);
                }
            } else {
                incomplete = true;
                carry = 0;
            }
            readAddress += count;
            if (readAddress == range.end) {
                ++rangeIndex;
                readAddress = 0;
                carry = 0; // Never bridge a guard, unreadable or changed-protection range.
            }
            continue;
        }
        if (address >= addressLimit) {
            if (allocation && eligibleAllocation() && rangeCount && !allocationOverflow) {
                scanningAllocation = true;
                continue;
            }
            finishPass(static_cast<uint32_t>(trap_Milliseconds()));
            return;
        }
        MEMORY_BASIC_INFORMATION info;
        if (!query(address, info) || info.RegionSize == 0) {
            incomplete = true;
            finishPass(static_cast<uint32_t>(trap_Milliseconds()));
            return;
        }
        const uintptr_t base = reinterpret_cast<uintptr_t>(info.BaseAddress);
        const uintptr_t owner = reinterpret_cast<uintptr_t>(info.AllocationBase);
        if (allocation && owner != allocation) {
            if (eligibleAllocation() && rangeCount && !allocationOverflow) {
                scanningAllocation = true;
                continue; // Inspect the previous allocation; query this address again afterwards.
            }
            resetAllocation();
        }
        if (info.RegionSize > UINTPTR_MAX - base || base + info.RegionSize <= address) {
            incomplete = true;
            finishPass(static_cast<uint32_t>(trap_Milliseconds()));
            return;
        }
        const uintptr_t end = base + info.RegionSize;
        address = end < addressLimit ? end : addressLimit;
        if (!owner || owner == ownAllocation || owner == reinterpret_cast<uintptr_t>(scratch)) continue;
        if (!allocation) allocation = owner;
        if (info.State != MEM_COMMIT) continue;
        if (executable(info.Protect)) executableAddress = base;
        if (privateReadOnlyCode(info)) readOnlyCodeAddress = base;
        if (depDisabled && privateWritableData(info)) writableDataAddress = base;
        if (readable(info.Protect)) {
            if (rangeCount < MAX_RANGES) ranges[rangeCount++] = Range{base, end,
                executable(info.Protect) || privateReadOnlyCode(info)};
            else {
                allocationOverflow = true;
                incomplete = true;
            }
        }
    }
#endif
}

bool processMemoryHitQueue() {
    if (pendingHit && !hitSent) {
        trap_SendClientCommand(va("jxac_memory_hit %s", pendingHit));
        hitSent = true;
        return true;
    }
    return false;
}

bool processMemoryQueue() {
    if (processMemoryHitQueue()) return true;
    if (pendingStatus) {
        trap_SendClientCommand(va("jxac_memory_status %s", pendingStatus));
        pendingStatus = NULL;
        return true;
    }
    return false;
}

} // namespace jxac
