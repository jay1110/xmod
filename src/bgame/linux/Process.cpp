#if defined( CGAMEDLL )
#    include <cgame/cg_local.h>
#elif defined( GAMEDLL )
#    include <game/g_local.h>
#elif defined( UIDLL )
#    include <ui/ui_local.h>
#else
#    error "DLL-MODULE is not defined."
#endif

//////////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// Emscripten/WebAssembly does not provide backtrace()/execinfo.h, machine
// register contexts (ucontext gregs), or meaningful process signals in the
// browser sandbox, so the native crash-handling and signal machinery below is
// compiled only for real POSIX targets. Similarly, Android (Bionic libc) does
// not provide execinfo.h/backtrace(). Platform-specific no-op implementations
// of the public Process methods are provided further down.
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)

#ifndef __USE_GNU
#   define __USE_GNU
#   include <sys/ucontext.h>
#   undef __USE_GNU
#else
#   include <sys/ucontext.h>
#endif

#include <signal.h>
#include <errno.h>
#if defined(__GLIBC__)
#include <execinfo.h>
#endif
#include <fcntl.h>

namespace {

//////////////////////////////////////////////////////////////////////////////

struct SigData
{
    int num;
    void (*handler)( int, siginfo_t*, void* );
    unsigned int flags;
    struct sigaction savedAction;
};

sigset_t sigSavedMask;

//////////////////////////////////////////////////////////////////////////////

void
coreTrace( int num, siginfo_t* info, ucontext_t* context )
{
#if defined(__GLIBC__)
    int    i;
    void*  array[1024];
    char** elements;
    int    size;

    size = backtrace( array, sizeof(array) / sizeof(void*) );
    printf( "STACK ELEMENTS: %d\n", size-1 );

    // This code segment taken from etpub.
    // Set the actual calling address for accurate stack traces.
    // If we don't do this stack traces are less accurate.
#if defined(__x86_64__) || defined(__amd64__)
    array[1] = (void*)context->uc_mcontext.gregs[REG_RIP];
#elif defined(__aarch64__)
    array[1] = (void*)context->uc_mcontext.pc;
#elif defined(__arm__)
    array[1] = (void*)(uintptr_t)context->uc_mcontext.arm_pc;
#else
    array[1] = (void*)context->uc_mcontext.gregs[REG_EIP];
#endif

    elements = backtrace_symbols(array, size);

    for (i = 1; i < size; i++)
        printf( "[%02d] %s\n", i, elements[i] );
#else
    // musl (for example Alpine) has no glibc execinfo API. Keep native signal
    // handling and the crash report; a core dump can provide the stack trace.
    printf( "STACK TRACE: unavailable without execinfo support\n" );
#endif
}

//////////////////////////////////////////////////////////////////////////////

void
writeCrashLog( int num, siginfo_t* info, ucontext_t* context )
{
    // Use async-signal-safe I/O to write crash log
    int fd = open( "xmod_crash.log", O_WRONLY | O_CREAT | O_APPEND, 0644 );
    if (fd < 0)
        return;

    time_t now = time( 0 );
    char fnow[32];
    struct tm tm_buf;
    gmtime_r( &now, &tm_buf );
    strftime( fnow, sizeof(fnow), "%Y-%m-%d %H:%M:%S", &tm_buf );

    char buf[8192];
    int len = snprintf( buf, sizeof(buf),
        "=== XMOD CRASH LOG ===\n"
        "Timestamp: %s\n"
        "Signal: %d\n"
        "si_errno: %d\n"
        "si_code: %d\n"
        "si_pid: %d\n"
        "si_uid: %d\n",
        fnow, num,
        (int)info->si_errno, (int)info->si_code,
        (int)info->si_pid, (int)info->si_uid );
    write( fd, buf, len );

    // glibc provides execinfo; musl builds still record the signal metadata.
#if defined(__GLIBC__)
    void*  array[1024];
    int    size;
    size = backtrace( array, sizeof(array) / sizeof(void*) );

#if defined(__x86_64__) || defined(__amd64__)
    array[1] = (void*)context->uc_mcontext.gregs[REG_RIP];
#elif defined(__aarch64__)
    array[1] = (void*)context->uc_mcontext.pc;
#elif defined(__arm__)
    array[1] = (void*)(uintptr_t)context->uc_mcontext.arm_pc;
#else
    array[1] = (void*)context->uc_mcontext.gregs[REG_EIP];
#endif

    len = snprintf( buf, sizeof(buf), "Stack frames: %d\n", size - 1 );
    write( fd, buf, len );

    // backtrace_symbols_fd is async-signal-safe
    backtrace_symbols_fd( array + 1, size - 1, fd );
#else
    const char noTrace[] = "Stack trace: unavailable without execinfo support\n";
    write( fd, noTrace, sizeof(noTrace) - 1 );
#endif

    len = snprintf( buf, sizeof(buf), "=== END CRASH LOG ===\n\n" );
    write( fd, buf, len );

    close( fd );
}

//////////////////////////////////////////////////////////////////////////////

void
handlerMsg( int num, siginfo_t* info, const char* name, const char* action )
{
    time_t now = time( 0 );
    char fnow[32];
    strftime( fnow, sizeof(fnow), "%c", localtime( &now ));

    ostringstream msg;
    msg <<   "-------"
        << "\n------- TIMESTAMP: " << fnow
        << "\n------- CAUGHT OS SIGNAL: " << name << " (" << num << ")"
        << "\n-------     si_errno = " << info->si_errno
        << "\n-------     si_code  = " << info->si_code
        << "\n-------     si_pid   = " << info->si_pid
        << "\n-------     si_uid   = " << info->si_uid
        << "\n------- ACTION: " << action
        << "\n-------" 
        << endl;

    trap_Print( msg.str().c_str() );
}

//////////////////////////////////////////////////////////////////////////////

void
handlerCORE( int num, siginfo_t* info, void* context )
{
    printf( "SIGNAL CAUGHT: %d\n", num );
    coreTrace( num, info, (ucontext_t*)context );
    writeCrashLog( num, info, (ucontext_t*)context );

    if (raise( num ))
        printf( "WARNING: unable to raise signal(%d): error #%d\n", num, errno );
}

//////////////////////////////////////////////////////////////////////////////

void
handlerHUP( int num, siginfo_t* info, void* context )
{
    handlerMsg( num, info, "SIGHUP", "queued shutdown" );
    process.setPendingShutdown( true );

    /* We force stdin to close because if CVAR ttycon=1 and the parent
     * process supplying TTY is killed, etded.x86 likes to spin.
     * It is probably not necessary to do this when ttycon=0 but
     * there's no harm in it either since we're shutting down anyways.
     */
    close( fileno( stdin ));
}

//////////////////////////////////////////////////////////////////////////////

void
handlerTERM( int num, siginfo_t* info, void* context )
{
    handlerMsg( num, info, "SIGTERM", "queued shutdown" );
    process.setPendingShutdown( true );

    /* We force stdin to close because if CVAR ttycon=1 and the parent
     * process supplying TTY is killed, etded.x86 likes to spin.
     * It is probably not necessary to do this when ttycon=0 but
     * there's no harm in it either since we're shutting down anyways.
     */
    close( fileno( stdin ));
}

//////////////////////////////////////////////////////////////////////////////

void
handlerUSR1( int num, siginfo_t* info, void* context )
{
    handlerMsg( num, info, "SIGUSR1", "queued reload" );
    process.setPendingReload( true );
}

//////////////////////////////////////////////////////////////////////////////

SigData sigList[] = {
    { SIGHUP,  handlerHUP,  0 },
    { SIGTERM, handlerTERM, 0 },
    { SIGUSR1, handlerUSR1, 0 },
    { SIGINT,  handlerCORE, SA_RESETHAND },
    { SIGQUIT, handlerCORE, SA_RESETHAND },
    { SIGILL,  handlerCORE, SA_RESETHAND },
    { SIGABRT, handlerCORE, SA_RESETHAND },
    { SIGBUS,  handlerCORE, SA_RESETHAND },
    { SIGFPE,  handlerCORE, SA_RESETHAND },
    { SIGSEGV, handlerCORE, SA_RESETHAND },
    { -1 },
};

//////////////////////////////////////////////////////////////////////////////

}

