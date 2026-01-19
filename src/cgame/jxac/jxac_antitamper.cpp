#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <cgame/jxac/jxac_antitamper.h>
#include <cstring>
#include <cstdlib>

// Simple case-insensitive substring search (local helper)
static const char* Q_stristr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return nullptr;
    size_t needleLen = strlen(needle);
    size_t haystackLen = strlen(haystack);
    if (needleLen > haystackLen) return nullptr;
    
    for (size_t i = 0; i <= haystackLen - needleLen; i++) {
        if (Q_stricmpn(haystack + i, needle, needleLen) == 0) {
            return haystack + i;
        }
    }
    return nullptr;
}

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <sys/ptrace.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

// macOS uses different ptrace constants than Linux
#ifdef __APPLE__
#ifndef PTRACE_TRACEME
#define PTRACE_TRACEME PT_TRACE_ME
#endif
#ifndef PTRACE_DETACH
#define PTRACE_DETACH PT_DETACH
#endif
#endif
#endif

namespace jxac {

// Anti-tamper constants
#define JXAC_ANTITAMPER_CHECK_INTERVAL 30000 // Check every 30 seconds

// Static state
static qboolean initialized = qfalse;
static int lastCheckTime = 0;

// Known tamper tool process names (lowercase)
static const char* tamperTools[] = {
    "cheatengine",
    "ollydbg",
    "x64dbg",
    "x32dbg",
    "ida",
    "ida64",
    "windbg",
    "processhacker",
    "procexp",
    "wireshark",
    "fiddler",
    NULL
};

///////////////////////////////////////////////////////////////////////////////

void AntiTamper::init() {
    if ( initialized ) {
        return;
    }
    
    Com_Printf( "JXAC: Initializing anti-tamper system\n" );
    
    initialized = qtrue;
    lastCheckTime = cg.time;
    
    // Perform initial check
    check();
    
    Com_Printf( "JXAC: Anti-tamper system initialized\n" );
}

///////////////////////////////////////////////////////////////////////////////

void AntiTamper::check() {
    if ( !initialized ) {
        return;
    }
    
    // Only check periodically
    if ( cg.time - lastCheckTime < JXAC_ANTITAMPER_CHECK_INTERVAL ) {
        return;
    }
    
    lastCheckTime = cg.time;
    
    // Check for debugger
    if ( checkDebugger() ) {
        reportTamper( "Debugger detected" );
        return;
    }
    
    // Check for tamper tools
    if ( checkTamperTools() ) {
        // Reported by checkTamperTools
        return;
    }
    
    // Check code integrity
    if ( !checkCodeIntegrity() ) {
        reportTamper( "Code integrity check failed" );
        return;
    }
    
    // Check for function hooks
    if ( !checkFunctionHooks() ) {
        reportTamper( "Function hook detected" );
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AntiTamper::reportTamper( const char* details ) {
    Com_Printf( "^1JXAC: TAMPER DETECTED: %s\n", details );
    
    // Send violation to server
    trap_SendClientCommand( va( "jxac_violation tamper %s", details ) );
}

///////////////////////////////////////////////////////////////////////////////

bool AntiTamper::checkDebugger() {
#ifdef _WIN32
    // Windows: Use IsDebuggerPresent API
    if ( IsDebuggerPresent() ) {
        return true;
    }
    
    // Additional check: CheckRemoteDebuggerPresent
    BOOL debuggerPresent = FALSE;
    CheckRemoteDebuggerPresent( GetCurrentProcess(), &debuggerPresent );
    if ( debuggerPresent ) {
        return true;
    }
    
    return false;
#else
    // Linux: Use ptrace trick
    // If we can ptrace ourselves, no debugger is attached
    // If ptrace fails with EPERM, a debugger is likely attached
    static bool ptraceChecked = false;
    static bool debuggerDetected = false;
    
    if ( !ptraceChecked ) {
        if ( ptrace( PTRACE_TRACEME, 0, 1, 0 ) == -1 ) {
            debuggerDetected = true;
        } else {
            // Detach immediately
            ptrace( PTRACE_DETACH, 0, 1, 0 );
        }
        ptraceChecked = true;
    }
    
    return debuggerDetected;
#endif
}

///////////////////////////////////////////////////////////////////////////////

bool AntiTamper::checkTamperTools() {
#ifdef _WIN32
    // Windows: Enumerate processes and check names
    HANDLE snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if ( snapshot == INVALID_HANDLE_VALUE ) {
        return false;
    }
    
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof( entry );
    
    if ( Process32First( snapshot, &entry ) ) {
        do {
            // Convert to lowercase for comparison
            char exeName[MAX_PATH];
            Q_strncpyz( exeName, entry.szExeFile, sizeof( exeName ) );
            Q_strlwr( exeName );
            
            // Check against known tamper tools
            for ( int i = 0; tamperTools[i] != NULL; i++ ) {
                if ( Q_stristr( exeName, tamperTools[i] ) ) {
                    CloseHandle( snapshot );
                    reportTamper( va( "Tamper tool detected: %s", entry.szExeFile ) );
                    return true;
                }
            }
        } while ( Process32Next( snapshot, &entry ) );
    }
    
    CloseHandle( snapshot );
    return false;
#else
    // Linux: Check /proc for suspicious processes
    // This is a simplified check - a full implementation would scan /proc
    FILE* f = fopen( "/proc/self/status", "r" );
    if ( !f ) {
        return false;
    }
    
    char line[256];
    while ( fgets( line, sizeof( line ), f ) ) {
        // Check for TracerPid (debugger)
        if ( strncmp( line, "TracerPid:", 10 ) == 0 ) {
            int pid = atoi( line + 10 );
            if ( pid != 0 ) {
                fclose( f );
                reportTamper( "Tracer process detected" );
                return true;
            }
        }
    }
    
    fclose( f );
    return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////

bool AntiTamper::checkCodeIntegrity() {
    // Simple integrity check: verify some known function pointers haven't been modified
    // In a real implementation, you'd calculate checksums of critical code sections
    
    // Check if trap functions are in expected memory regions
    // This is a basic check - a sophisticated attacker could bypass this
    
    // For now, always return true (no tampering detected)
    // A full implementation would:
    // 1. Store checksums of critical functions at init
    // 2. Periodically recalculate and compare
    // 3. Use obfuscation to make this check harder to bypass
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AntiTamper::checkFunctionHooks() {
    // Check if critical functions have been hooked
    // This is a simplified check - a full implementation would:
    // 1. Check first bytes of critical functions for JMP instructions
    // 2. Verify IAT (Import Address Table) hasn't been modified
    // 3. Check for inline hooks
    
    // For now, always return true (no hooks detected)
    // A real implementation would examine the first bytes of functions like:
    // - trap_SendClientCommand
    // - trap_R_ReadPixels
    // - malloc/free
    // And verify they haven't been patched with JMP instructions
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
