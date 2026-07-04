#include <bgame/impl.h>
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////////

namespace {

static LPTOP_LEVEL_EXCEPTION_FILTER savedExceptionFilter = NULL;

static LONG WINAPI CrashExceptionFilter( EXCEPTION_POINTERS* exInfo )
{
    FILE* f = fopen( "xmod_crash.log", "a" );
    if (f) {
        time_t now = time( 0 );
        char fnow[32];
        strftime( fnow, sizeof(fnow), "%Y-%m-%d %H:%M:%S", localtime( &now ));

        fprintf( f, "=== XMOD CRASH LOG ===\n" );
        fprintf( f, "Timestamp: %s\n", fnow );
        fprintf( f, "Exception Code: 0x%08lX\n", exInfo->ExceptionRecord->ExceptionCode );
        fprintf( f, "Exception Address: 0x%p\n", exInfo->ExceptionRecord->ExceptionAddress );
        fprintf( f, "Exception Flags: 0x%08lX\n", exInfo->ExceptionRecord->ExceptionFlags );

#if defined(_WIN64) || defined(__x86_64__) || defined(__amd64__)
        fprintf( f, "RIP: 0x%016llX\n", (unsigned long long)exInfo->ContextRecord->Rip );
        fprintf( f, "RSP: 0x%016llX\n", (unsigned long long)exInfo->ContextRecord->Rsp );
        fprintf( f, "RBP: 0x%016llX\n", (unsigned long long)exInfo->ContextRecord->Rbp );
#else
        fprintf( f, "EIP: 0x%08lX\n", (unsigned long)exInfo->ContextRecord->Eip );
        fprintf( f, "ESP: 0x%08lX\n", (unsigned long)exInfo->ContextRecord->Esp );
        fprintf( f, "EBP: 0x%08lX\n", (unsigned long)exInfo->ContextRecord->Ebp );
#endif

        fprintf( f, "=== END CRASH LOG ===\n\n" );
        fclose( f );
    }

    if (savedExceptionFilter)
        return savedExceptionFilter( exInfo );

    return EXCEPTION_CONTINUE_SEARCH;
}

} // namespace

//////////////////////////////////////////////////////////////////////////////

void
Process::beginCriticalSection()
{
}

//////////////////////////////////////////////////////////////////////////////

void
Process::endCriticalSection()
{
}

//////////////////////////////////////////////////////////////////////////////

void
Process::signalInit()
{
    savedExceptionFilter = SetUnhandledExceptionFilter( CrashExceptionFilter );
}

//////////////////////////////////////////////////////////////////////////////

void
Process::signalShutdown()
{
    SetUnhandledExceptionFilter( savedExceptionFilter );
    savedExceptionFilter = NULL;
}

//////////////////////////////////////////////////////////////////////////////

Process::mstime_t
Process::mstime()
{
    FILETIME ftime;
    GetSystemTimeAsFileTime( &ftime );

    ULARGE_INTEGER tmp;
    memcpy( &tmp, &ftime, sizeof(tmp) );

    return tmp.QuadPart / 10000;
}