#endif // !__EMSCRIPTEN__ && !__ANDROID__

//////////////////////////////////////////////////////////////////////////////

Process::mstime_t
Process::mstime()
{
    timeval tv;
    gettimeofday( &tv, 0 );
    return mstime_t( tv.tv_sec ) * mstime_t( 1000 ) + mstime_t( tv.tv_usec ) / mstime_t( 1000 );
}

//////////////////////////////////////////////////////////////////////////////

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)

void
Process::beginCriticalSection()
{
    sigset_t mask;
    sigfillset( &mask );
    sigprocmask( SIG_BLOCK, &mask, &sigSavedMask );
}

//////////////////////////////////////////////////////////////////////////////

void
Process::endCriticalSection()
{
    sigprocmask( SIG_SETMASK, &sigSavedMask, NULL );
}

//////////////////////////////////////////////////////////////////////////////

void
Process::signalInit()
{   
    struct sigaction action;
    SigData* data;

    // replace actions
    for (data = sigList; data->num != -1; data++) {
        memset( &action, 0, sizeof(action) );
        action.sa_flags = data->flags | SA_SIGINFO; // using 3-arg handler
        sigfillset( &action.sa_mask );
        action.sa_sigaction = data->handler;

        if (!sigaction( data->num, &action, &data->savedAction ))
            continue;

        printf( "WARNING: failed save/replace signal(%d): error #%d\n", data->num, errno );
    }
}

//////////////////////////////////////////////////////////////////////////////

void
Process::signalShutdown()
{
    SigData* data;

    for (data = sigList; data->num != -1; data++) {
        if (!sigaction( data->num, &data->savedAction, 0 ))
            continue;

        printf( "WARNING: failed to restore signal(%d) handler: error #%d\n", data->num, errno );
    }
}

#else // __EMSCRIPTEN__ || __ANDROID__

//////////////////////////////////////////////////////////////////////////////
// WebAssembly (browser) and Android have no meaningful process signals or
// critical-section signal masking, so these are no-ops.

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
}

//////////////////////////////////////////////////////////////////////////////

void
Process::signalShutdown()
{
}

#endif // __EMSCRIPTEN__ || __ANDROID__
