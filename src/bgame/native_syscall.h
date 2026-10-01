#ifndef BGAME_NATIVE_SYSCALL_H
#define BGAME_NATIVE_SYSCALL_H

#include <stdint.h>

// Native engines read every variadic trap argument as intptr_t. In particular,
// Apple ARM64 does not promote ordinary int arguments to pointer-sized slots.
class NativeSyscall
{
public:
    typedef intptr_t (QDECL* Ptr)( intptr_t, ... );
    Ptr fn;

    NativeSyscall& operator=( Ptr p ) { fn = p; return *this; }

    template< typename... Args >
    intptr_t operator()( intptr_t cmd, Args... args ) const
    {
        // ET: Legacy's VM_CALL_END also prevents reading absent arguments.
        return fn( cmd, (intptr_t)( args )..., static_cast<intptr_t>( -1337 ) );
    }
};

#endif // BGAME_NATIVE_SYSCALL_H
